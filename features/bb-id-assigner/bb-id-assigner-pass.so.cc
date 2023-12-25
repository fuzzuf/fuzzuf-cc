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
#include "bb-id-assigner-pass.so.h"

#include <utility>
#include <iostream>
#include <random>
#include <cstdlib>

// Ignore warnings in llvm headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#pragma GCC diagnostic pop

using namespace llvm;

namespace bbidassigner {

const char* do_path_splitting_key = "BB_ID_ASSIGNER_DO_PATH_SPLITTING";

const char* generation_strategy_key = "BB_ID_ASSIGNER_ID_GENERATION_STRATEGY";
const char* sequential = "SEQUENTIAL";
const char* random = "RANDOM";

const char* selection_strategy_key = "BB_ID_ASSIGNER_NODE_SELECTION_STRATEGY";
const char* all = "ALL";
const char* no_multiple_pred = "NO_MULTIPLE_PRED";

const char* map_size_key = "BB_ID_ASSIGNER_MAP_SIZE";


AnalysisKey BBIDAssigner::Key;

BBIDAssigner::Result BBIDAssigner::run(Module &M, ModuleAnalysisManager&) {
  BBIDAssigner::Result res;

  if (getenv(do_path_splitting_key)) {
    for (auto &F : M){
      SplitAllCriticalEdges(F);
    }
  }

  char* selection_strategy_env_val = getenv(selection_strategy_key);
  std::function<bool(BasicBlock&)> selector;
  if (!selection_strategy_env_val || !strcmp(selection_strategy_env_val, all)) {
    selector = [](BasicBlock&) { return true; };
  }
  if (!selection_strategy_env_val || !strcmp(selection_strategy_env_val, no_multiple_pred)) {
    selector = [](BasicBlock& bb) {
      return !bb.hasNPredecessorsOrMore(2);
    };
  }

  int id, map_size;
  auto RNG = M.createRNG(name());

  char* generation_strategy_env_val = getenv(generation_strategy_key);
  std::function<int()> generator;
  if (!generation_strategy_env_val || !strcmp(generation_strategy_env_val, sequential)) {
    id = 0;
    generator = [&id]() {
      return id++;
    };
  }
  else if (!strcmp(generation_strategy_env_val, random)) {
    char* map_size_env_val = getenv(map_size_key);
    if (!map_size_env_val) {
      llvm::errs() << "[!] No map size is given.\n";
      report_fatal_error("Illegal environment variable");
    }
    map_size = atoi(map_size_env_val);
    if (map_size < 1) {
      llvm::errs() << "[!] Map size should be greater or equal than one.\n";
      report_fatal_error("Illegal environment variable");
    }
  
    generator = [&RNG, &map_size]() {
      std::uniform_int_distribution<uint32_t> dist(0, map_size - 1);
      return dist(*RNG);
    };
  }
  else {
    llvm::errs() << "[!] " << generation_strategy_env_val << " is not a valid option for the strategy.\n";
    report_fatal_error("Illegal environment variable");
  }

  for (auto &F : M){
    for (auto &BB : F) {
      if (!selector(BB)) continue;
      res.insert(std::make_pair(&BB, generator()));
    }
  }
  return res;
}

} // namespace bbidassigner

extern "C" LLVM_ATTRIBUTE_WEAK
PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "BasicBlockIDAssigner", "v0.2",
    [](PassBuilder &PB) {
      PB.registerAnalysisRegistrationCallback(
        [](ModuleAnalysisManager &MAM) {
          MAM.registerPass([] { return bbidassigner::BBIDAssigner(); });
        }
      );
    }
  };
}
