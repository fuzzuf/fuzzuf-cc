/*
   american fuzzy lop - LLVM-mode instrumentation pass
   ---------------------------------------------------

   Written by Laszlo Szekeres <lszekeres@google.com> and
              Michal Zalewski <lcamtuf@google.com>

   LLVM integration design comes from Laszlo Szekeres. C bits copied-and-pasted
   from afl-as.c are Michal's fault.

   Copyright 2015, 2016 Google Inc. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

   This library is plugged into LLVM when invoking clang through afl-clang-fast.
   It tells the compiler to add code roughly equivalent to the bits discussed
   in ../afl-as.h.

 */

#define AFL_LLVM_PASS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bb-id-assigner-pass.so.h"

// Ignore warnings in llvm headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#pragma GCC diagnostic pop

using namespace llvm;

#define DEBUG_TYPE "bitmap"

namespace {
  struct AFLCoverage : public PassInfoMixin<AFLCoverage> {
    static bool isRequired() { return true; }
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  };
}

STATISTIC(NumInstBlocks, "Number of instrumented basic blocks");
STATISTIC(NumSkippedBlocks, "Number of basic blocks that skipped instrumentation");

const char* instrumentation_method_key = "BITMAP_INSTRUMENTATION_METHOD";
const char* hashed_edge = "HASHED_EDGE";
const char* node = "NODE";

const char* update_method_key = "BITMAP_UPDATE_METHOD";
const char* naive = "NAIVE";
const char* avoid_zero = "AVOID_ZERO";
const char* cap_ff = "CAP_FF";

const char* pass_load_point_key = "BITMAP_PASS_LOAD_POINT";
const char* optimizer_last = "OptimizerLast";
const char* full_link_time_optimization_last = "FullLinkTimeOptimizationLast";

const char* area_ptr_valname = "__bitmap_area_ptr";
const char* prev_loc_valname = "__bitmap_prev_loc";

PreservedAnalyses AFLCoverage::run(Module &M, ModuleAnalysisManager& MAM) {
  LLVMContext &C = M.getContext();

  auto setNoSanitize = [&M, &C](Instruction* inst) {
    inst->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  };

  IntegerType *Int8Ty  = IntegerType::getInt8Ty(C);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  ConstantInt *Zero    = ConstantInt::get(Int8Ty, 0);
  ConstantInt *One     = ConstantInt::get(Int8Ty, 1);
  ConstantInt *FF      = ConstantInt::get(Int8Ty, 0xff);

  /* Get globals for the SHM region and the previous location. Note that
     __bitmap_prev_loc is thread-local. */

  GlobalVariable *AFLMapPtr =
      new GlobalVariable(M, PointerType::get(Int8Ty, 0), false,
                         GlobalValue::ExternalLinkage, 0, area_ptr_valname);
  GlobalVariable *AFLPrevLoc;

  std::function<Value*(unsigned int, IRBuilderBase&)> getBitmapLocation;
  char* instrumentation_method_env = getenv(instrumentation_method_key);
  if (!instrumentation_method_env || !strcmp(instrumentation_method_env, hashed_edge)) {
    AFLPrevLoc = new GlobalVariable(
      M, Int32Ty, false, GlobalValue::ExternalLinkage, 0, prev_loc_valname,
      0, GlobalVariable::GeneralDynamicTLSModel, 0, false);

    getBitmapLocation = [&AFLPrevLoc, &setNoSanitize](unsigned int cur_loc, IRBuilderBase& IRB) {
      ConstantInt *CurLoc = ConstantInt::get(IRB.getInt32Ty(), cur_loc);
      LoadInst *PrevLoc = IRB.CreateLoad(IRB.getInt32Ty(), AFLPrevLoc); setNoSanitize(PrevLoc);
      Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());
      StoreInst *Store = IRB.CreateStore(ConstantInt::get(IRB.getInt32Ty(), cur_loc >> 1), AFLPrevLoc); setNoSanitize(Store);
      return IRB.CreateXor(PrevLocCasted, CurLoc);
    };
  }
  else if (!strcmp(instrumentation_method_env, node)) {
    getBitmapLocation = [](unsigned int cur_loc, IRBuilderBase& IRB) {
      return ConstantInt::get(IRB.getInt32Ty(), cur_loc);
    };
  }
  else {
    llvm::errs() << "[!] ID method " << instrumentation_method_env << " is not known\n";
    report_fatal_error("Unknown ID method");
  }


  std::function<void(Value*, IRBuilderBase&)> updateBitmap;
  char* update_method_env = getenv(update_method_key);
  if (!update_method_env || !strcmp(update_method_env, naive)) {
    updateBitmap = [&M, &C, &One, &setNoSanitize](Value* ptr, IRBuilderBase& IRB) {
      LoadInst *Counter = IRB.CreateLoad(IRB.getInt8Ty(), ptr); setNoSanitize(Counter);
      Value *Incr = IRB.CreateAdd(Counter, One);
      StoreInst *StoreCounter = IRB.CreateStore(Incr, ptr); setNoSanitize(StoreCounter);
    };
  }
  else if (!strcmp(update_method_env, avoid_zero)) {
    updateBitmap = [&M, &C, &Zero, &One, &setNoSanitize](Value* ptr, IRBuilderBase& IRB) {
      LoadInst *Counter = IRB.CreateLoad(IRB.getInt8Ty(), ptr); setNoSanitize(Counter);
      Value *Incr = IRB.CreateAdd(Counter, One);
      Value* Cf = IRB.CreateICmpEQ(Incr, Zero);
      Value* Carry = IRB.CreateZExt(Cf, IRB.getInt8Ty());
      Incr = IRB.CreateAdd(Incr, Carry);
      StoreInst *StoreCounter = IRB.CreateStore(Incr, ptr); setNoSanitize(StoreCounter);
    };
  }
  else if (!strcmp(update_method_env, cap_ff)) {
    updateBitmap = [&M, &C, &FF, &setNoSanitize](Value* ptr, IRBuilderBase& IRB) {
      LoadInst *Counter = IRB.CreateLoad(IRB.getInt8Ty(), ptr); setNoSanitize(Counter);
      Value* Adder = IRB.CreateICmpNE(Counter, FF);
      Value* Carry = IRB.CreateZExt(Adder, IRB.getInt8Ty());
      Value* Incr = IRB.CreateAdd(Counter, Carry);
      StoreInst *StoreCounter = IRB.CreateStore(Incr, ptr); setNoSanitize(StoreCounter);
    };
  }
  else {
    llvm::errs() << "[!] Update method " << update_method_env << " is not known\n";
    report_fatal_error("Unknown Update Method");
  }


  /* Instrument all the things! */
  DenseMap<BasicBlock*, unsigned int>& bb_ids = MAM.getResult<bbidassigner::BBIDAssigner>(M);
  for (auto &F : M) {
    for (auto &BB : F) {
      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&(*IP));

      if (!bb_ids.count(&BB)) {
        NumSkippedBlocks++;
        continue;
      }
      unsigned int cur_loc = bb_ids[&BB];

      Value *BitMapLoc = getBitmapLocation(cur_loc, IRB);
      LoadInst *MapPtr = IRB.CreateLoad(IRB.getInt8PtrTy(), AFLMapPtr); setNoSanitize(MapPtr);
      Value *MapPtrIdx = IRB.CreateGEP(IRB.getInt8Ty(), MapPtr, BitMapLoc);
      updateBitmap(MapPtrIdx, IRB);

      NumInstBlocks++;
    }
  }

  PreservedAnalyses PA;
  PA.preserve<bbidassigner::BBIDAssigner>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}


extern "C" LLVM_ATTRIBUTE_WEAK
PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "AFLCoverage", "v0.2",
    [](PassBuilder &PB) {
#if LLVM_VERSION_MAJOR <= 13
      using OptimizationLevel = typename PassBuilder::OptimizationLevel;
#endif

      char* env = getenv(pass_load_point_key);
      if (!env) return;
      if (strstr(env, optimizer_last)) {
        PB.registerOptimizerLastEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel) {
            MPM.addPass(AFLCoverage());
          }
        );
      }
      if (strstr(env, full_link_time_optimization_last)) {        
#if LLVM_VERSION_MAJOR >= 15
          PB.registerFullLinkTimeOptimizationLastEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel) {
            MPM.addPass(AFLCoverage());
          }
        );
#else
        errs() <<  "[!] Link Time Optimization is not supported. Please use LLVM to version 15 or above in order to use.\n";
#endif
      }
    }
  };
}
