#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

#---------------------------------------------------------------------------------------------------
#------------------------------------ START USER CONFIGURATION -------------------------------------
#---------------------------------------------------------------------------------------------------

# Directory of the LLVM source code
LLVM_SOURCE_DIR=

# Build directory of LLVM
LLVM_BUILD_DIR=

# CMake generator to use (e.g. Ninja or Xcode)
GENERATOR=

# Clang executable
CLANG=

#---------------------------------------------------------------------------------------------------
#------------------------------------- END USER CONFIGURATION --------------------------------------
#---------------------------------------------------------------------------------------------------

# LLVM binaries
LLVM_BIN_DIR=${LLVM_BUILD_DIR}/bin
LLC=${LLVM_BIN_DIR}/llc
OPT=${LLVM_BIN_DIR}/opt

# Pass name and source directory
if [[ ${CURRENT_WORKING_DIR} ]]
then
    PASS_SOURCE_DIR=${CURRENT_WORKING_DIR}
else
    PASS_SOURCE_DIR=$(pwd)/..
fi
PASS_NAME=log-printf-calls

# Dynamic library extension
SYSTEM_ID=$(uname -s)
if [[ ${SYSTEM_ID} == "Darwin" ]]; then
    DYNAMIC_LIB_EXTENSION=dylib
elif [[ ${SYSTEM_ID} == "Linux" ]]; then
    DYNAMIC_LIB_EXTENSION=so
else
    echo "Unsupported platform"
    exit 1
fi

# Pass dynamic library
PASS_LOADABLE_MODULE=${PASS_SOURCE_DIR}/lib/LLVMPrintfLogger.${DYNAMIC_LIB_EXTENSION}

# Script directory
SCRIPT_DIR=${PASS_SOURCE_DIR}/scripts

# Logfile directory and name
LOGFILE_DIR=${PASS_SOURCE_DIR}/logs
LOGFILE=${LOGFILE_DIR}/log.txt

# Color definitions
COLOR_WHITE="\033[1;37m"
COLOR_NONE="\033[0m"

# Helper function for printing formatted titles
print_title() {
    echo "$COLOR_WHITE> $1 $COLOR_NONE"
}
