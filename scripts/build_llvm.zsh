#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

# Load configuration
source ./configuration.zsh

#---------------------------------------------------------------------------------------------------

# Create build directory
mkdir -p ${LLVM_BUILD_DIR}
cd ${LLVM_BUILD_DIR}

# Generate build files
print_title "Configuring LLVM"
cmake ${LLVM_SOURCE_DIR} -G ${GENERATOR}    \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=On    \
      -DCMAKE_BUILD_TYPE=Debug              \
      -DBUILD_SHARED_LIBS=On                \
      -DLLVM_ENABLE_ASSERTIONS=On           \
      -DLLVM_TARGETS_TO_BUILD="X86"         \
      -DLLVM_ENABLE_SPHINX=Off              \
      -DLLVM_ENABLE_THREADS=On              \
      -DLLVM_INSTALL_UTILS=On

# Build LLVM
print_title "Building LLVM"
cmake --build .

# Return to script directory
cd ${SCRIPT_DIR}
