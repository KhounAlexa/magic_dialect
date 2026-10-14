# Every .inc consumed by the dialect is generated from MagicOps.td at build
# time; editing the .td file re-runs mlir-tblgen automatically.
set(LLVM_TARGET_DEFINITIONS MagicOps.td)
mlir_tablegen(MagicDialect.h.inc -gen-dialect-decls -dialect=magic)
mlir_tablegen(MagicDialect.cpp.inc -gen-dialect-defs -dialect=magic)
mlir_tablegen(MagicTypes.h.inc -gen-typedef-decls -typedefs-dialect=magic)
mlir_tablegen(MagicTypes.cpp.inc -gen-typedef-defs -typedefs-dialect=magic)
mlir_tablegen(MagicOps.h.inc -gen-op-decls)
mlir_tablegen(MagicOps.cpp.inc -gen-op-defs)
add_public_tablegen_target(MagicOpsIncGen)
