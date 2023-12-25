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
set(CMAKE_C_FLAGS "-O0 --features bb-id-assigner,bitmap-feedback,forkserver")

enable_testing()

add_executable(
  put-zeroone-edge-cov
  ../../../put/zeroone/zeroone.c
)

add_dependencies(
  put-zeroone-edge-cov
  fuzzuf-cc-for-test
)

add_executable(
  put-zeroone-node-cov
  ../../../put/zeroone/zeroone.c
)

add_dependencies(
  put-zeroone-node-cov
  fuzzuf-cc-for-test
)

set_target_properties(
  put-zeroone-node-cov
  PROPERTIES COMPILE_FLAGS "--bb-id-assigner-path-splitting --bb-id-assigner-node-selection-strategy NO_MULTIPLE_PRED --bitmap-instrumentation-method NODE"
)

add_executable(
  test-bitmap-run
  bitmap.cpp
)

target_link_libraries(
  test-bitmap-run
  fuzzuf-cc-utils
  Boost::system
  Boost::unit_test_framework
)

target_include_directories(
  test-bitmap-run
  PRIVATE
  ${FUZZUF_CC_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_dependencies(
  test-bitmap-run
  fuzzuf-cc-for-test
)

set_target_properties(
  test-bitmap-run
  PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS_STR}"
)

set_target_properties(
  test-bitmap-run
  PROPERTIES LINK_FLAGS "${ADDITIONAL_LINK_FLAGS_STR}"
)

add_test(
  NAME
  "bitmap.run"
  COMMAND
  ${CMAKE_BINARY_DIR}/test/features/bitmap/test-bitmap-run
)

add_executable(
  put-update-method-naive
  ../../../put/loop_ff/loop_ff.c
)

add_dependencies(
  put-update-method-naive
  fuzzuf-cc-for-test
)

set_target_properties(
  put-update-method-naive
  PROPERTIES COMPILE_FLAGS "--bb-id-assigner-id-generation-strategy SEQUENTIAL --bitmap-instrumentation-method NODE --bitmap-update-method NAIVE"
)

add_executable(
  put-update-method-avoid-zero
  ../../../put/loop_ff/loop_ff.c
)

add_dependencies(
  put-update-method-avoid-zero
  fuzzuf-cc-for-test
)

set_target_properties(
  put-update-method-avoid-zero
  PROPERTIES COMPILE_FLAGS "--bb-id-assigner-id-generation-strategy SEQUENTIAL --bitmap-instrumentation-method NODE --bitmap-update-method AVOID_ZERO"
)

add_executable(
  put-update-method-cap-ff
  ../../../put/loop_ff/loop_ff.c
)

add_dependencies(
  put-update-method-cap-ff
  fuzzuf-cc-for-test
)

set_target_properties(
  put-update-method-cap-ff
  PROPERTIES COMPILE_FLAGS "--bb-id-assigner-id-generation-strategy SEQUENTIAL --bitmap-instrumentation-method NODE --bitmap-update-method CAP_FF"
)

add_executable(
  test-bitmap-update-method
  update_method.cpp
)

target_link_libraries(
  test-bitmap-update-method
  fuzzuf-cc-utils
  Boost::system
  Boost::unit_test_framework
)

target_include_directories(
  test-bitmap-update-method
  PRIVATE
  ${FUZZUF_CC_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_dependencies(
  test-bitmap-update-method
  fuzzuf-cc-for-test
)

set_target_properties(
  test-bitmap-update-method
  PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS_STR}"
)

set_target_properties(
  test-bitmap-update-method
  PROPERTIES LINK_FLAGS "${ADDITIONAL_LINK_FLAGS_STR}"
)

add_test(
  NAME
  "bitmap.update-method"
  COMMAND
  ${CMAKE_BINARY_DIR}/test/features/bitmap/test-bitmap-update-method
)
