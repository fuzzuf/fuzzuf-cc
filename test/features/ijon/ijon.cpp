/*
 * fuzzuf-cc
 * Copyright (C) 2022-2023 Ricerca Security
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#define BOOST_TEST_MODULE feature.ijon.ijon
#define BOOST_TEST_DYN_LINK

#include <boost/range/iterator_range.hpp>
#include <boost/scope_exit.hpp>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "features/ijon/config.h"
#include "fuzzuf_cc/utils/create_shared_memory.hpp"
#include "fuzzuf_cc/utils/executor.hpp"

/**
 * Execute PUT instrumented for IJON and check if IJON related values are
 * stored. This test uses input value that is expected to exit successfully.
 */
BOOST_AUTO_TEST_CASE(IJON) {
  // setup
  const std::size_t afl_coverage_size = 65536u;
  const std::size_t ijon_counter_size = 65536u;
  const std::size_t ijon_max_size = 65536u;
  auto [shm_env, shm] = fuzzuf_cc::utils::create_shared_memory(
      AFL_SHM_ENV_VAR, afl_coverage_size + ijon_counter_size + ijon_max_size);
  std::string afl_coverage_size_str("__AFL_SIZE=");
  afl_coverage_size_str += std::to_string(afl_coverage_size);
  std::string ijon_counter_size_str("__IJON_COUNTER_SIZE=");
  ijon_counter_size_str += std::to_string(ijon_counter_size);
  std::string ijon_max_size_str("__IJON_MAX_SIZE=");
  ijon_max_size_str += std::to_string(ijon_max_size);
  std::vector<std::byte> input{std::byte(1), std::byte(2), std::byte(3),
                               std::byte(4), std::byte(5), std::byte(6),
                               std::byte(7), std::byte(8)};
  fuzzuf_cc::utils::Executor e(
      {TEST_BINARY_DIR "/features/ijon/ijon-test_put1"},
      {shm_env, afl_coverage_size_str, ijon_counter_size_str,
       ijon_max_size_str});
  auto [result, standard_output, standard_error] = e(std::move(input));

  // At least one edge is found
  BOOST_CHECK(std::size_t(std::count(
                  shm.begin(), std::next(shm.begin(), afl_coverage_size), 0)) !=
              afl_coverage_size);
  // At least one IJON counter is incremented
  BOOST_CHECK_EQUAL(
      std::size_t(std::count(
          std::next(shm.begin(), afl_coverage_size),
          std::next(shm.begin(), afl_coverage_size + ijon_counter_size), 0)),
      ijon_counter_size - 2);
  // At least one IJON_MAX value is recorded
  BOOST_CHECK_EQUAL(
      std::size_t(std::count(
          reinterpret_cast<std::uint64_t*>(std::next(
              shm.begin().get(), afl_coverage_size + ijon_counter_size)),
          reinterpret_cast<std::uint64_t*>(
              std::next(shm.begin().get(),
                        afl_coverage_size + ijon_counter_size + ijon_max_size)),
          0)),
      ijon_max_size / sizeof(std::uint64_t) - 2);

  // Check if PUT exited successfully
  BOOST_CHECK_EQUAL(result.error, ExecutePUTError::None);
  BOOST_CHECK_EQUAL(result.exit_code, 0);

  // Check if standard output is not empty
  BOOST_CHECK(standard_output.size() != 0);
  // Check if standard error is empty
  BOOST_CHECK_EQUAL(standard_error.size(), 0);
}
/**
 * Execute PUT instrumented for IJON and check if IJON related values are
 * stored. This test uses input value that is expected to abort.
 */
BOOST_AUTO_TEST_CASE(IJONAbort) {
  // setup
  const std::size_t afl_coverage_size = 65536u;
  const std::size_t ijon_counter_size = 65536u;
  const std::size_t ijon_max_size = 65536u;
  auto [shm_env, shm] = fuzzuf_cc::utils::create_shared_memory(
      AFL_SHM_ENV_VAR, afl_coverage_size + ijon_counter_size + ijon_max_size);
  std::string afl_coverage_size_str("__AFL_SIZE=");
  afl_coverage_size_str += std::to_string(afl_coverage_size);
  std::string ijon_counter_size_str("__IJON_COUNTER_SIZE=");
  ijon_counter_size_str += std::to_string(ijon_counter_size);
  std::string ijon_max_size_str("__IJON_MAX_SIZE=");
  ijon_max_size_str += std::to_string(ijon_max_size);
  std::vector<std::byte> input{std::byte(17), std::byte(0), std::byte(0),
                               std::byte(0),  std::byte(3), std::byte(4),
                               std::byte(0),  std::byte(0)};
  fuzzuf_cc::utils::Executor e(
      {TEST_BINARY_DIR "/features/ijon/ijon-test_put1"},
      {shm_env, afl_coverage_size_str, ijon_counter_size_str,
       ijon_max_size_str});
  auto [result, standard_output, standard_error] = e(std::move(input));

  // At least one edge is found
  BOOST_CHECK(std::size_t(std::count(
                  shm.begin(), std::next(shm.begin(), afl_coverage_size), 0)) !=
              afl_coverage_size);
  // At least one IJON counter is incremented
  BOOST_CHECK_EQUAL(
      std::size_t(std::count(
          std::next(shm.begin(), afl_coverage_size),
          std::next(shm.begin(), afl_coverage_size + ijon_counter_size), 0)),
      ijon_counter_size - 3);
  // At least one IJON_MAX value is recorded
  BOOST_CHECK_EQUAL(
      std::size_t(std::count(
          reinterpret_cast<std::uint64_t*>(std::next(
              shm.begin().get(), afl_coverage_size + ijon_counter_size)),
          reinterpret_cast<std::uint64_t*>(
              std::next(shm.begin().get(),
                        afl_coverage_size + ijon_counter_size + ijon_max_size)),
          0)),
      ijon_max_size / sizeof(std::uint64_t) - 2);

  // Check if PUT aborted
  BOOST_CHECK_EQUAL(result.error, ExecutePUTError::None);
  BOOST_CHECK(result.exit_code != 0);

  // Check if standard output is empty
  BOOST_CHECK_EQUAL(standard_output.size(), 0);
  // Check if standard error is not empty
  BOOST_CHECK(standard_error.size() != 0);
}
