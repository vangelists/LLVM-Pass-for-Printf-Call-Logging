#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

# Load configuration
source ./configuration.zsh

#---------------------------------------------------------------------------------------------------

# Enter script directory
cd ${SCRIPT_DIR}

# Clean up
print_title "Cleaning up"
rm -rf ${PASS_SOURCE_DIR}/build
rm -rf ${PASS_SOURCE_DIR}/lib
rm -rf ${PASS_SOURCE_DIR}/logs
