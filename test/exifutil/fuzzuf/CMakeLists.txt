cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-put-exifutil-fuzzuf
  LANGUAGES C
)

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-afl-cc)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -fno-stack-protector -Wno-address-of-packed-member -O3")

include(CheckIncludeFiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  fd-exifutil
  ../../../put/exifutil/exifutil.c
)

add_dependencies(
  fd-exifutil
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "exifutil.fuzzuf-afl-cc"
  COMMAND
  ${CMAKE_BINARY_DIR}/features/forkserver/benchmark_client ${CMAKE_BINARY_DIR}/test/exifutil/fuzzuf/fd-exifutil -f ${CMAKE_SOURCE_DIR}/put/exifutil/fuzz_input/jpeg.jpg
)
set_tests_properties("exifutil.fuzzuf-afl-cc" PROPERTIES ENVIRONMENT "N=10")

