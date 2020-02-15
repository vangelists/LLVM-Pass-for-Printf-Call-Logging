#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

# Pass root directory
CURRENT_WORKING_DIR=$(pwd)

# Load configuration
source scripts/configuration.zsh

#---------------------------------------------------------------------------------------------------

# Enter script directory
cd ${SCRIPT_DIR}

# Clean up
print_title "CLEANING UP WORKSPACE"
source ./clean_up.zsh
print_title "DONE\n"

# Build LLVM
print_title "BUILDING LLVM"
source ./build_llvm.zsh
print_title "DONE\n"

# Build LLVM pass
print_title "BUILDING LLVM PASS"
source ./build_pass.zsh
print_title "DONE\n"

# Run test
print_title "RUNNING TEST"
source ./run_test.zsh $1
print_title "DONE"
