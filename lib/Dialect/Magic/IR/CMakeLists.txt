add_mlir_dialect_library(MLIRMagicDialect
  MagicDialect.cpp
  MagicOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${MAGIC_SOURCE_DIR}/include/magic/Dialect/Magic/IR

  DEPENDS
  MLIRMagicOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRBytecodeOpInterface
  )
