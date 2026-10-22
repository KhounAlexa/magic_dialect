add_llvm_executable(magic-opt magic-opt.cpp)
llvm_update_compile_flags(magic-opt)

target_link_libraries(magic-opt
  PRIVATE
  MLIRFuncDialect
  MLIRMagicDialect
  MLIRMagicTransforms
  MLIROptLib
  )

mlir_check_all_link_libraries(magic-opt)
