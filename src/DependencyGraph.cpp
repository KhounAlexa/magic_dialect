#include "magic/DependencyGraph.h"
#include "magic/Dialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinAttributes.h"

#include "llvm/ADT/StringMap.h"

using namespace magic;

//===----------------------------------------------------------------------===//
// Graph construction
//===----------------------------------------------------------------------===//

static std::string functionNameFor(mlir::Operation *op) {
  if (!op)
    return "module";
  if (auto func = mlir::dyn_cast<mlir::func::FuncOp>(op))
    return func.getName().str();
  if (auto func = op->getParentOfType<mlir::func::FuncOp>())
    return func.getName().str();
  return "module";
}

DependencyGraph::DependencyGraph(mlir::Operation *root) {
  root->walk([&](mlir::Operation *op) {
    if (mlir::isa<magic::CreateOp, magic::MoveOp, magic::ConsumeOp,
                  magic::TGateOp, mlir::func::ReturnOp>(op))
      addOp(op);
  });
}

unsigned DependencyGraph::nodeFor(mlir::Operation *op) {
  auto it = nodeIds.find(op);
  if (it != nodeIds.end())
    return it->second;

  DependencyNode::Kind kind = DependencyNode::Kind::Other;
  if (mlir::isa<magic::CreateOp>(op))
    kind = DependencyNode::Kind::Create;
  else if (mlir::isa<magic::MoveOp>(op))
    kind = DependencyNode::Kind::Move;
  else if (mlir::isa<magic::ConsumeOp>(op))
    kind = DependencyNode::Kind::Consume;
  else if (mlir::isa<magic::TGateOp>(op))
    kind = DependencyNode::Kind::TGate;
  else if (mlir::isa<mlir::func::ReturnOp>(op))
    kind = DependencyNode::Kind::Return;

  unsigned id = nodes.size();
  nodeIds[op] = id;
  nodes.push_back({kind, op, /*argNumber=*/0, op->getLoc(),
                   functionNameFor(op)});
  return id;
}

unsigned DependencyGraph::nodeFor(mlir::BlockArgument arg) {
  void *key = arg.getAsOpaquePointer();
  auto it = nodeIds.find(key);
  if (it != nodeIds.end())
    return it->second;

  unsigned id = nodes.size();
  nodeIds[key] = id;
  nodes.push_back({DependencyNode::Kind::Argument, /*op=*/nullptr,
                   arg.getArgNumber(), arg.getLoc(),
                   functionNameFor(arg.getOwner()->getParentOp())});
  return id;
}

unsigned DependencyGraph::nodeForValue(mlir::Value v) {
  if (auto arg = mlir::dyn_cast<mlir::BlockArgument>(v))
    return nodeFor(arg);
  return nodeFor(v.getDefiningOp());
}

void DependencyGraph::addOp(mlir::Operation *op) {
  unsigned dst = nodeFor(op);
  for (mlir::Value operand : op->getOperands())
    edges.emplace_back(nodeForValue(operand), dst);
}

//===----------------------------------------------------------------------===//
// DOT export
//===----------------------------------------------------------------------===//

// A plain "file:line:col" string, deliberately not MLIR's own location
// printer -- that wraps the filename in double quotes (loc("f":1:2)),
// which would corrupt a DOT label's quoted-string syntax.
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

// Escape one line of content for safe use inside a DOT quoted-string label.
static std::string escapeDotLabel(llvm::StringRef s) {
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
// run back through escapeDotLabel).
static std::string joinDotLines(llvm::ArrayRef<std::string> lines) {
  std::string out;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i)
      out += "\\n";
    out += escapeDotLabel(lines[i]);
  }
  return out;
}

namespace {
struct DotStyle {
  std::vector<std::string> labelLines;
  llvm::StringRef fillColor = "lightgray";
  llvm::StringRef shape = "box";
};
} // namespace

static DotStyle styleFor(const DependencyNode &n) {
  DotStyle s;
  switch (n.kind) {
  case DependencyNode::Kind::Create:
    s.fillColor = "palegreen";
    s.labelLines = {"create", locString(n.loc)};
    break;
  case DependencyNode::Kind::Move:
    s.fillColor = "khaki";
    s.labelLines = {"move", locString(n.loc)};
    break;
  case DependencyNode::Kind::Consume:
    s.fillColor = "salmon";
    s.shape = "doubleoctagon";
    s.labelLines = {"consume", "(magic state required)", locString(n.loc)};
    break;
  case DependencyNode::Kind::TGate:
    s.fillColor = "orange";
    s.shape = "doubleoctagon";
    s.labelLines = {"T-gate", "(magic state required", "once lowered)",
                    locString(n.loc)};
    break;
  case DependencyNode::Kind::Return:
    s.fillColor = "white";
    s.shape = "cds";
    s.labelLines = {"return"};
    break;
  case DependencyNode::Kind::Argument:
    s.fillColor = "lightblue";
    s.shape = "ellipse";
    s.labelLines = {"arg" + std::to_string(n.argNumber), "(logical qubit)"};
    break;
  case DependencyNode::Kind::Other:
    s.labelLines = {n.op->getName().getStringRef().str()};
    break;
  }
  return s;
}

void magic::writeDependencyGraphDot(const DependencyGraph &graph,
                                    llvm::raw_ostream &os) {
  llvm::ArrayRef<DependencyNode> nodes = graph.getNodes();

  os << "digraph MagicDeps {\n"
     << "  rankdir=LR;\n"
     << "  node [style=filled, fontname=\"Helvetica\"];\n";

  // Group nodes by function (one subgraph per circuit) so the picture reads
  // as one DAG per circuit even when a module has several. Clusters are
  // emitted in first-seen order so the output is deterministic.
  llvm::StringMap<unsigned> clusterIndex;
  std::vector<std::pair<std::string, std::vector<unsigned>>> clusters;
  for (unsigned i = 0; i < nodes.size(); ++i) {
    auto [it, inserted] =
        clusterIndex.try_emplace(nodes[i].function, clusters.size());
    if (inserted)
      clusters.push_back({nodes[i].function, {}});
    clusters[it->second].second.push_back(i);
  }

  for (unsigned c = 0; c < clusters.size(); ++c) {
    os << "  subgraph cluster_" << c << " {\n"
       << "    label=\"" << escapeDotLabel(clusters[c].first) << "\";\n"
       << "    style=dashed;\n";
    for (unsigned i : clusters[c].second) {
      DotStyle s = styleFor(nodes[i]);
      os << "    n" << i << " [label=\"" << joinDotLines(s.labelLines)
         << "\", shape=" << s.shape << ", fillcolor=" << s.fillColor
         << "];\n";
    }
    os << "  }\n";
  }

  for (const DependencyGraph::Edge &e : graph.getEdges())
    os << "  n" << e.first << " -> n" << e.second << ";\n";

  os << "}\n";
}
