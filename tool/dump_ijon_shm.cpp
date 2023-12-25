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
/**
 * @file dump_ijon_shm.cpp
 * @author Ricerca Security <fuzzuf-dev@ricsec.co.jp>
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "fuzzuf_cc/utils/create_shared_memory.hpp"
#include "fuzzuf_cc/utils/executor.hpp"
#include "features/ijon/config.h"

// This tool executes an IJON instrumented binary and dumps the contents of
// shared memory
int main(int argc, char* const argv[]) {
  std::size_t afl_coverage_size = 65536u;
  std::size_t ijon_counter_size = 65536u;
  std::size_t ijon_max_size = 65536u;
  auto [shm_env, shm] = fuzzuf_cc::utils::create_shared_memory(
      AFL_SHM_ENV_VAR, afl_coverage_size + ijon_counter_size + ijon_max_size);
  std::fill(shm.begin(), shm.end(), 0);
  std::string afl_coverage_size_str("__AFL_SIZE=");
  afl_coverage_size_str += std::to_string(afl_coverage_size);
  std::string ijon_counter_size_str("__IJON_COUNTER_SIZE=");
  ijon_counter_size_str += std::to_string(ijon_counter_size);
  std::string ijon_max_size_str("__IJON_MAX_SIZE=");
  ijon_max_size_str += std::to_string(ijon_max_size);
  std::vector<std::string> args;
  args.reserve(argc - 1);
  for (auto i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  fuzzuf_cc::utils::Executor e(
      std::move(args), {shm_env, afl_coverage_size_str, ijon_counter_size_str,
                        ijon_max_size_str});
  std::string input("Hello, World!");
  std::vector<std::byte> input_u;
  std::transform(input.begin(), input.end(), std::back_inserter(input_u),
                 [](auto v) { return std::byte(v); });
  auto [result, standard_output, standard_error] = e(std::move(input_u));
  {
    std::size_t i = 0u;
    for (const auto v : shm) {
      if (v) {
        std::cout << "shm[" << i << "] = " << int(v) << std::endl;
      }
      ++i;
    }
  }
  std::cout << "result : " << int(result.exit_code) << std::endl;
  std::string standard_output_u;
  std::transform(standard_output.begin(), standard_output.end(),
                 std::back_inserter(standard_output_u),
                 [](auto v) { return char(v); });
  std::cout << "stdout : " << standard_output_u << std::endl;
  std::string standard_error_u;
  std::transform(standard_error.begin(), standard_error.end(),
                 std::back_inserter(standard_error_u),
                 [](auto v) { return char(v); });
  std::cout << "stderr : " << standard_error_u << std::endl;
}
