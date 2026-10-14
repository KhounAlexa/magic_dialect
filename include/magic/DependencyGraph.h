#ifndef MAGIC_DEPENDENCYGRAPH_H
#define MAGIC_DEPENDENCYGRAPH_H

#include "mlir/IR/Location.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <utility>
#include <vector>

namespace magic {

// One node of the gate-execution / magic-state dependency DAG: a
// magic-dialect op, a func.return, or a logical qubit that enters a function
// as a block argument.
struct DependencyNode {
  enum class Kind { Create, Move, Consume, TGate, Return, Argument, Other };

  Kind kind;
  // The op this node stands for; null for Kind::Argument.
  mlir::Operation *op;
  // Argument position within its block; only meaningful for Kind::Argument.
  unsigned argNumber;
  mlir::Location loc;
  // Name of the enclosing func.func ("module" if there is none). Exporters
  // use it to group the nodes of one circuit together.
  std::string function;
};

// In-memory dependency graph over a module. Building it has no side effects
// (no files, no diagnostics), so it can be reused by any consumer: the DOT
// exporter below, a future JSON exporter, or a scheduling analysis.
//
// The (Operation *) constructor makes this usable as an MLIR analysis, i.e.
// via Pass::getAnalysis<magic::DependencyGraph>().
class DependencyGraph {
public:
  using Edge = std::pair<unsigned, unsigned>; // (source node, user node)

  explicit DependencyGraph(mlir::Operation *root);

  llvm::ArrayRef<DependencyNode> getNodes() const { return nodes; }
  llvm::ArrayRef<Edge> getEdges() const { return edges; }

private:
  unsigned nodeFor(mlir::Operation *op);
  unsigned nodeFor(mlir::BlockArgument arg);
  unsigned nodeForValue(mlir::Value v);
  void addOp(mlir::Operation *op);

  llvm::DenseMap<void *, unsigned> nodeIds;
  std::vector<DependencyNode> nodes;
  std::vector<Edge> edges;
};

// Render the graph in Graphviz DOT syntax, one dashed cluster per function.
void writeDependencyGraphDot(const DependencyGraph &graph,
                             llvm::raw_ostream &os);

} // namespace magic

#endif
