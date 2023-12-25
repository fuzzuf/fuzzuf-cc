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

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-afl-cc)
set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-afl-c++)

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
  test_put
  ../../../features/forkserver/test_put.c
)

add_dependencies(
  test_put
  fuzzuf-cc-for-test
)


#target_link_libraries(
#  test_put
#  forkserver
#)

add_test(
  NAME
  "forkserver.run"
  COMMAND
  ${CMAKE_BINARY_DIR}/features/forkserver/benchmark_client ${CMAKE_BINARY_DIR}/test/features/forkserver/test_put
)
set_tests_properties("forkserver.run" PROPERTIES ENVIRONMENT "N=1000")

add_test(
  NAME
  "forkserver.stdin"
  COMMAND
  flock /tmp/fuzzuf-cc_test_forkserver_stdin.lock ${CMAKE_CURRENT_SOURCE_DIR}/stdin_test.sh ${CMAKE_BINARY_DIR}
)
set_tests_properties("forkserver.stdin" PROPERTIES ENVIRONMENT "N=10")

add_test(
  NAME
  "forkserver.stdout"
  COMMAND
  flock /tmp/fuzzuf-cc_test_forkserver_stdout.lock ${CMAKE_CURRENT_SOURCE_DIR}/stdout_test.sh ${CMAKE_BINARY_DIR}
)
set_tests_properties("forkserver.stdout" PROPERTIES ENVIRONMENT "N=10")
