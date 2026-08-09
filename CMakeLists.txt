cmake_minimum_required(VERSION 3.20)
project(magic_dialect)

find_package(MLIR REQUIRED CONFIG)

include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(magic-opt
  magic-opt.cpp
  Dialect.cpp
  LowerTGate.cpp
  DependencyAnalysis.cpp
)

target_link_libraries(magic-opt
  PRIVATE
  MLIROptLib
  MLIRIR
  MLIRFuncDialect
)
