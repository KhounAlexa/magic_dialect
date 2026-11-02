//===- DependencyGraph.cpp - Magic-state dependency DAG -------------------===//

#include "magic/Dialect/Magic/Analysis/DependencyGraph.h"
#include "magic/Dialect/Magic/IR/Magic.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinAttributes.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/Twine.h"

using namespace mlir;
using namespace mlir::magic;

//===----------------------------------------------------------------------===//
// Graph construction
//===----------------------------------------------------------------------===//

static mlir::func::FuncOp enclosingFunc(mlir::Operation *op) {
  if (!op)
    return nullptr;
  if (auto func = mlir::dyn_cast<mlir::func::FuncOp>(op))
    return func;
  return op->getParentOfType<mlir::func::FuncOp>();
}

static std::string functionNameFor(mlir::Operation *op) {
  if (auto func = enclosingFunc(op))
    return func.getName().str();
  return "module";
}

// The op that "owns" a value: its defining op, or the parent op of the block
// it is an argument of.
static mlir::Operation *ownerOf(mlir::Value v) {
  if (auto arg = mlir::dyn_cast<mlir::BlockArgument>(v))
    return arg.getOwner()->getParentOp();
  return v.getDefiningOp();
}

DependencyGraph::DependencyGraph(mlir::Operation *root) {
  root->walk([&](mlir::Operation *op) {
    if (mlir::isa<CreateOp, MoveOp, ConsumeOp, TGateOp>(op))
      addOp(op);
    else if (mlir::isa<mlir::func::ReturnOp>(op))
      addReturn(op);
  });

  // A result that no op ever reads still leaves the circuit (typically the
  // classical bit of a magic.consume that is not returned). Give it an
  // Output node so every wire visibly ends somewhere. `nodes` grows inside
  // addOutput, so iterate by index over a snapshot of the size.
  for (unsigned i = 0, e = nodes.size(); i < e; ++i) {
    mlir::Operation *op = nodes[i].op;
    if (!op || nodes[i].kind == DependencyNode::Kind::Sink)
      continue;
    for (mlir::Value result : op->getResults())
      if (result.use_empty())
        addOutput(result, i);
  }
}

DependencyGraph::FunctionInfo &DependencyGraph::infoFor(mlir::Operation *op) {
  std::string name = functionNameFor(op);
  auto it = functions.find(name);
  if (it != functions.end())
    return it->second;

  mlir::func::FuncOp func = enclosingFunc(op);
  mlir::Location loc = func ? func.getLoc() : op->getLoc();

  FunctionInfo info;
  info.source = nodes.size();
  nodes.push_back({DependencyNode::Kind::Source, /*op=*/nullptr,
                   /*value=*/mlir::Value(), /*wire=*/"", loc, name});
  info.sink = nodes.size();
  nodes.push_back({DependencyNode::Kind::Sink, /*op=*/nullptr,
                   /*value=*/mlir::Value(), /*wire=*/"", loc, name});
  return functions.try_emplace(name, info).first->second;
}

unsigned DependencyGraph::nodeFor(mlir::Operation *op) {
  auto it = nodeIds.find(op);
  if (it != nodeIds.end())
    return it->second;

  FunctionInfo &info = infoFor(op);

  if (mlir::isa<mlir::func::ReturnOp>(op)) {
    nodeIds[op] = info.sink;
    if (!nodes[info.sink].op)
      nodes[info.sink].op = op;
    return info.sink;
  }

  DependencyNode::Kind kind = DependencyNode::Kind::Other;
  if (mlir::isa<CreateOp>(op))
    kind = DependencyNode::Kind::Create;
  else if (mlir::isa<MoveOp>(op))
    kind = DependencyNode::Kind::Move;
  else if (mlir::isa<ConsumeOp>(op))
    kind = DependencyNode::Kind::Consume;
  else if (mlir::isa<TGateOp>(op))
    kind = DependencyNode::Kind::TGate;

  unsigned id = nodes.size();
  nodeIds[op] = id;
  nodes.push_back({kind, op, /*value=*/mlir::Value(), /*wire=*/"",
                   op->getLoc(), functionNameFor(op)});

  // An op without operands is a root of the DAG; hang it off the Source so
  // the circuit stays connected, like Qiskit's input nodes.
  if (op->getNumOperands() == 0)
    edges.push_back({info.source, id, ""});
  return id;
}

unsigned DependencyGraph::nodeFor(mlir::BlockArgument arg) {
  void *key = arg.getAsOpaquePointer();
  auto it = nodeIds.find(key);
  if (it != nodeIds.end())
    return it->second;

  mlir::Operation *owner = arg.getOwner()->getParentOp();
  FunctionInfo &info = infoFor(owner);

  unsigned id = nodes.size();
  nodeIds[key] = id;
  nodes.push_back({DependencyNode::Kind::Input, /*op=*/nullptr, arg,
                   wireFor(arg), arg.getLoc(), functionNameFor(owner)});
  edges.push_back({info.source, id, ""});
  return id;
}

unsigned DependencyGraph::nodeForValue(mlir::Value v) {
  if (auto arg = mlir::dyn_cast<mlir::BlockArgument>(v))
    return nodeFor(arg);
  return nodeFor(v.getDefiningOp());
}

// Wire naming, Qiskit style: logical qubits are q[i], magic states m[i],
// classical bits c[i], numbered per function in order of first appearance.
// A value produced by threading a wire through an op (move, tgate, the
// updated qubit of consume) keeps the wire name of its operand.
const std::string &DependencyGraph::wireFor(mlir::Value v) {
  auto it = wires.find(v);
  if (it != wires.end())
    return it->second;

  FunctionInfo &info = infoFor(ownerOf(v));
  std::string name;

  if (mlir::Operation *def = v.getDefiningOp()) {
    if (mlir::isa<CreateOp>(def)) {
      name = ("m[" + llvm::Twine(info.states++) + "]").str();
    } else if (mlir::isa<MoveOp, TGateOp>(def)) {
      name = wireFor(def->getOperand(0));
    } else if (auto consume = mlir::dyn_cast<ConsumeOp>(def)) {
      if (v == consume.getUpdatedQubit())
        name = wireFor(consume.getQubit());
      else
        name = ("c[" + llvm::Twine(info.bits++) + "]").str();
    }
  }

  if (name.empty()) {
    // Block arguments and anything else: name by type.
    if (mlir::isa<QubitType>(v.getType()))
      name = ("q[" + llvm::Twine(info.qubits++) + "]").str();
    else if (mlir::isa<MagicStateType>(v.getType()))
      name = ("m[" + llvm::Twine(info.states++) + "]").str();
    else if (v.getType().isInteger(1))
      name = ("c[" + llvm::Twine(info.bits++) + "]").str();
    else
      name = ("v[" + llvm::Twine(info.others++) + "]").str();
  }

  return wires.try_emplace(v, std::move(name)).first->second;
}

llvm::StringRef DependencyGraph::getWireName(mlir::Value v) const {
  auto it = wires.find(v);
  return it == wires.end() ? llvm::StringRef() : llvm::StringRef(it->second);
}

void DependencyGraph::addOp(mlir::Operation *op) {
  unsigned dst = nodeFor(op);
  for (mlir::Value operand : op->getOperands())
    edges.push_back({nodeForValue(operand), dst, wireFor(operand)});
  // Name results now so numbering follows source order, not first use.
  for (mlir::Value result : op->getResults())
    wireFor(result);
}

void DependencyGraph::addReturn(mlir::Operation *returnOp) {
  nodeFor(returnOp); // binds this return to its function's Sink
  for (mlir::Value operand : returnOp->getOperands())
    addOutput(operand, nodeForValue(operand));
}

void DependencyGraph::addOutput(mlir::Value v, unsigned producer) {
  mlir::Operation *owner = ownerOf(v);
  unsigned sink = infoFor(owner).sink;
  std::string wire = wireFor(v);

  unsigned id = nodes.size();
  nodes.push_back({DependencyNode::Kind::Output, /*op=*/nullptr, v, wire,
                   v.getLoc(), functionNameFor(owner)});
  edges.push_back({producer, id, wire});
  edges.push_back({id, sink, ""});
}

//===----------------------------------------------------------------------===//
// DOT export
//===----------------------------------------------------------------------===//

// A plain "file:line:col" string, deliberately not MLIR's own location
// printer -- that wraps the filename in double quotes (loc("f":1:2)),
// which would corrupt a DOT quoted-string attribute.
static std::string locString(mlir::Location loc) {
  if (auto flc = mlir::dyn_cast<mlir::FileLineColLoc>(loc)) {
    return (flc.getFilename().getValue() + ":" + llvm::Twine(flc.getLine()) +
            ":" + llvm::Twine(flc.getColumn()))
        .str();
  }
  std::string s;
  llvm::raw_string_ostream os(s);
  os << loc;
  return s;
}

// Escape content for safe use inside a DOT quoted string.
static std::string escapeDot(llvm::StringRef s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '"' || c == '\\')
      out.push_back('\\');
    out.push_back(c);
  }
  return out;
}

// Escape each line individually, then join with DOT's own "\n"
// label-newline escape (added here, not user content, so it must not be
// run back through escapeDot).
static std::string joinDotLines(llvm::ArrayRef<std::string> lines) {
  std::string out;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i)
      out += "\\n";
    out += escapeDot(lines[i]);
  }
  return out;
}

bool mlir::magic::parseDotStyle(llvm::StringRef name, DotStyle &style) {
  if (name.equals_insensitive("qiskit")) {
    style = DotStyle::Qiskit;
    return true;
  }
  if (name.equals_insensitive("classic")) {
    style = DotStyle::Classic;
    return true;
  }
  return false;
}

namespace {
// Fully formatted DOT attributes for one node, or empty to skip it.
struct NodeStyle {
  std::string attrs;
};
} // namespace

// Mirrors the defaults of qiskit.visualization.dag_drawer: input nodes
// green, op nodes lightblue, output nodes red, all ellipses.
static NodeStyle qiskitStyleFor(const DependencyNode &n) {
  std::string label;
  llvm::StringRef fill = "lightgray";
  llvm::StringRef extra = "";
  switch (n.kind) {
  case DependencyNode::Kind::Source:
    label = "Source";
    fill = "darkseagreen";
    extra = ", fontsize=8";
    break;
  case DependencyNode::Kind::Sink:
    label = "Sink";
    fill = "red";
    extra = ", color=blue, fontsize=8";
    break;
  case DependencyNode::Kind::Input:
    label = n.wire;
    fill = "green";
    break;
  case DependencyNode::Kind::Output:
    label = n.wire;
    fill = "red";
    break;
  case DependencyNode::Kind::Create:
  case DependencyNode::Kind::Move:
  case DependencyNode::Kind::Consume:
  case DependencyNode::Kind::TGate:
  case DependencyNode::Kind::Other:
    label = n.op->getName().stripDialect().str();
    fill = "lightblue";
    break;
  }
  std::string attrs =
      ("label=\"" + escapeDot(label) + "\", fillcolor=" + fill + extra).str();
  // Source positions survive as tooltips (visible in SVG output) so the
  // picture stays as uncluttered as Qiskit's.
  if (n.op)
    attrs += ", tooltip=\"" + escapeDot(locString(n.loc)) + "\"";
  return {attrs};
}

// The original magic-opt look: boxes with source locations, consume and
// tgate highlighted as the points where a magic state is required.
static NodeStyle classicStyleFor(const DependencyNode &n) {
  std::vector<std::string> lines;
  llvm::StringRef fill = "lightgray";
  llvm::StringRef shape = "box";
  switch (n.kind) {
  case DependencyNode::Kind::Source:
  case DependencyNode::Kind::Output:
    return {}; // not drawn in this style
  case DependencyNode::Kind::Create:
    fill = "palegreen";
    lines = {"create", locString(n.loc)};
    break;
  case DependencyNode::Kind::Move:
    fill = "khaki";
    lines = {"move", locString(n.loc)};
    break;
  case DependencyNode::Kind::Consume:
    fill = "salmon";
    shape = "doubleoctagon";
    lines = {"consume", "(magic state required)", locString(n.loc)};
    break;
  case DependencyNode::Kind::TGate:
    fill = "orange";
    shape = "doubleoctagon";
    lines = {"T-gate", "(magic state required", "once lowered)",
             locString(n.loc)};
    break;
  case DependencyNode::Kind::Sink:
    fill = "white";
    shape = "cds";
    lines = {"return"};
    break;
  case DependencyNode::Kind::Input: {
    fill = "lightblue";
    shape = "ellipse";
    unsigned argNumber =
        mlir::cast<mlir::BlockArgument>(n.value).getArgNumber();
    lines = {"arg" + std::to_string(argNumber), "(logical qubit)"};
    break;
  }
  case DependencyNode::Kind::Other:
    lines = {n.op->getName().getStringRef().str()};
    break;
  }
  return {("label=\"" + joinDotLines(lines) + "\", shape=" + shape +
           ", fillcolor=" + fill)
              .str()};
}

void mlir::magic::writeDependencyGraphDot(const DependencyGraph &graph,
                                          llvm::raw_ostream &os,
                                          DotStyle style) {
  llvm::ArrayRef<DependencyNode> nodes = graph.getNodes();
  const bool qiskit = style == DotStyle::Qiskit;

  os << "digraph MagicDeps {\n";
  if (qiskit)
    os << "  rankdir=TB;\n"
       << "  node [style=filled, shape=ellipse];\n";
  else
    os << "  rankdir=LR;\n"
       << "  node [style=filled, fontname=\"Helvetica\"];\n";

  // Group nodes by function (one subgraph per circuit) so the picture reads
  // as one DAG per circuit even when a module has several. Clusters are
  // emitted in first-seen order so the output is deterministic.
  llvm::StringMap<unsigned> clusterIndex;
  std::vector<std::pair<std::string, std::vector<unsigned>>> clusters;
  llvm::StringMap<unsigned> sinkByFunction;
  for (unsigned i = 0; i < nodes.size(); ++i) {
    auto [it, inserted] =
        clusterIndex.try_emplace(nodes[i].function, clusters.size());
    if (inserted)
      clusters.push_back({nodes[i].function, {}});
    clusters[it->second].second.push_back(i);
    if (nodes[i].kind == DependencyNode::Kind::Sink)
      sinkByFunction[nodes[i].function] = i;
  }

  for (unsigned c = 0; c < clusters.size(); ++c) {
    os << "  subgraph cluster_" << c << " {\n"
       << "    label=\"" << escapeDot(clusters[c].first) << "\";\n"
       << "    style=dashed;\n";
    for (unsigned i : clusters[c].second) {
      NodeStyle s =
          qiskit ? qiskitStyleFor(nodes[i]) : classicStyleFor(nodes[i]);
      if (s.attrs.empty())
        continue;
      os << "    n" << i << " [" << s.attrs << "];\n";
    }
    os << "  }\n";
  }

  for (const DependencyEdge &e : graph.getEdges()) {
    unsigned src = e.source, dst = e.target;
    if (!qiskit) {
      // Classic hides Source and Output nodes: drop their attachment edges
      // and route a returned value straight into the "return" node. A value
      // that is simply never used has no edge at all in this style.
      DependencyNode::Kind sk = nodes[src].kind, dk = nodes[dst].kind;
      if (sk == DependencyNode::Kind::Source ||
          sk == DependencyNode::Kind::Output)
        continue;
      if (dk == DependencyNode::Kind::Output) {
        if (nodes[dst].value.use_empty())
          continue;
        dst = sinkByFunction.lookup(nodes[dst].function);
      }
    }
    os << "  n" << src << " -> n" << dst;
    if (qiskit && !e.wire.empty())
      os << " [label=\"" << escapeDot(e.wire) << "\"]";
    os << ";\n";
  }

  os << "}\n";
}
