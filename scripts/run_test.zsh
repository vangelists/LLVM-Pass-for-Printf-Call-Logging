#!/bin/zsh
#
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

# Load configuration
source ./configuration.zsh

#---------------------------------------------------------------------------------------------------

if [[ $1 != "" ]]; then
    TEST_NAME=$1
else
    TEST_NAME=Random
fi

TEST_DIR=${PASS_SOURCE_DIR}/test
TEST_PATH=${TEST_DIR}/${TEST_NAME}
MODIFIED_TEST_PATH=${TEST_PATH}_logging

#---------------------------------------------------------------------------------------------------

# Enter script directory
cd ${SCRIPT_DIR}

# Build test
print_title "Building test"
# Optimizations disabled completely to avoid replacement of printf() calls with puts()
${CLANG} -std=c++17 -O0 -fcxx-exceptions -emit-llvm ${TEST_PATH}.cpp -c -o ${TEST_PATH}.bc
${LLC} -filetype=obj ${TEST_PATH}.bc
${CLANG} ${TEST_PATH}.o -o ${TEST_PATH}

# Delete previous logfile
print_title "Deleting previous logfile"
rm -rf ${LOGFILE_DIR}
mkdir -p ${LOGFILE_DIR}
cd ${LOGFILE_DIR}

# Run test
print_title "Running test"
ORIGINAL_TEST_OUTPUT=$(${TEST_PATH})
echo "$ORIGINAL_TEST_OUTPUT"

# Run pass on test
print_title "Running pass on test"
${OPT} -O3 -load ${PASS_LOADABLE_MODULE} -${PASS_NAME} < ${TEST_PATH}.bc > ${MODIFIED_TEST_PATH}.bc
${LLC} -filetype=obj ${MODIFIED_TEST_PATH}.bc
${CLANG} ${MODIFIED_TEST_PATH}.o -o ${MODIFIED_TEST_PATH}

# Run modified test
print_title "Running modified test"
MODIFIED_TEST_OUTPUT=$(${TEST_PATH}_logging)
echo "$MODIFIED_TEST_OUTPUT"

# Print logfile contents
print_title "Printing logfile contents"
print_title "BEWARE: Clang may replace calls to printf() with puts() when optimizations (even -O1) are enabled."
print_title "        These calls won't be logged, since logging only direct calls is asked for this project."
LOGFILE_CONTENT=$(cat ${LOGFILE})
echo "$LOGFILE_CONTENT"

# Print wheter modified test output matches original
print_title "Modified test output matches original (appropriateness depends on test):"
if [[ "$MODIFIED_TEST_OUTPUT" == "$ORIGINAL_TEST_OUTPUT" ]]
then
    echo "Yes"
else
    echo "No"
fi

# Print whether logfile contents match original test output
print_title "Logfile contents match original test output (appropriateness depends on test):"
if [[ "$LOGFILE_CONTENT" == "$ORIGINAL_TEST_OUTPUT" ]]
then
    echo "Yes"
else
    echo "No"
fi

# Clean up test object files and executables
print_title "Cleaning up test object files and executables"
rm -f ${TEST_PATH}
rm -f ${TEST_PATH}.o
rm -f ${TEST_PATH}.bc
rm -f ${MODIFIED_TEST_PATH}
rm -f ${MODIFIED_TEST_PATH}.o
rm -f ${MODIFIED_TEST_PATH}.bc

# Return to script directory
cd ${SCRIPT_DIR}
