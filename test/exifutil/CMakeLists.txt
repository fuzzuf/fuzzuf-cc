find_program (HAS_CLANG "clang")
if( HAS_CLANG )
add_subdirectory(
  plain
)
add_subdirectory(
  asan
)
endif()
find_program (HAS_AFL_CLANG "afl-clang")
if( HAS_AFL_CLANG )
add_subdirectory(
  afl
)
endif()
add_subdirectory(
  fuzzuf
)

