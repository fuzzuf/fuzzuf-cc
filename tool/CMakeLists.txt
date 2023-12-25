add_executable(
  dump_ijon_shm
  dump_ijon_shm.cpp
)

set_target_properties(
  dump_ijon_shm
  PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS_STR}"
)
set_target_properties(
  dump_ijon_shm
  PROPERTIES LINK_FLAGS "${ADDITIONAL_LINK_FLAGS_STR}"
)

target_include_directories(
  dump_ijon_shm
  PRIVATE
  ${FUZZUF_CC_INCLUDE_DIRS}
)

target_link_libraries(
  dump_ijon_shm
  fuzzuf-cc-utils
)
