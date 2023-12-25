add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_llvm_pass_plugin(bitmap-llvm-pass bitmap-llvm-pass.so.cc)

target_include_directories(
  bitmap-llvm-pass
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../bb-id-assigner/include
)

set_target_properties(
  bitmap-llvm-pass
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall"
)
