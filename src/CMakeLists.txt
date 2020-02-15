# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

set(LLVM_LINK_COMPONENTS Core Support TransformUtils)
add_llvm_loadable_module(LLVMPrintfLogger PrintfLogger.cpp)
