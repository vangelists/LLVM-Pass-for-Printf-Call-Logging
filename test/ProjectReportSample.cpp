// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

//
// Original source code.
//

#include <cstdio>

int main() {
    printf("Hi!\n");
    return 0;
}


//
// Sample source code after pass.
//

#if 0
#include <cstdio>

void* logfile = nullptr;
const char* logfilePath = "log.txt";
const char* fopenMode = "w+";
const char* fopenFailureMessage = "Failed to open 'log.txt'. "
                                  "Logging of printf() calls disabled.\n";


void __LLVMPassForPrintfCallLogging_openLogfile() {
    logfile = fopen(logfilePath, fopenMode);
    if (logfile == nullptr) {
        printf(fopenFailureMessage);
    }
}

void __LLVMPassForPrintfCallLogging_closeLogfile() {
    if (logfile != nullptr) {
        fclose(logfile);
    }
}

int main() {
    __LLVMPassForPrintfCallLogging_openLogfile();

    printf("Hi!\n");
    if (logfile != nullptr) {
        fprintf(logfile, "Hi!\n");
    }

    __LLVMPassForPrintfCallLogging_closeLogfile();
    return 0;
}
#endif
