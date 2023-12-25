add_library(
  ijon-llvm-rt
  OBJECT
  ijon-llvm-rt.o.c
)

set_target_properties(
  ijon-llvm-rt
  PROPERTIES
  PREFIX
  ""
)

set_target_properties(
  ijon-llvm-rt
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall"
)

