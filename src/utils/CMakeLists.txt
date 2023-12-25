add_library(
  fuzzuf-cc-utils
  SHARED
  create_shared_memory.cpp
  errno_to_system_error.cpp
  executor.cpp
)

set_target_properties(
  fuzzuf-cc-utils
  PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS_STR}"
)
set_target_properties(
  fuzzuf-cc-utils
  PROPERTIES LINK_FLAGS "${ADDITIONAL_LINK_FLAGS_STR}"
)

target_include_directories(
  fuzzuf-cc-utils
  PRIVATE
  ${FUZZUF_CC_INCLUDE_DIRS}
)

