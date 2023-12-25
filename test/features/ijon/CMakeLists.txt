cmake_minimum_required(VERSION 3.10)
set(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  ${CMAKE_MODULE_PATH}
)

PROJECT(
  fuzzuf-cc-targets
  LANGUAGES CXX C
)

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-ijon-cc)

execute_process(
  COMMAND git rev-parse HEAD
  OUTPUT_VARIABLE PATTR_COMMIT
)

execute_process(
  COMMAND whoami
  OUTPUT_VARIABLE PATTR_USER
)

include(CheckIncludeFiles)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)
find_package(PkgConfig)
enable_testing()

add_executable(
  ijon-test_put0
  ../../../features/ijon/test_put.c
)

add_dependencies(
  ijon-test_put0
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "ijon.run_test_put0"
  COMMAND
  ${CMAKE_BINARY_DIR}/features/forkserver/benchmark_client ${CMAKE_BINARY_DIR}/test/features/ijon/ijon-test_put0
)
set_tests_properties("ijon.run_test_put0" PROPERTIES ENVIRONMENT "N=1000")

add_executable(
  ijon-test_put1
  ../../../put/ijon/test.c 
)

add_dependencies(
  ijon-test_put1
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "ijon.run_test_put1"
  COMMAND
  ${CMAKE_BINARY_DIR}/features/forkserver/benchmark_client ${CMAKE_BINARY_DIR}/test/features/ijon/ijon-test_put1
)
set_tests_properties("ijon.run_test_put1" PROPERTIES ENVIRONMENT "N=10")

add_executable(
  ijon-test_put2
  ../../../put/ijon/test2.c 
)

add_dependencies(
  ijon-test_put2
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "ijon.run_test_put2"
  COMMAND
  ${CMAKE_BINARY_DIR}/features/forkserver/benchmark_client ${CMAKE_BINARY_DIR}/test/features/ijon/ijon-test_put2
)
set_tests_properties("ijon.run_test_put2" PROPERTIES ENVIRONMENT "N=10")

add_executable(
  ijon-run
  ijon.cpp 
)

target_link_libraries(
  ijon-run
  fuzzuf-cc-utils
  Boost::system
  Boost::unit_test_framework
)

target_include_directories(
  ijon-run
  PRIVATE
  ${FUZZUF_CC_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_dependencies(
  ijon-run
  fuzzuf-cc-for-test
)

set_target_properties(
  ijon-run
  PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS_STR}"
)
set_target_properties(
  ijon-run
  PROPERTIES LINK_FLAGS "${ADDITIONAL_LINK_FLAGS_STR}"
)

add_test(
  NAME
  "ijon.run"
  COMMAND
  ${CMAKE_BINARY_DIR}/test/features/ijon/ijon-run
)
set_tests_properties("ijon.run" PROPERTIES ENVIRONMENT "N=10")

