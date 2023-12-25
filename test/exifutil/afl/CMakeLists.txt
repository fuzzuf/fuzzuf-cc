cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-put-exifutil-afl
  LANGUAGES C
)

set(CMAKE_C_COMPILER afl-clang)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -fno-stack-protector -Wno-address-of-packed-member -O3")
SET(CMAKE_AR      "llvm-ar")
SET(CMAKE_LINKER  "llvm-ld")
SET(CMAKE_NM      "llvm-nm")
SET(CMAKE_OBJDUMP "llvm-objdump")
SET(CMAKE_RANLIB  "llvm-ranlib")

include(CheckIncludeFiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  afl-exifutil
  ../../../put/exifutil/exifutil.c
)

