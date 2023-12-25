cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-put-cat-fuzzuf-clang
  LANGUAGES C
)

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-clang)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")

include(CheckIncludeFiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  fuzzuf-clang-cat
  ../../../put/cat/cat.c
)

add_dependencies(
  fuzzuf-clang-cat
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "cat.fuzzuf-clang"
  COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}
)

