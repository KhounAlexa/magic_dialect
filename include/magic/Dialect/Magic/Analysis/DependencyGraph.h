//===- DependencyGraph.h - Magic-state dependency DAG -----------*- C++ -*-===//

#ifndef MAGIC_DIALECT_MAGIC_ANALYSIS_DEPENDENCYGRAPH_H
#define MAGIC_DIALECT_MAGIC_ANALYSIS_DEPENDENCYGRAPH_H

#include "mlir/IR/Location.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>

namespace mlir {
namespace magic {

// One node of the circuit DAG. The graph has the shape of Qiskit's
// DAGCircuit: per function, a Source node fans out to one Input node per
// wire entering the circuit, operation nodes sit in between, and one Output
// node per value leaving the circuit feeds a Sink node.
struct DependencyNode {
  enum class Kind {
    Source,  // start of the circuit (one per function)
    Input,   // a wire entering the function as a block argument
    Create,  // magic.create -- starts a magic-state wire
    Move,    // magic.move
    Consume, // magic.consume -- ends a magic-state wire
    TGate,   // magic.tgate (pre-lowering form)
    Output,  // a value leaving the function (returned, or never used)
    Sink,    // end of the circuit (the func.return, one per function)
    Other
  };

  Kind kind;
  // The op this node stands for: the magic op, or the func.return for Sink.
  // Null for Source, Input and Output.
  mlir::Operation *op;
  // The wire value for Input and Output nodes; null otherwise.
  mlir::Value value;
  // Wire label for Input and Output nodes, e.g. "q[0]", "m[1]", "c[0]".
  std::string wire;
  mlir::Location loc;
  // Name of the enclosing func.func ("module" if there is none). Exporters
  // use it to group the nodes of one circuit together.
  std::string function;
};

// A directed edge. `wire` names the value flowing along it ("q[0]" for a
// logical qubit, "m[0]" for a magic state, "c[0]" for a classical bit) and is
// empty for the Source -> Input/Create and Output -> Sink attachment edges.
struct DependencyEdge {
  unsigned source;
  unsigned target;
  std::string wire;
};

// In-memory dependency graph over a module. Building it has no side effects
// (no files, no diagnostics), so it can be reused by any consumer: the DOT
// exporter below, a future JSON exporter, or a scheduling analysis.
//
// The (Operation *) constructor makes this usable as an MLIR analysis, i.e.
// via Pass::getAnalysis<mlir::magic::DependencyGraph>().
class DependencyGraph {
public:
  explicit DependencyGraph(mlir::Operation *root);

  llvm::ArrayRef<DependencyNode> getNodes() const { return nodes; }
  llvm::ArrayRef<DependencyEdge> getEdges() const { return edges; }

  // Wire name of a value seen while building the graph, e.g. "q[0]" for the
  // first logical-qubit argument of its function or "m[2]" for the third
  // magic state created in it. Empty for values the graph does not know.
  llvm::StringRef getWireName(mlir::Value v) const;

private:
  // Per-function bookkeeping: the Source/Sink node ids and the counters used
  // to number wires.
  struct FunctionInfo {
    unsigned source;
    unsigned sink;
    unsigned qubits = 0;
    unsigned states = 0;
    unsigned bits = 0;
    unsigned others = 0;
  };

  FunctionInfo &infoFor(mlir::Operation *op);
  unsigned nodeFor(mlir::Operation *op);
  unsigned nodeFor(mlir::BlockArgument arg);
  unsigned nodeForValue(mlir::Value v);
  const std::string &wireFor(mlir::Value v);
  void addOp(mlir::Operation *op);
  void addReturn(mlir::Operation *returnOp);
  void addOutput(mlir::Value v, unsigned producer);

  llvm::StringMap<FunctionInfo> functions;
  llvm::DenseMap<void *, unsigned> nodeIds;
  llvm::DenseMap<mlir::Value, std::string> wires;
  std::vector<DependencyNode> nodes;
  std::vector<DependencyEdge> edges;
};

// How writeDependencyGraphDot draws the graph. Both styles render the same
// DependencyGraph; they differ only in what is shown and how.
enum class DotStyle {
  // Like Qiskit's dag_drawer: top-to-bottom, a Source and a Sink per
  // function, green inputs, light blue operations, red outputs, every edge
  // labelled with its wire. Source locations become tooltips.
  Qiskit,
  // The original magic-opt picture: left-to-right, colored boxes carrying
  // the op's source location, consume/tgate highlighted as "magic state
  // required". Source and Output nodes are not drawn; returned values flow
  // straight into a "return" node.
  Classic
};

// Parse a style name ("qiskit", "classic"); returns false on unknown input.
bool parseDotStyle(llvm::StringRef name, DotStyle &style);

// Render the graph in Graphviz DOT syntax, one dashed cluster per function.
void writeDependencyGraphDot(const DependencyGraph &graph,
                             llvm::raw_ostream &os,
                             DotStyle style = DotStyle::Qiskit);

} // namespace magic
} // namespace mlir

#endif // MAGIC_DIALECT_MAGIC_ANALYSIS_DEPENDENCYGRAPH_H
