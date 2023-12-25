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
#ifndef BB_ID_ASSIGNER_PASS_H
#define BB_ID_ASSIGNER_PASS_H

// Ignore warnings in llvm headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/PassManager.h"
#pragma GCC diagnostic pop

namespace bbidassigner {

using namespace llvm;

struct BBIDAssigner : public AnalysisInfoMixin<BBIDAssigner> {
  using Result = DenseMap<BasicBlock*, unsigned int>;
  Result run(Module&, ModuleAnalysisManager&);
  static AnalysisKey Key;
};

}

#endif
