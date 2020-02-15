#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

# Load configuration
source ./configuration.zsh

#---------------------------------------------------------------------------------------------------

# Enter script directory
cd ${SCRIPT_DIR}

# Build LLVM pass
print_title "Building pass"
mkdir -p ${PASS_SOURCE_DIR}/build
cd ${PASS_SOURCE_DIR}/build
cmake ${PASS_SOURCE_DIR} -DCMAKE_PREFIX_PATH=${LLVM_BUILD_DIR} -G ${GENERATOR}
cmake --build .

# Return to script directory
cd ${SCRIPT_DIR}
