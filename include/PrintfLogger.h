// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include <llvm/Pass.h>

namespace llvm {

struct PrintfLogger : public ModulePass {
    PrintfLogger();
    bool runOnModule(Module& module) override;

    static char ID;
};

} // namespace llvm
