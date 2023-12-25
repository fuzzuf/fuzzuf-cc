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
#define BOOST_TEST_MODULE feature.bitmap.update_method
#define BOOST_TEST_DYN_LINK

#include <boost/range/iterator_range.hpp>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <cstdint>

#include "config.h"
#include "features/ijon/config.h"
#include "fuzzuf_cc/utils/create_shared_memory.hpp"
#include "fuzzuf_cc/utils/executor.hpp"

void CheckCoverage(std::string binary_path, std::uint8_t expected_count) {
  // setup
  const std::size_t afl_coverage_size = 65535u;
  auto [shm_env, shm] = fuzzuf_cc::utils::create_shared_memory(
      AFL_SHM_ENV_VAR, afl_coverage_size);

  fuzzuf_cc::utils::Executor e({binary_path}, {shm_env});
  auto [result, standard_output, standard_error] = e({});

  // check if PUT exited successfully
  BOOST_CHECK_EQUAL(result.error, ExecutePUTError::None);
  BOOST_CHECK_EQUAL(result.exit_code, 0);

  // check if bitmap[0] is equal to expected_count
  BOOST_CHECK_EQUAL(shm[0], expected_count);
}

BOOST_AUTO_TEST_CASE(BitmapUpdateMethod) {
  CheckCoverage(TEST_BINARY_DIR "/features/bitmap/put-update-method-naive",
                0x00);

  CheckCoverage(TEST_BINARY_DIR "/features/bitmap/put-update-method-avoid-zero",
                0x01);

  CheckCoverage(TEST_BINARY_DIR "/features/bitmap/put-update-method-cap-ff",
                0xff);
}
