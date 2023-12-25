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

set(CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/fuzzuf-cc)
set(CMAKE_C_FLAGS "-O0 --features bb-id-assigner,bitmap-feedback,cfg-exporter,forkserver --bb-id-assigner-id-generation-strategy SEQUENTIAL")

enable_testing()

add_executable(
  put-branch
  ../../../put/branch/branch.c
)

add_dependencies(
  put-branch
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "cfg-exporter.jmp"
  COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}/put-branch ${CMAKE_CURRENT_SOURCE_DIR}/put-branch.cfg.expected
)

add_executable(
  put-loopff
  ../../../put/loop_ff/loop_ff.c
)

add_dependencies(
  put-loopff
  fuzzuf-cc-for-test
)

add_test(
  NAME
  "cfg-exporter.indirect"
  COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_CURRENT_BINARY_DIR}/put-loopff ${CMAKE_CURRENT_SOURCE_DIR}/put-loopff.cfg.expected
)
