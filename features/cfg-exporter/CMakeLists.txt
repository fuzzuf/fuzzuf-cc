add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_llvm_pass_plugin(cfg-exporter-pass cfg-exporter-pass.so.cc)

target_include_directories(
  cfg-exporter-pass
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../bb-id-assigner/include
)

set_target_properties(
  cfg-exporter-pass
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall"
)
