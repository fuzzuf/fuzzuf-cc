cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

if( ${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12.0 )
  PROJECT(
    fuzzuf-cc
    LANGUAGES CXX C
    HOMEPAGE_URL https://github.com/fuzzuf/fuzzuf-cc
    DESCRIPTION "Extensible compiler for instrumentation"
    VERSION 0.1.0
  )
else()
  PROJECT(
    fuzzuf-cc
    LANGUAGES CXX C
    DESCRIPTION "Extensible compiler for instrumentation"
    VERSION 0.1.0
  )
  set( CMAKE_HOMEPAGE_URL_COMPILER "https://github.com/fuzzuf/fuzzuf-cc" )
endif()

include(CheckIncludeFiles)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)
find_package(PkgConfig)
find_package(PkgConfig)
find_package( Python3x REQUIRED )
find_package( Boost 1.53.0 REQUIRED )
find_package( Boost 1.53.0 COMPONENTS system unit_test_framework REQUIRED )
find_package( LLVM 15 REQUIRED CONFIG )
enable_testing()

if( ${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.6.0 )
  option( ENABLE_CLANG_TIDY "enable clang-tidy" OFF )
  if( ENABLE_CLANG_TIDY )
    find_program( CLANG_TIDY clang-tidy REQUIRED )
    set( CLANG_TIDY_CONFIG "-checks=-*,boost-*,bugprone-*,cert-*,cppcoreguidelines-*,hicpp-*,modernize-*,performance-*,portability-*,readability-*,misc-*" )
    set( CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY} ${CLANG_TIDY_CONFIG} )
    set( CLANG_TIDY_CONFIG_FOR_TEST "${CLANG_TIDY_CONFIG},-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-cppcoreguidelines-macro-usage,-cert-err58-cpp,-cppcoreguidelines-avoid-non-const-global-variables,-cppcoreguidelines-pro-type-vararg,-hicpp-vararg,-hicpp-explicit-conversions,-cppcoreguidelines-macro-usage,-cppcoreguidelines-pro-type-cstyle-cast,-cppcoreguidelines-pro-type-member-init,-cppcoreguidelines-special-member-functions,-hicpp-explicit-conversions,-cppcoreguidelines-pro-type-vararg,-hicpp-vararg" )
  endif()
endif()

set( ADDITIONAL_COMPILE_FLAGS )
list(APPEND ADDITIONAL_COMPILE_FLAGS "-Werror" )
option( ENABLE_ADDRSAN "build with address sanitizer" OFF)
if( ENABLE_ADDRSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=address" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=address" )
  if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT ENABLE_CLANG_TIDY )
    list(APPEND ADDITIONAL_COMPILE_FLAGS "-static-libasan" )
    list(APPEND ADDITIONAL_LINK_FLAGS "-static-libasan" )
  endif()
endif()
option( ENABLE_MSAN "build with memory sanitizer" OFF)
if( ENABLE_MSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=memory" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=memory" )
endif()

option( ENABLE_PCOMPSAN "build with pointer compare sanitizer" OFF)
if( ENABLE_PCOMPSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=pointer-compare" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=pointer-compare" )
endif()

option( ENABLE_PSUBSAN "build with pointer subtract sanitizer" OFF)
if( ENABLE_PSUBSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=pointer-subtract" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=pointer-subtract" )
endif()

option( ENABLE_THREADSAN "build with thread sanitizer" OFF)
if( ENABLE_THREADSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=thread" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=thread" )
endif()

option( ENABLE_LEAKSAN "build with leak sanitizer" OFF)
if( ENABLE_LEAKSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=leak" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=leak" )
endif()

option( ENABLE_UBSAN "build with undefined behavior sanitizer" OFF)
if( ENABLE_UBSAN )
  list(APPEND ADDITIONAL_COMPILE_FLAGS "-fsanitize=undefined" )
  list(APPEND ADDITIONAL_LINK_FLAGS "-fsanitize=undefined" )
  if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT ENABLE_CLANG_TIDY )
    list(APPEND ADDITIONAL_COMPILE_FLAGS "-static-libubsan" )
    list(APPEND ADDITIONAL_LINK_FLAGS "-static-libubsan" )
  elseif( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  endif()
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  list(APPEND ADDITIONAL_COMPILER_FLAGS "-fsized-deallocation" )
endif()
option( ENABLE_CXX20 "build with -std=c++20" OFF)
if( ENABLE_CXX20 )
  set( CMAKE_CXX_STANDARD "20" )
else()
  set( CMAKE_CXX_STANDARD "17" )
endif()

set(CMAKE_BUILD_TYPE Debug)

if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0 )
    option( ENABLE_STATIC_ANALYZER "enable static analyzer" OFF)
    if( ENABLE_STATIC_ANALYZER )
      list(APPEND ADDITIONAL_COMPILE_FLAGS "-fanalyzer")
    endif()
  endif()
endif()

string(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS_STR "${ADDITIONAL_COMPILE_FLAGS}")
string(REPLACE ";" " " ADDITIONAL_LINK_FLAGS_STR "${ADDITIONAL_LINK_FLAGS}")

set(ADDITIONAL_WARNING_FLAGS)
list( APPEND ADDITIONAL_WARNING_FLAGS "-Wall" )
list( APPEND ADDITIONAL_WARNING_FLAGS "-Wextra" )
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR ENABLE_CLANG_TIDY )
  list( APPEND ADDITIONAL_WARNING_FLAGS "-Wno-unevaluated-expression" )
  list( APPEND ADDITIONAL_WARNING_FLAGS "-Wno-unneeded-internal-declaration" )
  list( APPEND ADDITIONAL_WARNING_FLAGS "-Wno-inconsistent-missing-override" )
  list( APPEND ADDITIONAL_WARNING_FLAGS "-Wno-unused-variable" )
  list( APPEND ADDITIONAL_WARNING_FLAGS "-fsized-deallocation" )
endif()
string(REPLACE ";" " " ADDITIONAL_WARNING_FLAGS_STR "${ADDITIONAL_WARNING_FLAGS}")

set( RELEASE_MARCH "native" CACHE STRING "target cpu architecture")

set(CMAKE_CXX_FLAGS_DEBUG "-g ${ADDITIONAL_WARNING_FLAGS_STR}")
set(CMAKE_C_FLAGS_DEBUG "-g ${ADDITIONAL_WARNING_FLAGS_STR}")
set(CMAKE_CXX_FLAGS_RELEASE "-march=${RELEASE_MARCH} -O2 ${ADDITIONAL_WARNING_FLAGS_STR}")
set(CMAKE_C_FLAGS_RELEASE "-march=${RELEASE_MARCH} -O2 ${ADDITIONAL_WARNING_FLAGS_STR}")

option( ENABLE_VERBOSE_MODE "enable verbose log" OFF)

list(APPEND FUZZUF_CC_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include" )
list(APPEND FUZZUF_CC_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}" )
list(APPEND FUZZUF_CC_INCLUDE_DIRS "${CMAKE_BINARY_DIR}" )
list(APPEND FUZZUF_CC_INCLUDE_DIRS "${Boost_INCLUDE_DIRS}" )

configure_file(
  "${CMAKE_SOURCE_DIR}/check/config.h.in"
  "${CMAKE_BINARY_DIR}/config.h"
)

configure_file(
  "${CMAKE_SOURCE_DIR}/check/config.py.in"
  "${CMAKE_BINARY_DIR}/_config.py"
)

subdirs( features src tool test )

add_custom_target(
  fuzzuf-cc-for-test
  ALL
  BYPRODUCTS
  ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf_cc
  ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-cc
  ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-c++
  ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-clang
  COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/check/fix_lib_dir.sh ${CMAKE_CURRENT_SOURCE_DIR}
                                                   ${CMAKE_CURRENT_BINARY_DIR}
                                                   $<TARGET_OBJECTS:forkserver>
                                                   $<TARGET_OBJECTS:ijon-llvm-rt>
                                                   $<TARGET_OBJECTS:bitmap-llvm-rt>
                                                   features/bitmap/llvm-pass/bitmap-llvm-pass.so
                                                   features/bb-id-assigner/bb-id-assigner-pass.so
                                                   features/cfg-exporter/cfg-exporter-pass.so
  DEPENDS
  forkserver
  ijon-llvm-rt
  bitmap-llvm-rt
  bitmap-llvm-pass
  bb-id-assigner-pass
  cfg-exporter-pass
)
install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf_cc/
  DESTINATION ${Python3x_FIRST_SITELIB}/fuzzuf_cc/
  FILES_MATCHING
  PATTERN "*.py"
  PATTERN "*.o"
  PATTERN "*.so"
  PATTERN "*.h"
)
if( ${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.14.0 )
  install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-c++
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-c++
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-ijon-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-clang
    TYPE BIN
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
  )
else()
  install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-c++
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-afl-c++
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-ijon-cc
    ${CMAKE_CURRENT_BINARY_DIR}/fuzzuf-clang
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
  )
endif()

set( PATTR_VERSION_RELEASE 1 CACHE STRING "Release number of package" )
math( EXPR PATTR_VERSION_RELEASE_CALCULATED "${PATTR_VERSION_RELEASE}" )
set( EASYPACK_VENDOR "Ricerca Security" )
set( EASYPACK_RELEASE "${PATTR_VERSION_RELEASE_CALCULATED}" )
set( EASYPACK_MAINTAINER "Ricerca Security <fuzzuf-dev@ricsec.co.jp>" )
set( EASYPACK_COMPRESSION_TYPE "xz" )
set( EASYPACK_LICENSE_NAME "AGPL v3.0" )
include(EasyPack)


