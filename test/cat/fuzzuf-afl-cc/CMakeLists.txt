cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-put-cat-fuzzuf-afl-cc
  LANGUAGES C
)

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-afl-cc)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")

include(CheckIncludeFiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  fuzzuf-afl-cc-cat
  ../../../put/cat/cat.c
)

add_dependencies(
  fuzzuf-afl-cc-cat
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "cat.fuzzuf-afl-cc"
  COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}
)

