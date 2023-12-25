add_library(
  bitmap-llvm-rt
  OBJECT
  bitmap-llvm-rt.o.c
)

set_target_properties(
  bitmap-llvm-rt
  PROPERTIES
  PREFIX
  ""
)

set_target_properties(
  bitmap-llvm-rt
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall"
)

add_subdirectory(llvm-pass)
