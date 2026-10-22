cmake_minimum_required(VERSION 3.20.0)
project(magic-dialect LANGUAGES CXX C)

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")

# Out-of-tree build against an installed (or built) LLVM/MLIR, following
# mlir/examples/standalone. The source layout mirrors the MLIR tree
# (include/magic/... <-> include/mlir/..., lib/Dialect/Magic <->
# mlir/lib/Dialect/Magic) so the dialect can later be moved in-tree.
find_package(MLIR REQUIRED CONFIG)
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

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

set(MAGIC_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(MAGIC_BINARY_DIR ${PROJECT_BINARY_DIR})

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${MAGIC_SOURCE_DIR}/include)
# TableGen output lands in <build>/include/magic/...
include_directories(${MAGIC_BINARY_DIR}/include)
link_directories(${LLVM_BUILD_LIBRARY_DIR})
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)

option(MAGIC_INCLUDE_TESTS "Generate the check-magic lit test target." ON)
if(MAGIC_INCLUDE_TESTS)
  add_subdirectory(test)
endif()
