
add_library(
  forkserver
  OBJECT
  forkserver.c
)

set_target_properties(
  forkserver
  PROPERTIES
  PREFIX
  ""
)

set_target_properties(
  forkserver
  PROPERTIES
  COMPILE_FLAGS
  "-Werror -Wall -fPIC"
)

if( ENABLE_VERBOSE_MODE )
  target_compile_definitions(
    forkserver
    PRIVATE
    -DSHOW_LOG=y
  )
endif()

add_executable(
  benchmark_client
  benchmark_client.cc
)

set_target_properties(
  benchmark_client
  PROPERTIES
  COMPILE_FLAGS
  "${ADDITIONAL_COMPILE_FLAGS_STR}"
)

set_target_properties(
  benchmark_client
  PROPERTIES
  LINK_FLAGS
  "${ADDITIONAL_LINK_FLAGS_STR}"
)


