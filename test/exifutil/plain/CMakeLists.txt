cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-put-exifutil-plain
  LANGUAGES C
)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -fno-stack-protector -Wno-address-of-packed-member -O3")

include(CheckIncludeFiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  exifutil
  ../../../put/exifutil/exifutil.c
)

