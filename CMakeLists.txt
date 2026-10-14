cmake_minimum_required(VERSION 3.20)
project(magic_dialect LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MLIR REQUIRED CONFIG)
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Pull in mlir_tablegen() / add_public_tablegen_target() so the .inc files
# are regenerated from MagicOps.td as part of the build instead of being
# committed by hand.
list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}" "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddLLVM)
include(AddMLIR)

# Some distro packages (e.g. Ubuntu's llvm-18) export MLIR_TABLEGEN_EXE as a
# bare program name rather than an imported target or absolute path; resolve
# it explicitly so the custom tablegen commands always find the tool.
if(NOT TARGET "${MLIR_TABLEGEN_EXE}" AND NOT EXISTS "${MLIR_TABLEGEN_EXE}")
  find_program(MAGIC_MLIR_TBLGEN
    NAMES mlir-tblgen "mlir-tblgen-${LLVM_VERSION_MAJOR}"
    HINTS "${LLVM_TOOLS_BINARY_DIR}"
    REQUIRED)
  set(MLIR_TABLEGEN_EXE "${MAGIC_MLIR_TBLGEN}")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
# Generated headers land in <build>/include/magic/*.inc.
include_directories(${PROJECT_BINARY_DIR}/include)
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(include/magic)

add_executable(magic-opt
  tools/magic-opt.cpp
  src/Dialect.cpp
  src/LowerTGate.cpp
  src/DependencyGraph.cpp
  src/DependencyAnalysis.cpp
)

add_dependencies(magic-opt MagicOpsIncGen)

target_link_libraries(magic-opt
  PRIVATE
  MLIROptLib
  MLIRIR
  MLIRFuncDialect
)
