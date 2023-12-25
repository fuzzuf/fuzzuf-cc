/*
 * fuzzuf-cc
 * Copyright (C) 2023 Ricerca Security
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
#define BOOST_TEST_MODULE feature.bitmap.bitmap
#define BOOST_TEST_DYN_LINK

#include <boost/range/iterator_range.hpp>
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

using SharedMemory =
    boost::iterator_range<fuzzuf_cc::utils::range::shared_iterator<
        std::uint8_t *, std::shared_ptr<void>>>;

struct DiffInfo {
  std::vector<int> passed;
  std::vector<int> ignored;
};

enum InstrumentationMethod {
  HASHED_EDGE,
  NODE,
};

/*
   throughout this PUT, we assume there is a mapping between seeds(or input)
   and integers (bit_seq) the mapping is just the same as little endian. for
   example, if bit_seq is 4, then the corresponding input would be "00100000" in
   the follwing comments, sometimes we refer seeds like "the seed corresponding
   to 4"
*/
std::unordered_map<int, u8> GetCoverage(fuzzuf_cc::utils::Executor &e,
                                        SharedMemory &shm, int bit_seq) {
  // create seed corresponding to bit_seq
  std::vector<std::byte> input;
  for (int i = 0; i < 8; i++) {
    int d = bit_seq >> i & 1;
    input.push_back(std::byte('0' + d));
  }
  input.push_back(std::byte('\n'));

  std::fill(shm.begin(), shm.end(), 0);
  auto [result, standard_output, standard_error] = e(std::move(input));

  // check if PUT exited successfully
  BOOST_CHECK_EQUAL(result.error, ExecutePUTError::None);
  BOOST_CHECK_EQUAL(result.exit_code, 0);

  std::unordered_map<int, u8> trace;
  for (std::size_t i = 0; i < shm.size(); i++) {
    if (shm[i]) {
      trace[i] = shm[i];
    }
  }
  return trace;
}

// execute instrumented PUT and check if bitmap values are correctly stored.
static void CheckCoverage(std::string binary_path,
                          InstrumentationMethod inst_method) {
  // setup
  const std::size_t afl_coverage_size = 65535u;
  auto [shm_env, shm] = fuzzuf_cc::utils::create_shared_memory(
      AFL_SHM_ENV_VAR, afl_coverage_size);
  fuzzuf_cc::utils::Executor e({binary_path}, {shm_env});

  std::size_t expected_diffs;
  switch (inst_method) {
    case HASHED_EDGE:
      // the difference should be exactly two edges
      expected_diffs = 2;
      break;
    case NODE:
      // the difference should be exactly one node
      expected_diffs = 1;
      break;
  }

  // create "diffs" between 00000000 and 10000000, between 0000000 and
  // 01000000,
  // ...
  auto base_cov = GetCoverage(e, shm, 0);
  // coverages cannot be empty
  BOOST_CHECK(base_cov.size() > 0);

  DiffInfo diffs[8];
  for (int i = 0; i < 8; i++) {
    auto cov = GetCoverage(e, shm, 1 << i);
    // coverages cannot be empty
    BOOST_CHECK(cov.size() > 0);

    // into DiffInfo::passed, append basic blocks which the seed corresponding
    // to (1 << i) passes, but which the seed corresponding to 0 doesn't pass
    for (const auto &itr : cov) {
      if (!base_cov.count(itr.first)) diffs[i].passed.emplace_back(itr.first);
    }
    // into DiffInfo::ignored, append basic blocks which the seed corresponding
    // to (1 << i) doesn't pass, but which the seed corresponding to 0 passes
    for (const auto &itr : base_cov) {
      if (!cov.count(itr.first)) diffs[i].ignored.emplace_back(itr.first);
    }

    std::string name(8, '0');
    name[i] = '1';
    std::cout << "---" << name << "---\n";
    std::cout << "Hashes of edges which this input should pass:";
    for (int edge_idx : diffs[i].passed) {
      std::cout << " " << edge_idx;
    }
    std::cout << "\nHashes of edges which given input should not pass:";
    for (int edge_idx : diffs[i].ignored) {
      std::cout << " " << edge_idx;
    }
    std::cout << std::endl;

    // there must be some differences
    BOOST_CHECK(diffs[i].passed.size() > 0);
    BOOST_CHECK(diffs[i].ignored.size() > 0);

    // the number of differences should be equal to expected_diffs
    BOOST_CHECK_EQUAL(diffs[i].passed.size(), expected_diffs);
    BOOST_CHECK_EQUAL(diffs[i].ignored.size(), expected_diffs);
  }

  int lim = 1 << 8;
  for (int bit_seq = 0; bit_seq < lim; bit_seq++) {
    auto cov = GetCoverage(e, shm, bit_seq);
    for (int i = 0; i < 8; i++) {
      // check if the i-th bit counting from LSB is set
      bool is_ith_set = bit_seq >> i & 1;
      if (is_ith_set) {
        // the i-th bit of this seed is set
        // that means seed bit_seq should have passed edges in diffs[i].passed
        for (int edge_idx : diffs[i].passed) {
          BOOST_CHECK(cov[edge_idx] > 0);
        }
        // and that seed bit_seq should not have passed edges in
        // diffs[i].ignored
        for (int edge_idx : diffs[i].ignored) {
          BOOST_CHECK(cov[edge_idx] == 0);
        }
      } else {
        // the i-th bit of this seed is not set
        // that means seed bit_seq should have passed edges in
        // diffs[i].ignored
        for (int edge_idx : diffs[i].ignored) {
          BOOST_CHECK(cov[edge_idx] > 0);
        }
        // and that seed bit_seq should not have passed edges in
        // diffs[i].passed
        for (int edge_idx : diffs[i].passed) {
          BOOST_CHECK(cov[edge_idx] == 0);
        }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(Bitmap) {
  const std::vector<std::pair<std::string, InstrumentationMethod>> binaries = {
      {TEST_BINARY_DIR "/features/bitmap/put-zeroone-edge-cov", HASHED_EDGE},
      {TEST_BINARY_DIR "/features/bitmap/put-zeroone-node-cov", NODE},
  };

  for (const auto &binary : binaries) {
    auto [binary_path, inst_method] = binary;
    std::cout << binary_path << std::endl;
    CheckCoverage(binary_path, inst_method);
  }
}
