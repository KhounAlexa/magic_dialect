#include "Dialect.h"
#include "Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <system_error>
#include <vector>

namespace {

// One node in the rendered dependency DAG: a magic-dialect op, or a logical
// qubit that enters a function as a block argument. The label is kept as
// separate lines (rather than one pre-joined string) so each piece of
// content can be escaped individually before we insert our own DOT
// newline separators between them -- see joinDotLines.
struct DagNode {
  std::string dotId;
  std::vector<std::string> labelLines;
  llvm::StringRef fillColor;
  llvm::StringRef shape;
  std::string cluster;
};

struct DependencyAnalysisPass
    : public mlir::PassWrapper<DependencyAnalysisPass,
                                mlir::OperationPass<mlir::ModuleOp>> {

  llvm::StringRef getArgument() const final { return "print-magic-deps"; }

  llvm::StringRef getDescription() const final {
    return "Analyze gate-execution / magic-state dependencies and emit a "
           "DAG (magic-deps.dot) alongside per-op diagnostics. Works on "
           "both a raw circuit (magic.tgate) and a lowered one "
           "(magic.create/move/consume).";
  }

  llvm::DenseMap<void *, unsigned> nodeIds;
  std::vector<DagNode> nodes;
  std::vector<std::pair<unsigned, unsigned>> edges;

  // A plain "file:line:col" string, deliberately not MLIR's own location
  // printer -- that wraps the filename in double quotes (loc("f":1:2)),
  // which would corrupt a DOT label's quoted-string syntax.
  static std::string locString(mlir::Location loc) {
    if (auto flc = mlir::dyn_cast<mlir::FileLineColLoc>(loc)) {
      return (flc.getFilename().getValue() + ":" +
              llvm::Twine(flc.getLine()) + ":" + llvm::Twine(flc.getColumn()))
          .str();
    }
    std::string s;
    llvm::raw_string_ostream os(s);
    os << loc;
    return s;
  }

  // Escape one line of content for safe use inside a DOT quoted-string
  // label.
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

  static std::string clusterFor(mlir::Operation *op) {
    if (auto func = op->getParentOfType<mlir::func::FuncOp>())
      return func.getName().str();
    return "module";
  }

  unsigned nodeFor(mlir::Operation *op) {
    auto it = nodeIds.find(op);
    if (it != nodeIds.end())
      return it->second;

    llvm::StringRef color = "lightgray";
    llvm::StringRef shape = "box";
    std::vector<std::string> lines = {op->getName().getStringRef().str()};

    if (mlir::isa<magic::CreateOp>(op)) {
      color = "palegreen";
      lines = {"create", locString(op->getLoc())};
    } else if (mlir::isa<magic::MoveOp>(op)) {
      color = "khaki";
      lines = {"move", locString(op->getLoc())};
    } else if (mlir::isa<magic::ConsumeOp>(op)) {
      color = "salmon";
      shape = "doubleoctagon";
      lines = {"consume", "(magic state required)", locString(op->getLoc())};
    } else if (mlir::isa<magic::TGateOp>(op)) {
      color = "orange";
      shape = "doubleoctagon";
      lines = {"T-gate", "(magic state required", "once lowered)",
                locString(op->getLoc())};
    } else if (mlir::isa<mlir::func::ReturnOp>(op)) {
      color = "white";
      shape = "cds";
      lines = {"return"};
    }

    unsigned id = nodes.size();
    nodeIds[op] = id;
    nodes.push_back({"n" + std::to_string(id), std::move(lines), color,
                      shape, clusterFor(op)});
    return id;
  }

  unsigned nodeFor(mlir::BlockArgument arg) {
    void *key = arg.getAsOpaquePointer();
    auto it = nodeIds.find(key);
    if (it != nodeIds.end())
      return it->second;

    unsigned id = nodes.size();
    nodeIds[key] = id;
    std::vector<std::string> lines = {
        "arg" + std::to_string(arg.getArgNumber()), "(logical qubit)"};
    std::string cluster = "module";
    if (auto func = mlir::dyn_cast_or_null<mlir::func::FuncOp>(
            arg.getOwner()->getParentOp()))
      cluster = func.getName().str();
    nodes.push_back({"n" + std::to_string(id), std::move(lines), "lightblue",
                      "ellipse", cluster});
    return id;
  }

  unsigned nodeForValue(mlir::Value v) {
    if (auto arg = mlir::dyn_cast<mlir::BlockArgument>(v))
      return nodeFor(arg);
    return nodeFor(v.getDefiningOp());
  }

  void addOp(mlir::Operation *op) {
    unsigned dst = nodeFor(op);
    for (mlir::Value operand : op->getOperands())
      edges.emplace_back(nodeForValue(operand), dst);
  }

  void writeDot(llvm::raw_ostream &os) {
    os << "digraph MagicDeps {\n"
       << "  rankdir=LR;\n"
       << "  node [style=filled, fontname=\"Helvetica\"];\n";

    // Group nodes by cluster (one subgraph per function) so the picture
    // reads as one DAG per circuit even when a module has several.
    llvm::StringMap<std::vector<unsigned>> byCluster;
    for (unsigned i = 0; i < nodes.size(); ++i)
      byCluster[nodes[i].cluster].push_back(i);

    unsigned clusterIdx = 0;
    for (auto &entry : byCluster) {
      os << "  subgraph cluster_" << clusterIdx++ << " {\n"
         << "    label=\"" << escapeDotLabel(entry.first()) << "\";\n"
         << "    style=dashed;\n";
      for (unsigned i : entry.second) {
        const DagNode &n = nodes[i];
        os << "    " << n.dotId << " [label=\"" << joinDotLines(n.labelLines)
           << "\", shape=" << n.shape << ", fillcolor=" << n.fillColor
           << "];\n";
      }
      os << "  }\n";
    }

    for (auto &e : edges)
      os << "  " << nodes[e.first].dotId << " -> " << nodes[e.second].dotId
         << ";\n";

    os << "}\n";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    nodeIds.clear();
    nodes.clear();
    edges.clear();

    // Build the dependency graph and emit per-op diagnostics identifying
    // where a magic state is required, whether the circuit is still raw
    // (magic.tgate not yet lowered) or already lowered
    // (magic.create/move/consume present).
    module.walk([&](mlir::Operation *op) {
      if (mlir::isa<magic::CreateOp, magic::MoveOp, magic::ConsumeOp,
                     magic::TGateOp, mlir::func::ReturnOp>(op))
        addOp(op);
    });

    module.walk([&](magic::TGateOp tgateOp) {
      tgateOp.emitRemark()
          << "magic state required HERE (not yet lowered): this T gate "
             "will need exactly 1 magic state qubit, produced via "
             "create -> move -> consume once --lower-tgate runs";
    });

    module.walk([&](magic::ConsumeOp consumeOp) {
      auto moveOp = consumeOp.getMagicState().getDefiningOp<magic::MoveOp>();
      if (!moveOp) {
        consumeOp.emitRemark() << "consume has no traceable move op";
        return;
      }
      auto createOp = moveOp.getValue().getDefiningOp<magic::CreateOp>();
      if (!createOp) {
        consumeOp.emitRemark() << "consume's move has no traceable create op";
        return;
      }
      consumeOp.emitRemark()
          << "magic state required HERE: this consume op depends on the "
             "create op at "
          << createOp.getLoc() << " (via move op at " << moveOp.getLoc()
          << ")";
    });

    llvm::StringRef dotOutputPath = "magic-deps.dot";
    std::error_code ec;
    llvm::raw_fd_ostream dotFile(dotOutputPath, ec);
    if (ec) {
      module.emitError() << "failed to write dependency graph to '"
                          << dotOutputPath << "': " << ec.message();
      return;
    }
    writeDot(dotFile);
    module.emitRemark() << "wrote magic-state dependency DAG to '"
                         << dotOutputPath
                         << "' (render with: dot -Tpng magic-deps.dot -o "
                            "magic-deps.png)";
  }
};

} // namespace

namespace magic {
void registerDependencyAnalysisPass() {
  mlir::PassRegistration<DependencyAnalysisPass>();
}
} // namespace magic
