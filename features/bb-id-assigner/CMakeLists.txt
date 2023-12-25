add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_llvm_pass_plugin(bb-id-assigner-pass bb-id-assigner-pass.so.cc)

target_include_directories(
  bb-id-assigner-pass
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(
  bb-id-assigner-pass
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall"
)
