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
#include <iomanip>
#include <random>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bb-id-assigner-pass.so.h"

// Ignore warnings in llvm headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCSection.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#pragma GCC diagnostic pop

#define EDGE_JMP      0
#define EDGE_INDIRECT 1

using namespace llvm;

namespace cfgexporter {
  struct CFGExporter : public PassInfoMixin<CFGExporter> {
    CFGExporter() {}
    static bool isRequired() { return true; }
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  };
}

// bool cfgexporter::CFGExporter::connect_functions;

const char* pass_load_point_key = "CFG_EXPORTER_LOAD_POINT";
const char* optimizer_last = "OptimizerLast";
const char* full_link_time_optimization_last = "FullLinkTimeOptimizationLast";

PreservedAnalyses cfgexporter::CFGExporter::run(Module &M, ModuleAnalysisManager& MAM) {
  DenseMap<BasicBlock*, unsigned int>* bb_ids_ptr = MAM.getCachedResult<bbidassigner::BBIDAssigner>(M);

  if (!bb_ids_ptr) {
    errs() << "[!] Cached result not found. This pass was likely executed before the result was generated, or after the cache was invalidated due to the code changes.\n";
    report_fatal_error("Cached result not found");
  }

  auto bb_ids = *bb_ids_ptr;

  std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> edges;
  std::unordered_set<Function*> func_considered;

  auto add_edge = [&](unsigned int u, unsigned int v, unsigned int t) {
    edges.emplace_back(u, v, t);
  };

  for (auto &F : M) {
    for (auto &BB : F) {
      if (!bb_ids.count(&BB)) {
        dbgs() << "[*] BB not found: " << BB << "\n";
        continue;
      }

      unsigned v = bb_ids[&BB];

      /* First, add edges between vertices
          in the same function(i.e. edges of "jmp") */

      for (auto *C : successors(&BB)) {
        auto itr = bb_ids.find(C);
        if (itr == bb_ids.end()) {
          dbgs() << "[*] BB not found: " << C << "\n";
          continue;
        }

        unsigned int u = itr->second;
        add_edge(v, u, EDGE_JMP);
      }

      /* Second, add edges between vertices
          in different functions(i.e. edges of "call")  */

      for (Instruction &I : BB) {
        if (auto *CB = dyn_cast<CallBase>(&I)) {
          if (auto func = CB->getCalledFunction()) {
            if (func->empty()) { // for some reason it's empty sometimes
                continue;
            }

            auto &C = func->getEntryBlock();
            auto itr = bb_ids.find(&C);
            if (itr == bb_ids.end()) {
              dbgs() << "[*] function entry block not found: " << func->getName().str().c_str() << "\n";
              continue;
            }

            unsigned int u = itr->second;
            add_edge(v, u, EDGE_INDIRECT);
          }
        }
      }
    }
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned long long> distribution(0, UINT64_MAX);
  unsigned long long cfg_id = distribution(gen);

  std::ostringstream section_name_ss;
  section_name_ss << ".cfg-" <<  std::setfill('0') << std::setw(16) << std::hex << cfg_id;

  std::ostringstream sym_name_ss;
  sym_name_ss << "__cfg_" <<  std::setfill('0') << std::setw(16) << std::hex << cfg_id;

  std::ostringstream content_ss;
  for (auto [u, v, t] : edges) {
    content_ss << u << ' ' << v << ' ' << t << '\n';
  }

  Constant *msg = ConstantDataArray::getString(M.getContext(), content_ss.str());

  auto val = new GlobalVariable(M, msg->getType(), true, GlobalValue::InternalLinkage, msg, sym_name_ss.str());
  val->setSection(section_name_ss.str());
  appendToUsed(M, {val});

  return PreservedAnalyses::all();
}


extern "C" LLVM_ATTRIBUTE_WEAK
PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "CFGExporter", "v0.2",
    [](PassBuilder &PB) {
#if LLVM_VERSION_MAJOR <= 13
      using OptimizationLevel = typename PassBuilder::OptimizationLevel;
#endif

      char* env = getenv(pass_load_point_key);
      if (!env) return;
      if (strstr(env, optimizer_last)) {
        PB.registerOptimizerLastEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel) {
            MPM.addPass(cfgexporter::CFGExporter());
          }
        );
      }
      if (strstr(env, full_link_time_optimization_last)) {        
#if LLVM_VERSION_MAJOR >= 15
          PB.registerFullLinkTimeOptimizationLastEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel) {
            MPM.addPass(cfgexporter::CFGExporter());
          }
        );
#else
        errs() <<  "[!] Link Time Optimization is not supported. Please use LLVM to version 15 or above in order to use.\n";
#endif
      }
    }
  };
}
