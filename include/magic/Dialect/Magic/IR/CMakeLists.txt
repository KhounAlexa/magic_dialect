# Equivalent to `add_mlir_dialect(MagicOps magic)` in the MLIR tree. It is
# spelled out here because that helper also adds a dependency on the
# in-tree-only `mlir-headers` target. The generated file and target names
# match it, so moving in-tree only means replacing this block.
set(LLVM_TARGET_DEFINITIONS MagicOps.td)
mlir_tablegen(MagicOps.h.inc -gen-op-decls)
mlir_tablegen(MagicOps.cpp.inc -gen-op-defs)
mlir_tablegen(MagicOpsTypes.h.inc -gen-typedef-decls -typedefs-dialect=magic)
mlir_tablegen(MagicOpsTypes.cpp.inc -gen-typedef-defs -typedefs-dialect=magic)
mlir_tablegen(MagicOpsDialect.h.inc -gen-dialect-decls -dialect=magic)
mlir_tablegen(MagicOpsDialect.cpp.inc -gen-dialect-defs -dialect=magic)
add_public_tablegen_target(MLIRMagicOpsIncGen)
