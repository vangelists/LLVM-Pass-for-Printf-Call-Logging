// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include <PrintfLogger.h>

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <functional>
#include <map>
#include <set>

using namespace llvm;

using Arguments = std::vector<Value*>;
using ConditionGenerator = std::function<Value*(void)>;
using FunctionName = std::string_view;
using IRGenerator = std::function<void(void)>;

char PrintfLogger::ID = 0;

//--------------------------------------------------------------------------------------------------

namespace {
    constexpr auto generatedFunctionPrefix = "__LLVMPassForPrintfCallLogging_";
    constexpr auto logfilePath = "log.txt";
    constexpr auto passName = "log-printf-calls";
    const auto passDescription = std::string("Log all calls to printf() in '") + logfilePath + "'";
    const auto fopenFailureMessage = std::string("Failed to open '") + logfilePath +
                                     "'. Logging of printf() calls disabled.\n";

    std::unique_ptr<IRBuilder<>> irBuilder;
    std::set<Instruction*> processedPrintfCalls;
    std::map<FunctionName, Constant*> libraryFunctions;

    Module* module;
    LLVMContext* context;

    GlobalVariable* logfile;
    LoadInst* logfilePointer;

    constexpr auto EMPTY_LAMBDA = [](){};

    constexpr auto loggingEnabledCheckGenerator = []() {
        logfilePointer = irBuilder->CreateLoad(logfile);
        return irBuilder->CreateICmpNE(logfilePointer,
                                       Constant::getNullValue(logfilePointer->getType()));
    };

    Function* openLogfileFunction;
    Function* closeLogfileFunction;
}

//--------------------------------------------------------------------------------------------------

template<typename T>
static inline Type* generateType() {
    return TypeBuilder<T, false>::get(*context);
}

template<typename T>
static inline FunctionType* generateFunctionType() {
    return cast<FunctionType>(generateType<T>());
}

static inline std::string getGeneratedFunctionName(FunctionName&& functionName) {
    return std::string(generatedFunctionPrefix).append(functionName);
}

//--------------------------------------------------------------------------------------------------

static inline void cacheLibraryFunction(FunctionName&& functionName, FunctionType* functionType) {
    libraryFunctions[functionName] = module->getOrInsertFunction(functionName.data(), functionType);
}

static inline void cacheLibraryFunctions() {
    cacheLibraryFunction("fopen", generateFunctionType<void*(const char*, const char*)>());
    cacheLibraryFunction("fclose", generateFunctionType<int(void*)>());
    cacheLibraryFunction("printf", generateFunctionType<int(char*, ...)>());
    cacheLibraryFunction("fprintf", generateFunctionType<int(void*, const char*, ...)>());
}

static inline Constant* getLibraryFunction(FunctionName&& functionName) {
    assert("Library function not found in cache!" && libraryFunctions[functionName]);
    return libraryFunctions[functionName];
}

//--------------------------------------------------------------------------------------------------

static inline bool printfCallHasBeenProcessed(Instruction* callToPrintf) {
    return processedPrintfCalls.find(callToPrintf) != processedPrintfCalls.end();
}

static inline void recordPrintfCall(Instruction* callToPrintf) {
    assert("Call to printf() has already been processed!" && !printfCallHasBeenProcessed(callToPrintf));
    processedPrintfCalls.insert(callToPrintf);
}

static void handlePrintfCall(Instruction* instructionAfterCallToPrintf, Arguments& printfArguments) {
    irBuilder->SetInsertPoint(instructionAfterCallToPrintf);
    const auto thenBlock = SplitBlockAndInsertIfThen(loggingEnabledCheckGenerator(),
                                                     instructionAfterCallToPrintf, false);
    printfArguments.insert(printfArguments.begin(), logfilePointer);
    irBuilder->SetInsertPoint(thenBlock);
    irBuilder->CreateCall(getLibraryFunction("fprintf"), printfArguments);
}

static void handlePrintfInvocation(Instruction* printfInvocation, Arguments& printfArguments) {
    irBuilder->SetInsertPoint(printfInvocation);
    TerminatorInst* thenBlockTerminator;
    TerminatorInst* elseBlockTerminator;
    SplitBlockAndInsertIfThenElse(loggingEnabledCheckGenerator(), printfInvocation,
                                  &thenBlockTerminator, &elseBlockTerminator);
    printfInvocation->moveBefore(elseBlockTerminator);

    irBuilder->SetInsertPoint(thenBlockTerminator);
    recordPrintfCall(irBuilder->CreateCall(getLibraryFunction("printf"), printfArguments));

    printfArguments.insert(printfArguments.begin(), logfilePointer);
    const auto invokeInstruction = cast<InvokeInst>(printfInvocation);
    const auto normalDestination = invokeInstruction->getNormalDest();
    const auto unwindDestination = invokeInstruction->getUnwindDest();
    irBuilder->CreateInvoke(getLibraryFunction("fprintf"), normalDestination, unwindDestination,
                            printfArguments);

    const auto emptyBlock = cast<BasicBlock>(thenBlockTerminator->getOperand(0));
    thenBlockTerminator->eraseFromParent();
    elseBlockTerminator->eraseFromParent();
    emptyBlock->eraseFromParent();
}

static bool injectLogInstruction(CallSite&& callToPrintf) {
    if (callToPrintf.getParent()->getParent()->getName().startswith(generatedFunctionPrefix)) {
        return false;
    }

    Arguments printfArguments(std::make_move_iterator(callToPrintf.arg_begin()),
                              std::make_move_iterator(callToPrintf.arg_end()));
    recordPrintfCall(callToPrintf.getInstruction());

    if (const auto instructionAfterCallToPrintf = callToPrintf->getNextNonDebugInstruction()) {
        assert("Call to printf() is a terminator instruction!" &&
               !isa<TerminatorInst>(*callToPrintf.getInstruction()));
        handlePrintfCall(instructionAfterCallToPrintf, printfArguments);
        return false;
    } else {
        assert("Call to printf() is not a terminator instruction!" &&
               isa<TerminatorInst>(callToPrintf.getInstruction()));
        handlePrintfInvocation(callToPrintf.getInstruction(), printfArguments);
        return true;
    }
}

//--------------------------------------------------------------------------------------------------

static inline std::pair<bool, CallSite> isUnprocessedPrintfCall(Instruction* instruction) {
    if (auto callInstruction = CallSite(instruction)) {
        if (callInstruction.getCalledValue()->stripPointerCasts()->getName() == "printf") {
            if (!printfCallHasBeenProcessed(callInstruction.getInstruction())) {
                return {true, callInstruction};
            }
        }
    }
    return {};
}

static inline bool processFunction(Function& function) {
    for (auto& block : function) {
        for (auto& instruction : block) {
            if (auto callInstruction = isUnprocessedPrintfCall(&instruction); callInstruction.first) {
                if (injectLogInstruction(std::move(callInstruction.second))) {
                    return true;
                }
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------

static inline void injectLogfileOpenCode(Function& main) {
    const auto firstInstruction = main.getEntryBlock().getFirstInsertionPt();
    irBuilder->SetInsertPoint(&*firstInstruction);
    irBuilder->CreateCall(openLogfileFunction);
}

static inline void injectLogfileCloseCode(Function& main) {
    for (auto& block : main) {
        for (auto& instruction : block) {
            if (isa<ReturnInst>(&instruction)) {
                irBuilder->SetInsertPoint(&instruction);
                irBuilder->CreateCall(closeLogfileFunction);
            }
        }
    }
}

static inline void injectLogfileHandlingCode(Function& main) {
    injectLogfileOpenCode(main);
    injectLogfileCloseCode(main);
}

//--------------------------------------------------------------------------------------------------

static inline void initializeLogfilePointer() {
    const auto logfilePointerType = generateType<void*>();
    const auto initializer = Constant::getNullValue(logfilePointerType);
    logfile = new GlobalVariable(*module, logfilePointerType, false, GlobalValue::InternalLinkage,
                                 initializer, "logfile");
}

static inline Function* insertModuleFunction(FunctionName&& functionName, FunctionType* functionType) {
    assert("Function already exists in module!" && !module->getFunction(functionName.data()));
    const auto generatedFunctionType = functionType ? functionType : generateFunctionType<void(void)>();
    return cast<Function>(module->getOrInsertFunction(functionName.data(), generatedFunctionType));
}

static Function* generateFunctionWithConditional(FunctionName&& functionName,
                                                 FunctionType* functionType,
                                                 ConditionGenerator&& conditionGenerator,
                                                 IRGenerator&& onTrueGenerator = EMPTY_LAMBDA,
                                                 IRGenerator&& onFalseGenerator = EMPTY_LAMBDA) {
    const auto generatedFunction =
            insertModuleFunction(getGeneratedFunctionName(std::move(functionName)), functionType);
    const auto conditionBlock = BasicBlock::Create(*context, "condition", generatedFunction);
    irBuilder->SetInsertPoint(conditionBlock);

    const auto trueBlock = BasicBlock::Create(*context, "true", generatedFunction);
    const auto falseBlock = BasicBlock::Create(*context, "false", generatedFunction);
    irBuilder->CreateCondBr(conditionGenerator(), trueBlock, falseBlock);

    irBuilder->SetInsertPoint(trueBlock);
    onTrueGenerator();
    irBuilder->CreateRetVoid();

    irBuilder->SetInsertPoint(falseBlock);
    onFalseGenerator();
    irBuilder->CreateRetVoid();

    return generatedFunction;
}

static void generateOpenLogfileFunction() {
    const auto condition = []() {
        const auto fopenArgumentFilename = irBuilder->CreateGlobalStringPtr(logfilePath,
                                                                            "logfilePath");
        const auto fopenArgumentMode = irBuilder->CreateGlobalStringPtr("w+", "fopenMode");
        const auto logfilePointer = irBuilder->CreateCall(getLibraryFunction("fopen"),
                                                          {fopenArgumentFilename, fopenArgumentMode});
        irBuilder->CreateStore(logfilePointer, logfile);
        return irBuilder->CreateICmpEQ(logfilePointer,
                                       Constant::getNullValue(logfilePointer->getType()));
    };
    const auto onTrue = []() {
        const auto printfArgument = irBuilder->CreateGlobalStringPtr(fopenFailureMessage,
                                                                     "fopenFailureMessage");
        recordPrintfCall(irBuilder->CreateCall(getLibraryFunction("printf"), printfArgument));
    };
    openLogfileFunction = generateFunctionWithConditional("openLogfile", nullptr, condition, onTrue);
}

static void generateCloseLogfileFunction() {
    const auto condition = loggingEnabledCheckGenerator;
    const auto onTrue = []() {
        irBuilder->CreateCall(getLibraryFunction("fclose"), logfilePointer);
    };
    closeLogfileFunction = generateFunctionWithConditional("closeLogfile", nullptr, condition, onTrue);
}

static inline void initializePass(Module& M) {
    module = &M;
    context = &module->getContext();
    irBuilder = std::make_unique<IRBuilder<>>(*context);

    initializeLogfilePointer();
    cacheLibraryFunctions();
    generateOpenLogfileFunction();
    generateCloseLogfileFunction();
}

//--------------------------------------------------------------------------------------------------

PrintfLogger::PrintfLogger() : ModulePass(ID) {}

bool PrintfLogger::runOnModule(Module& module) {
    if (const auto mainFunction = module.getFunction("main")) {
        initializePass(module);
        injectLogfileHandlingCode(*mainFunction);
        for (auto& function : module) {
            do {} while (processFunction(function));
        }
        return true;
    } else {
        errs() << "main() not found in module '" << module.getName() << "'. "
                  "Logging of printf() calls disabled for this module.\n";
        return false;
    }
}

//--------------------------------------------------------------------------------------------------

static RegisterPass<PrintfLogger> PrintfLoggingPass(passName, passDescription);
