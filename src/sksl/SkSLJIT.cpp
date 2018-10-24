/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#ifdef SK_LLVM_AVAILABLE

#include "SkSLJIT.h"

#include "SkCpu.h"
#include "SkRasterPipeline.h"
#include "ir/SkSLAppendStage.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionReference.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLUnresolvedFunction.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"

static constexpr int MAX_VECTOR_COUNT = 16;

extern "C" void sksl_pipeline_append(SkRasterPipeline* p, int stage, void* ctx) {
    p->append((SkRasterPipeline::StockStage) stage, ctx);
}

#define PTR_SIZE sizeof(void*)

extern "C" void sksl_pipeline_append_callback(SkRasterPipeline* p, void* fn) {
    p->append(fn, nullptr);
}

extern "C" void sksl_debug_print(float f) {
    printf("Debug: %f\n", f);
}

extern "C" float sksl_clamp1(float f, float min, float max) {
    return SkTPin(f, min, max);
}

using float2 = __attribute__((vector_size(8))) float;
using float3 = __attribute__((vector_size(16))) float;
using float4 = __attribute__((vector_size(16))) float;

extern "C" float2 sksl_clamp2(float2 f, float min, float max) {
    return float2 { SkTPin(f[0], min, max), SkTPin(f[1], min, max) };
}

extern "C" float3 sksl_clamp3(float3 f, float min, float max) {
    return float3 { SkTPin(f[0], min, max), SkTPin(f[1], min, max), SkTPin(f[2], min, max) };
}

extern "C" float4 sksl_clamp4(float4 f, float min, float max) {
    return float4 { SkTPin(f[0], min, max), SkTPin(f[1], min, max), SkTPin(f[2], min, max),
                    SkTPin(f[3], min, max) };
}

namespace SkSL {

static constexpr int STAGE_PARAM_COUNT = 12;

static bool ends_with_branch(const Statement& stmt) {
    switch (stmt.fKind) {
        case Statement::kBlock_Kind: {
            const Block& b = (const Block&) stmt;
            if (b.fStatements.size()) {
                return ends_with_branch(*b.fStatements.back());
            }
            return false;
        }
        case Statement::kBreak_Kind:    // fall through
        case Statement::kContinue_Kind: // fall through
        case Statement::kReturn_Kind:   // fall through
            return true;
        default:
            return false;
    }
}

JIT::JIT(Compiler* compiler)
: fCompiler(*compiler) {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMLinkInMCJIT();
    SkASSERT(!SkCpu::Supports(SkCpu::SKX)); // not yet supported
    if (SkCpu::Supports(SkCpu::HSW)) {
        fVectorCount = 8;
        fCPU = "haswell";
    } else if (SkCpu::Supports(SkCpu::AVX)) {
        fVectorCount = 8;
        fCPU = "ivybridge";
    } else {
        fVectorCount = 4;
        fCPU = nullptr;
    }
    fContext = LLVMContextCreate();
    fVoidType = LLVMVoidTypeInContext(fContext);
    fInt1Type = LLVMInt1TypeInContext(fContext);
    fInt1VectorType = LLVMVectorType(fInt1Type, fVectorCount);
    fInt1Vector2Type = LLVMVectorType(fInt1Type, 2);
    fInt1Vector3Type = LLVMVectorType(fInt1Type, 3);
    fInt1Vector4Type = LLVMVectorType(fInt1Type, 4);
    fInt8Type = LLVMInt8TypeInContext(fContext);
    fInt8PtrType = LLVMPointerType(fInt8Type, 0);
    fInt32Type = LLVMInt32TypeInContext(fContext);
    fInt64Type = LLVMInt64TypeInContext(fContext);
    fSizeTType = LLVMInt64TypeInContext(fContext);
    fInt32VectorType = LLVMVectorType(fInt32Type, fVectorCount);
    fInt32Vector2Type = LLVMVectorType(fInt32Type, 2);
    fInt32Vector3Type = LLVMVectorType(fInt32Type, 3);
    fInt32Vector4Type = LLVMVectorType(fInt32Type, 4);
    fFloat32Type = LLVMFloatTypeInContext(fContext);
    fFloat32VectorType = LLVMVectorType(fFloat32Type, fVectorCount);
    fFloat32Vector2Type = LLVMVectorType(fFloat32Type, 2);
    fFloat32Vector3Type = LLVMVectorType(fFloat32Type, 3);
    fFloat32Vector4Type = LLVMVectorType(fFloat32Type, 4);
}

JIT::~JIT() {
    LLVMOrcDisposeInstance(fJITStack);
    LLVMContextDispose(fContext);
}

void JIT::addBuiltinFunction(const char* ourName, const char* realName, LLVMTypeRef returnType,
                             std::vector<LLVMTypeRef> parameters) {
    bool found = false;
    for (const auto& pair : *fProgram->fSymbols) {
        if (Symbol::kFunctionDeclaration_Kind == pair.second->fKind) {
            const FunctionDeclaration& f = (const FunctionDeclaration&) *pair.second;
            if (pair.first != ourName || returnType != this->getType(f.fReturnType) ||
                parameters.size() != f.fParameters.size()) {
                continue;
            }
            for (size_t i = 0; i < parameters.size(); ++i) {
                if (parameters[i] != this->getType(f.fParameters[i]->fType)) {
                    goto next;
                }
            }
            fFunctions[&f] = LLVMAddFunction(fModule, realName, LLVMFunctionType(returnType,
                                                                                 parameters.data(),
                                                                                 parameters.size(),
                                                                                 false));
            found = true;
        }
        if (Symbol::kUnresolvedFunction_Kind == pair.second->fKind) {
            // FIXME consolidate this with the code above
            for (const auto& f : ((const UnresolvedFunction&) *pair.second).fFunctions) {
                if (pair.first != ourName || returnType != this->getType(f->fReturnType) ||
                    parameters.size() != f->fParameters.size()) {
                    continue;
                }
                for (size_t i = 0; i < parameters.size(); ++i) {
                    if (parameters[i] != this->getType(f->fParameters[i]->fType)) {
                        goto next;
                    }
                }
                fFunctions[f] = LLVMAddFunction(fModule, realName, LLVMFunctionType(
                                                                                  returnType,
                                                                                  parameters.data(),
                                                                                  parameters.size(),
                                                                                  false));
                found = true;
            }
        }
        next:;
    }
    SkASSERT(found);
}

void JIT::loadBuiltinFunctions() {
    this->addBuiltinFunction("abs", "fabs", fFloat32Type, { fFloat32Type });
    this->addBuiltinFunction("sin", "sinf", fFloat32Type, { fFloat32Type });
    this->addBuiltinFunction("cos", "cosf", fFloat32Type, { fFloat32Type });
    this->addBuiltinFunction("tan", "tanf", fFloat32Type, { fFloat32Type });
    this->addBuiltinFunction("sqrt", "sqrtf", fFloat32Type, { fFloat32Type });
    this->addBuiltinFunction("clamp", "sksl_clamp1", fFloat32Type, { fFloat32Type,
                                                                     fFloat32Type,
                                                                     fFloat32Type });
    this->addBuiltinFunction("clamp", "sksl_clamp2", fFloat32Vector2Type, { fFloat32Vector2Type,
                                                                            fFloat32Type,
                                                                            fFloat32Type });
    this->addBuiltinFunction("clamp", "sksl_clamp3", fFloat32Vector3Type, { fFloat32Vector3Type,
                                                                            fFloat32Type,
                                                                            fFloat32Type });
    this->addBuiltinFunction("clamp", "sksl_clamp4", fFloat32Vector4Type, { fFloat32Vector4Type,
                                                                            fFloat32Type,
                                                                            fFloat32Type });
    this->addBuiltinFunction("print", "sksl_debug_print", fVoidType, { fFloat32Type });
}

uint64_t JIT::resolveSymbol(const char* name, JIT* jit) {
    LLVMOrcTargetAddress result;
    if (!LLVMOrcGetSymbolAddress(jit->fJITStack, &result, name)) {
        if (!strcmp(name, "_sksl_pipeline_append")) {
            result = (uint64_t) &sksl_pipeline_append;
        } else if (!strcmp(name, "_sksl_pipeline_append_callback")) {
            result = (uint64_t) &sksl_pipeline_append_callback;
        } else if (!strcmp(name, "_sksl_clamp1")) {
            result = (uint64_t) &sksl_clamp1;
        } else if (!strcmp(name, "_sksl_clamp2")) {
            result = (uint64_t) &sksl_clamp2;
        } else if (!strcmp(name, "_sksl_clamp3")) {
            result = (uint64_t) &sksl_clamp3;
        } else if (!strcmp(name, "_sksl_clamp4")) {
            result = (uint64_t) &sksl_clamp4;
        } else if (!strcmp(name, "_sksl_debug_print")) {
            result = (uint64_t) &sksl_debug_print;
        } else {
            result = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name);
        }
    }
    SkASSERT(result);
    return result;
}

LLVMValueRef JIT::compileFunctionCall(LLVMBuilderRef builder, const FunctionCall& fc) {
    LLVMValueRef func = fFunctions[&fc.fFunction];
    SkASSERT(func);
    std::vector<LLVMValueRef> parameters;
    for (const auto& a : fc.fArguments) {
        parameters.push_back(this->compileExpression(builder, *a));
    }
    return LLVMBuildCall(builder, func, parameters.data(), parameters.size(), "");
}

LLVMTypeRef JIT::getType(const Type& type) {
    switch (type.kind()) {
        case Type::kOther_Kind:
            if (type.name() == "void") {
                return fVoidType;
            }
            SkASSERT(type.name() == "SkRasterPipeline");
            return fInt8PtrType;
        case Type::kScalar_Kind:
            if (type.isSigned() || type.isUnsigned()) {
                return fInt32Type;
            }
            if (type.isUnsigned()) {
                return fInt32Type;
            }
            if (type.isFloat()) {
                return fFloat32Type;
            }
            SkASSERT(type.name() == "bool");
            return fInt1Type;
        case Type::kArray_Kind:
            return LLVMPointerType(this->getType(type.componentType()), 0);
        case Type::kVector_Kind:
            if (type.name() == "float2" || type.name() == "half2") {
                return fFloat32Vector2Type;
            }
            if (type.name() == "float3" || type.name() == "half3") {
                return fFloat32Vector3Type;
            }
            if (type.name() == "float4" || type.name() == "half4") {
                return fFloat32Vector4Type;
            }
            if (type.name() == "int2" || type.name() == "short2" || type.name == "byte2") {
                return fInt32Vector2Type;
            }
            if (type.name() == "int3" || type.name() == "short3" || type.name == "byte3") {
                return fInt32Vector3Type;
            }
            if (type.name() == "int4" || type.name() == "short4" || type.name == "byte3") {
                return fInt32Vector4Type;
            }
            // fall through
        default:
            ABORT("unsupported type");
    }
}

void JIT::setBlock(LLVMBuilderRef builder, LLVMBasicBlockRef block) {
    fCurrentBlock = block;
    LLVMPositionBuilderAtEnd(builder, block);
}

std::unique_ptr<JIT::LValue> JIT::getLValue(LLVMBuilderRef builder, const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            class PointerLValue : public LValue {
            public:
                PointerLValue(LLVMValueRef ptr)
                : fPointer(ptr) {}

                LLVMValueRef load(LLVMBuilderRef builder) override {
                    return LLVMBuildLoad(builder, fPointer, "lvalue load");
                }

                void store(LLVMBuilderRef builder, LLVMValueRef value) override {
                    LLVMBuildStore(builder, value, fPointer);
                }

            private:
                LLVMValueRef fPointer;
            };
            const Variable* var = &((VariableReference&) expr).fVariable;
            if (var->fStorage == Variable::kParameter_Storage &&
                !(var->fModifiers.fFlags & Modifiers::kOut_Flag) &&
                fPromotedParameters.find(var) == fPromotedParameters.end()) {
                // promote parameter to variable
                fPromotedParameters.insert(var);
                LLVMPositionBuilderAtEnd(builder, fAllocaBlock);
                LLVMValueRef alloca = LLVMBuildAlloca(builder, this->getType(var->fType),
                                                      String(var->fName).c_str());
                LLVMBuildStore(builder, fVariables[var], alloca);
                LLVMPositionBuilderAtEnd(builder, fCurrentBlock);
                fVariables[var] = alloca;
            }
            LLVMValueRef ptr = fVariables[var];
            return std::unique_ptr<LValue>(new PointerLValue(ptr));
        }
        case Expression::kTernary_Kind: {
            class TernaryLValue : public LValue {
            public:
                TernaryLValue(JIT* jit, LLVMValueRef test, std::unique_ptr<LValue> ifTrue,
                              std::unique_ptr<LValue> ifFalse)
                : fJIT(*jit)
                , fTest(test)
                , fIfTrue(std::move(ifTrue))
                , fIfFalse(std::move(ifFalse)) {}

                LLVMValueRef load(LLVMBuilderRef builder) override {
                    LLVMBasicBlockRef trueBlock = LLVMAppendBasicBlockInContext(
                                                                              fJIT.fContext,
                                                                              fJIT.fCurrentFunction,
                                                                              "true ? ...");
                    LLVMBasicBlockRef falseBlock = LLVMAppendBasicBlockInContext(
                                                                              fJIT.fContext,
                                                                              fJIT.fCurrentFunction,
                                                                              "false ? ...");
                    LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fJIT.fContext,
                                                                            fJIT.fCurrentFunction,
                                                                            "ternary merge");
                    LLVMBuildCondBr(builder, fTest, trueBlock, falseBlock);
                    fJIT.setBlock(builder, trueBlock);
                    LLVMValueRef ifTrue = fIfTrue->load(builder);
                    LLVMBuildBr(builder, merge);
                    fJIT.setBlock(builder, falseBlock);
                    LLVMValueRef ifFalse = fIfTrue->load(builder);
                    LLVMBuildBr(builder, merge);
                    fJIT.setBlock(builder, merge);
                    LLVMTypeRef type = LLVMPointerType(LLVMTypeOf(ifTrue), 0);
                    LLVMValueRef phi = LLVMBuildPhi(builder, type, "?");
                    LLVMValueRef incomingValues[2] = { ifTrue, ifFalse };
                    LLVMBasicBlockRef incomingBlocks[2] = { trueBlock, falseBlock };
                    LLVMAddIncoming(phi, incomingValues, incomingBlocks, 2);
                    return phi;
                }

                void store(LLVMBuilderRef builder, LLVMValueRef value) override {
                    LLVMBasicBlockRef trueBlock = LLVMAppendBasicBlockInContext(
                                                                              fJIT.fContext,
                                                                              fJIT.fCurrentFunction,
                                                                              "true ? ...");
                    LLVMBasicBlockRef falseBlock = LLVMAppendBasicBlockInContext(
                                                                              fJIT.fContext,
                                                                              fJIT.fCurrentFunction,
                                                                              "false ? ...");
                    LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fJIT.fContext,
                                                                            fJIT.fCurrentFunction,
                                                                            "ternary merge");
                    LLVMBuildCondBr(builder, fTest, trueBlock, falseBlock);
                    fJIT.setBlock(builder, trueBlock);
                    fIfTrue->store(builder, value);
                    LLVMBuildBr(builder, merge);
                    fJIT.setBlock(builder, falseBlock);
                    fIfTrue->store(builder, value);
                    LLVMBuildBr(builder, merge);
                    fJIT.setBlock(builder, merge);
                }

            private:
                JIT& fJIT;
                LLVMValueRef fTest;
                std::unique_ptr<LValue> fIfTrue;
                std::unique_ptr<LValue> fIfFalse;
            };
            const TernaryExpression& t = (const TernaryExpression&) expr;
            LLVMValueRef test = this->compileExpression(builder, *t.fTest);
            return std::unique_ptr<LValue>(new TernaryLValue(this,
                                                             test,
                                                             this->getLValue(builder,
                                                                             *t.fIfTrue),
                                                             this->getLValue(builder,
                                                                             *t.fIfFalse)));
        }
        case Expression::kSwizzle_Kind: {
            class SwizzleLValue : public LValue {
            public:
                SwizzleLValue(JIT* jit, LLVMTypeRef type, std::unique_ptr<LValue> base,
                              std::vector<int> components)
                : fJIT(*jit)
                , fType(type)
                , fBase(std::move(base))
                , fComponents(components) {}

                LLVMValueRef load(LLVMBuilderRef builder) override {
                    LLVMValueRef base = fBase->load(builder);
                    if (fComponents.size() > 1) {
                        LLVMValueRef result = LLVMGetUndef(fType);
                        for (size_t i = 0; i < fComponents.size(); ++i) {
                            LLVMValueRef element = LLVMBuildExtractElement(
                                                                       builder,
                                                                       base,
                                                                       LLVMConstInt(fJIT.fInt32Type,
                                                                                    fComponents[i],
                                                                                    false),
                                                                       "swizzle extract");
                            result = LLVMBuildInsertElement(builder, result, element,
                                                            LLVMConstInt(fJIT.fInt32Type, i, false),
                                                            "swizzle insert");
                        }
                        return result;
                    }
                    SkASSERT(fComponents.size() == 1);
                    return LLVMBuildExtractElement(builder, base,
                                                            LLVMConstInt(fJIT.fInt32Type,
                                                                         fComponents[0],
                                                                         false),
                                                            "swizzle extract");
                }

                void store(LLVMBuilderRef builder, LLVMValueRef value) override {
                    LLVMValueRef result = fBase->load(builder);
                    if (fComponents.size() > 1) {
                        for (size_t i = 0; i < fComponents.size(); ++i) {
                            LLVMValueRef element = LLVMBuildExtractElement(builder, value,
                                                                           LLVMConstInt(
                                                                                    fJIT.fInt32Type,
                                                                                    i,
                                                                                    false),
                                                                           "swizzle extract");
                            result = LLVMBuildInsertElement(builder, result, element,
                                                            LLVMConstInt(fJIT.fInt32Type,
                                                                         fComponents[i],
                                                                         false),
                                                            "swizzle insert");
                        }
                    } else {
                        result = LLVMBuildInsertElement(builder, result, value,
                                                        LLVMConstInt(fJIT.fInt32Type,
                                                                     fComponents[0],
                                                                     false),
                                                        "swizzle insert");
                    }
                    fBase->store(builder, result);
                }

            private:
                JIT& fJIT;
                LLVMTypeRef fType;
                std::unique_ptr<LValue> fBase;
                std::vector<int> fComponents;
            };
            const Swizzle& s = (const Swizzle&) expr;
            return std::unique_ptr<LValue>(new SwizzleLValue(this, this->getType(s.fType),
                                                             this->getLValue(builder, *s.fBase),
                                                             s.fComponents));
        }
        default:
            ABORT("unsupported lvalue");
    }
}

JIT::TypeKind JIT::typeKind(const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return this->typeKind(type.componentType());
    }
    if (type.fName == "int" || type.fName == "short" || type.fName == "byte") {
        return JIT::kInt_TypeKind;
    } else if (type.fName == "uint" || type.fName == "ushort" || type.fName == "ubyte") {
        return JIT::kUInt_TypeKind;
    } else if (type.fName == "float" || type.fName == "double" || type.fName == "half") {
        return JIT::kFloat_TypeKind;
    }
    ABORT("unsupported type: %s\n", type.description().c_str());
}

void JIT::vectorize(LLVMBuilderRef builder, LLVMValueRef* value, int columns) {
    LLVMValueRef result = LLVMGetUndef(LLVMVectorType(LLVMTypeOf(*value), columns));
    for (int i = 0; i < columns; ++i) {
        result = LLVMBuildInsertElement(builder,
                                        result,
                                        *value,
                                        LLVMConstInt(fInt32Type, i, false),
                                        "vectorize");
    }
    *value = result;
}

void JIT::vectorize(LLVMBuilderRef builder, const BinaryExpression& b, LLVMValueRef* left,
                    LLVMValueRef* right) {
    if (b.fLeft->fType.kind() == Type::kScalar_Kind &&
        b.fRight->fType.kind() == Type::kVector_Kind) {
        this->vectorize(builder, left, b.fRight->fType.columns());
    } else if (b.fLeft->fType.kind() == Type::kVector_Kind &&
               b.fRight->fType.kind() == Type::kScalar_Kind) {
        this->vectorize(builder, right, b.fLeft->fType.columns());
    }
}


LLVMValueRef JIT::compileBinary(LLVMBuilderRef builder, const BinaryExpression& b) {
    #define BINARY(SFunc, UFunc, FFunc) {                                    \
        LLVMValueRef left = this->compileExpression(builder, *b.fLeft);      \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        this->vectorize(builder, b, &left, &right);                          \
        switch (this->typeKind(b.fLeft->fType)) {                            \
            case kInt_TypeKind:                                              \
                return SFunc(builder, left, right, "binary");                \
            case kUInt_TypeKind:                                             \
                return UFunc(builder, left, right, "binary");                \
            case kFloat_TypeKind:                                            \
                return FFunc(builder, left, right, "binary");                \
            default:                                                         \
                ABORT("unsupported typeKind");                               \
        }                                                                    \
    }
    #define COMPOUND(SFunc, UFunc, FFunc) {                                  \
        std::unique_ptr<LValue> lvalue = this->getLValue(builder, *b.fLeft); \
        LLVMValueRef left = lvalue->load(builder);                           \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        this->vectorize(builder, b, &left, &right);                          \
        LLVMValueRef result;                                                 \
        switch (this->typeKind(b.fLeft->fType)) {                            \
            case kInt_TypeKind:                                              \
                result = SFunc(builder, left, right, "binary");              \
                break;                                                       \
            case kUInt_TypeKind:                                             \
                result = UFunc(builder, left, right, "binary");              \
                break;                                                       \
            case kFloat_TypeKind:                                            \
                result = FFunc(builder, left, right, "binary");              \
                break;                                                       \
            default:                                                         \
                ABORT("unsupported typeKind");                               \
        }                                                                    \
        lvalue->store(builder, result);                                      \
        return result;                                                       \
    }
    #define COMPARE(SFunc, SOp, UFunc, UOp, FFunc, FOp) {                    \
        LLVMValueRef left = this->compileExpression(builder, *b.fLeft);      \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        this->vectorize(builder, b, &left, &right);                          \
        switch (this->typeKind(b.fLeft->fType)) {                            \
            case kInt_TypeKind:                                              \
                return SFunc(builder, SOp, left, right, "binary");           \
            case kUInt_TypeKind:                                             \
                return UFunc(builder, UOp, left, right, "binary");           \
            case kFloat_TypeKind:                                            \
                return FFunc(builder, FOp, left, right, "binary");           \
            default:                                                         \
                ABORT("unsupported typeKind");                               \
        }                                                                    \
    }
    switch (b.fOperator) {
        case Token::EQ: {
            std::unique_ptr<LValue> lvalue = this->getLValue(builder, *b.fLeft);
            LLVMValueRef result = this->compileExpression(builder, *b.fRight);
            lvalue->store(builder, result);
            return result;
        }
        case Token::PLUS:
            BINARY(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
        case Token::MINUS:
            BINARY(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
        case Token::STAR:
            BINARY(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
        case Token::SLASH:
            BINARY(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
        case Token::PERCENT:
            BINARY(LLVMBuildSRem, LLVMBuildURem, LLVMBuildSRem);
        case Token::BITWISEAND:
            BINARY(LLVMBuildAnd, LLVMBuildAnd, LLVMBuildAnd);
        case Token::BITWISEOR:
            BINARY(LLVMBuildOr, LLVMBuildOr, LLVMBuildOr);
        case Token::SHL:
            BINARY(LLVMBuildShl, LLVMBuildShl, LLVMBuildShl);
        case Token::SHR:
            BINARY(LLVMBuildAShr, LLVMBuildLShr, LLVMBuildAShr);
        case Token::PLUSEQ:
            COMPOUND(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
        case Token::MINUSEQ:
            COMPOUND(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
        case Token::STAREQ:
            COMPOUND(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
        case Token::SLASHEQ:
            COMPOUND(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
        case Token::BITWISEANDEQ:
            COMPOUND(LLVMBuildAnd, LLVMBuildAnd, LLVMBuildAnd);
        case Token::BITWISEOREQ:
            COMPOUND(LLVMBuildOr, LLVMBuildOr, LLVMBuildOr);
        case Token::EQEQ:
            switch (b.fLeft->fType.kind()) {
                case Type::kScalar_Kind:
                    COMPARE(LLVMBuildICmp, LLVMIntEQ,
                            LLVMBuildICmp, LLVMIntEQ,
                            LLVMBuildFCmp, LLVMRealOEQ);
                case Type::kVector_Kind: {
                    LLVMValueRef left = this->compileExpression(builder, *b.fLeft);
                    LLVMValueRef right = this->compileExpression(builder, *b.fRight);
                    this->vectorize(builder, b, &left, &right);
                    LLVMValueRef value;
                    switch (this->typeKind(b.fLeft->fType)) {
                        case kInt_TypeKind:
                            value = LLVMBuildICmp(builder, LLVMIntEQ, left, right, "binary");
                            break;
                        case kUInt_TypeKind:
                            value = LLVMBuildICmp(builder, LLVMIntEQ, left, right, "binary");
                            break;
                        case kFloat_TypeKind:
                            value = LLVMBuildFCmp(builder, LLVMRealOEQ, left, right, "binary");
                            break;
                        default:
                            ABORT("unsupported typeKind");
                    }
                    LLVMValueRef args[1] = { value };
                    LLVMValueRef func;
                    switch (b.fLeft->fType.columns()) {
                        case 2: func = fFoldAnd2Func; break;
                        case 3: func = fFoldAnd3Func; break;
                        case 4: func = fFoldAnd4Func; break;
                        default:
                            SkASSERT(false);
                            func = fFoldAnd2Func;
                    }
                    return LLVMBuildCall(builder, func, args, 1, "all");
                }
                default:
                    SkASSERT(false);
            }
        case Token::NEQ:
            switch (b.fLeft->fType.kind()) {
                case Type::kScalar_Kind:
                    COMPARE(LLVMBuildICmp, LLVMIntNE,
                            LLVMBuildICmp, LLVMIntNE,
                            LLVMBuildFCmp, LLVMRealONE);
                case Type::kVector_Kind: {
                    LLVMValueRef left = this->compileExpression(builder, *b.fLeft);
                    LLVMValueRef right = this->compileExpression(builder, *b.fRight);
                    this->vectorize(builder, b, &left, &right);
                    LLVMValueRef value;
                    switch (this->typeKind(b.fLeft->fType)) {
                        case kInt_TypeKind:
                            value = LLVMBuildICmp(builder, LLVMIntNE, left, right, "binary");
                            break;
                        case kUInt_TypeKind:
                            value = LLVMBuildICmp(builder, LLVMIntNE, left, right, "binary");
                            break;
                        case kFloat_TypeKind:
                            value = LLVMBuildFCmp(builder, LLVMRealONE, left, right, "binary");
                            break;
                        default:
                            ABORT("unsupported typeKind");
                    }
                    LLVMValueRef args[1] = { value };
                    LLVMValueRef func;
                    switch (b.fLeft->fType.columns()) {
                        case 2: func = fFoldOr2Func; break;
                        case 3: func = fFoldOr3Func; break;
                        case 4: func = fFoldOr4Func; break;
                        default:
                            SkASSERT(false);
                            func = fFoldOr2Func;
                    }
                    return LLVMBuildCall(builder, func, args, 1, "all");
                }
                default:
                    SkASSERT(false);
            }
        case Token::LT:
            COMPARE(LLVMBuildICmp, LLVMIntSLT,
                    LLVMBuildICmp, LLVMIntULT,
                    LLVMBuildFCmp, LLVMRealOLT);
        case Token::LTEQ:
            COMPARE(LLVMBuildICmp, LLVMIntSLE,
                    LLVMBuildICmp, LLVMIntULE,
                    LLVMBuildFCmp, LLVMRealOLE);
        case Token::GT:
            COMPARE(LLVMBuildICmp, LLVMIntSGT,
                    LLVMBuildICmp, LLVMIntUGT,
                    LLVMBuildFCmp, LLVMRealOGT);
        case Token::GTEQ:
            COMPARE(LLVMBuildICmp, LLVMIntSGE,
                    LLVMBuildICmp, LLVMIntUGE,
                    LLVMBuildFCmp, LLVMRealOGE);
        case Token::LOGICALAND: {
            LLVMValueRef left = this->compileExpression(builder, *b.fLeft);
            LLVMBasicBlockRef ifFalse = fCurrentBlock;
            LLVMBasicBlockRef ifTrue = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                     "true && ...");
            LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                    "&& merge");
            LLVMBuildCondBr(builder, left, ifTrue, merge);
            this->setBlock(builder, ifTrue);
            LLVMValueRef right = this->compileExpression(builder, *b.fRight);
            LLVMBuildBr(builder, merge);
            this->setBlock(builder, merge);
            LLVMValueRef phi = LLVMBuildPhi(builder, fInt1Type, "&&");
            LLVMValueRef incomingValues[2] = { right, LLVMConstInt(fInt1Type, 0, false) };
            LLVMBasicBlockRef incomingBlocks[2] = { ifTrue, ifFalse };
            LLVMAddIncoming(phi, incomingValues, incomingBlocks, 2);
            return phi;
        }
        case Token::LOGICALOR: {
            LLVMValueRef left = this->compileExpression(builder, *b.fLeft);
            LLVMBasicBlockRef ifTrue = fCurrentBlock;
            LLVMBasicBlockRef ifFalse = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                      "false || ...");
            LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                    "|| merge");
            LLVMBuildCondBr(builder, left, merge, ifFalse);
            this->setBlock(builder, ifFalse);
            LLVMValueRef right = this->compileExpression(builder, *b.fRight);
            LLVMBuildBr(builder, merge);
            this->setBlock(builder, merge);
            LLVMValueRef phi = LLVMBuildPhi(builder, fInt1Type, "||");
            LLVMValueRef incomingValues[2] = { right, LLVMConstInt(fInt1Type, 1, false) };
            LLVMBasicBlockRef incomingBlocks[2] = { ifFalse, ifTrue };
            LLVMAddIncoming(phi, incomingValues, incomingBlocks, 2);
            return phi;
        }
        default:
            printf("%s\n", b.description().c_str());
            ABORT("unsupported binary operator");
    }
}

LLVMValueRef JIT::compileIndex(LLVMBuilderRef builder, const IndexExpression& idx) {
    LLVMValueRef base = this->compileExpression(builder, *idx.fBase);
    LLVMValueRef index = this->compileExpression(builder, *idx.fIndex);
    LLVMValueRef ptr = LLVMBuildGEP(builder, base, &index, 1, "index ptr");
    return LLVMBuildLoad(builder, ptr, "index load");
}

LLVMValueRef JIT::compilePostfix(LLVMBuilderRef builder, const PostfixExpression& p) {
    std::unique_ptr<LValue> lvalue = this->getLValue(builder, *p.fOperand);
    LLVMValueRef result = lvalue->load(builder);
    LLVMValueRef mod;
    LLVMValueRef one = LLVMConstInt(this->getType(p.fType), 1, false);
    switch (p.fOperator) {
        case Token::PLUSPLUS:
            switch (this->typeKind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    mod = LLVMBuildAdd(builder, result, one, "++");
                    break;
                case kFloat_TypeKind:
                    mod = LLVMBuildFAdd(builder, result, one, "++");
                    break;
                default:
                    ABORT("unsupported typeKind");
            }
            break;
        case Token::MINUSMINUS:
            switch (this->typeKind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    mod = LLVMBuildSub(builder, result, one, "--");
                    break;
                case kFloat_TypeKind:
                    mod = LLVMBuildFSub(builder, result, one, "--");
                    break;
                default:
                    ABORT("unsupported typeKind");
            }
            break;
        default:
            ABORT("unsupported postfix op");
    }
    lvalue->store(builder, mod);
    return result;
}

LLVMValueRef JIT::compilePrefix(LLVMBuilderRef builder, const PrefixExpression& p) {
    LLVMValueRef one = LLVMConstInt(this->getType(p.fType), 1, false);
    if (Token::LOGICALNOT == p.fOperator) {
        LLVMValueRef base = this->compileExpression(builder, *p.fOperand);
        return LLVMBuildXor(builder, base, one, "!");
    }
    if (Token::MINUS == p.fOperator) {
        LLVMValueRef base = this->compileExpression(builder, *p.fOperand);
        return LLVMBuildSub(builder, LLVMConstInt(this->getType(p.fType), 0, false), base, "-");
    }
    std::unique_ptr<LValue> lvalue = this->getLValue(builder, *p.fOperand);
    LLVMValueRef raw = lvalue->load(builder);
    LLVMValueRef result;
    switch (p.fOperator) {
        case Token::PLUSPLUS:
            switch (this->typeKind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    result = LLVMBuildAdd(builder, raw, one, "++");
                    break;
                case kFloat_TypeKind:
                    result = LLVMBuildFAdd(builder, raw, one, "++");
                    break;
                default:
                    ABORT("unsupported typeKind");
            }
            break;
        case Token::MINUSMINUS:
            switch (this->typeKind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    result = LLVMBuildSub(builder, raw, one, "--");
                    break;
                case kFloat_TypeKind:
                    result = LLVMBuildFSub(builder, raw, one, "--");
                    break;
                default:
                    ABORT("unsupported typeKind");
            }
            break;
        default:
            ABORT("unsupported prefix op");
    }
    lvalue->store(builder, result);
    return result;
}

LLVMValueRef JIT::compileVariableReference(LLVMBuilderRef builder, const VariableReference& v) {
    const Variable& var = v.fVariable;
    if (Variable::kParameter_Storage == var.fStorage &&
        !(var.fModifiers.fFlags & Modifiers::kOut_Flag) &&
        fPromotedParameters.find(&var) == fPromotedParameters.end()) {
        return fVariables[&var];
    }
    return LLVMBuildLoad(builder, fVariables[&var], String(var.fName).c_str());
}

void JIT::appendStage(LLVMBuilderRef builder, const AppendStage& a) {
    SkASSERT(a.fArguments.size() >= 1);
    SkASSERT(a.fArguments[0]->fType == *fCompiler.context().fSkRasterPipeline_Type);
    LLVMValueRef pipeline = this->compileExpression(builder, *a.fArguments[0]);
    LLVMValueRef stage = LLVMConstInt(fInt32Type, a.fStage, 0);
    switch (a.fStage) {
        case SkRasterPipeline::callback: {
            SkASSERT(a.fArguments.size() == 2);
            SkASSERT(a.fArguments[1]->fKind == Expression::kFunctionReference_Kind);
            const FunctionDeclaration& functionDecl =
                                             *((FunctionReference&) *a.fArguments[1]).fFunctions[0];
            bool found = false;
            for (const auto& pe : *fProgram) {
                if (ProgramElement::kFunction_Kind == pe.fKind) {
                    const FunctionDefinition& def = (const FunctionDefinition&) pe;
                    if (&def.fDeclaration == &functionDecl) {
                        LLVMValueRef fn = this->compileStageFunction(def);
                        LLVMValueRef args[2] = {
                            pipeline,
                            LLVMBuildBitCast(builder, fn, fInt8PtrType, "callback cast")
                        };
                        LLVMBuildCall(builder, fAppendCallbackFunc, args, 2, "");
                        found = true;
                        break;
                    }
                }
            }
            SkASSERT(found);
            break;
        }
        default: {
            LLVMValueRef ctx;
            if (a.fArguments.size() == 2) {
                ctx = this->compileExpression(builder, *a.fArguments[1]);
                ctx = LLVMBuildBitCast(builder, ctx, fInt8PtrType, "context cast");
            } else {
                SkASSERT(a.fArguments.size() == 1);
                ctx = LLVMConstNull(fInt8PtrType);
            }
            LLVMValueRef args[3] = {
                pipeline,
                stage,
                ctx
            };
            LLVMBuildCall(builder, fAppendFunc, args, 3, "");
            break;
        }
    }
}

LLVMValueRef JIT::compileConstructor(LLVMBuilderRef builder, const Constructor& c) {
    switch (c.fType.kind()) {
        case Type::kScalar_Kind: {
            SkASSERT(c.fArguments.size() == 1);
            TypeKind from = this->typeKind(c.fArguments[0]->fType);
            TypeKind to = this->typeKind(c.fType);
            LLVMValueRef base = this->compileExpression(builder, *c.fArguments[0]);
            switch (to) {
                case kFloat_TypeKind:
                    switch (from) {
                        case kInt_TypeKind:
                            return LLVMBuildSIToFP(builder, base, this->getType(c.fType), "cast");
                        case kUInt_TypeKind:
                            return LLVMBuildUIToFP(builder, base, this->getType(c.fType), "cast");
                        case kFloat_TypeKind:
                            return base;
                        case kBool_TypeKind:
                            SkASSERT(false);
                    }
                case kInt_TypeKind:
                    switch (from) {
                        case kInt_TypeKind:
                            return base;
                        case kUInt_TypeKind:
                            return base;
                        case kFloat_TypeKind:
                            return LLVMBuildFPToSI(builder, base, this->getType(c.fType), "cast");
                        case kBool_TypeKind:
                            SkASSERT(false);
                    }
                case kUInt_TypeKind:
                    switch (from) {
                        case kInt_TypeKind:
                            return base;
                        case kUInt_TypeKind:
                            return base;
                        case kFloat_TypeKind:
                            return LLVMBuildFPToUI(builder, base, this->getType(c.fType), "cast");
                        case kBool_TypeKind:
                            SkASSERT(false);
                    }
                case kBool_TypeKind:
                    SkASSERT(false);
            }
        }
        case Type::kVector_Kind: {
            LLVMValueRef vec = LLVMGetUndef(this->getType(c.fType));
            if (c.fArguments.size() == 1 && c.fArguments[0]->fType.kind() == Type::kScalar_Kind) {
                LLVMValueRef value = this->compileExpression(builder, *c.fArguments[0]);
                for (int i = 0; i < c.fType.columns(); ++i) {
                    vec = LLVMBuildInsertElement(builder, vec, value,
                                                 LLVMConstInt(fInt32Type, i, false),
                                                 "vec build 1");
                }
            } else {
                int index = 0;
                for (const auto& arg : c.fArguments) {
                    LLVMValueRef value = this->compileExpression(builder, *arg);
                    if (arg->fType.kind() == Type::kVector_Kind) {
                        for (int i = 0; i < arg->fType.columns(); ++i) {
                            LLVMValueRef column = LLVMBuildExtractElement(builder,
                                                                          vec,
                                                                          LLVMConstInt(fInt32Type,
                                                                                       i,
                                                                                       false),
                                                                          "construct extract");
                            vec = LLVMBuildInsertElement(builder, vec, column,
                                                         LLVMConstInt(fInt32Type, index++, false),
                                                         "vec build 2");
                        }
                    } else {
                        vec = LLVMBuildInsertElement(builder, vec, value,
                                                     LLVMConstInt(fInt32Type, index++, false),
                                                     "vec build 3");
                    }
                }
            }
            return vec;
        }
        default:
            break;
    }
    ABORT("unsupported constructor");
}

LLVMValueRef JIT::compileSwizzle(LLVMBuilderRef builder, const Swizzle& s) {
    LLVMValueRef base = this->compileExpression(builder, *s.fBase);
    if (s.fComponents.size() > 1) {
        LLVMValueRef result = LLVMGetUndef(this->getType(s.fType));
        for (size_t i = 0; i < s.fComponents.size(); ++i) {
            LLVMValueRef element = LLVMBuildExtractElement(
                                                       builder,
                                                       base,
                                                       LLVMConstInt(fInt32Type,
                                                                    s.fComponents[i],
                                                                    false),
                                                       "swizzle extract");
            result = LLVMBuildInsertElement(builder, result, element,
                                            LLVMConstInt(fInt32Type, i, false),
                                            "swizzle insert");
        }
        return result;
    }
    SkASSERT(s.fComponents.size() == 1);
    return LLVMBuildExtractElement(builder, base,
                                            LLVMConstInt(fInt32Type,
                                                         s.fComponents[0],
                                                         false),
                                            "swizzle extract");
}

LLVMValueRef JIT::compileTernary(LLVMBuilderRef builder, const TernaryExpression& t) {
    LLVMValueRef test = this->compileExpression(builder, *t.fTest);
    LLVMBasicBlockRef trueBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                "if true");
    LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                            "if merge");
    LLVMBasicBlockRef falseBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                 "if false");
    LLVMBuildCondBr(builder, test, trueBlock, falseBlock);
    this->setBlock(builder, trueBlock);
    LLVMValueRef ifTrue = this->compileExpression(builder, *t.fIfTrue);
    trueBlock = fCurrentBlock;
    LLVMBuildBr(builder, merge);
    this->setBlock(builder, falseBlock);
    LLVMValueRef ifFalse = this->compileExpression(builder, *t.fIfFalse);
    falseBlock = fCurrentBlock;
    LLVMBuildBr(builder, merge);
    this->setBlock(builder, merge);
    LLVMValueRef phi = LLVMBuildPhi(builder, this->getType(t.fType), "?");
    LLVMValueRef incomingValues[2] = { ifTrue, ifFalse };
    LLVMBasicBlockRef incomingBlocks[2] = { trueBlock, falseBlock };
    LLVMAddIncoming(phi, incomingValues, incomingBlocks, 2);
    return phi;
}

LLVMValueRef JIT::compileExpression(LLVMBuilderRef builder, const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kAppendStage_Kind: {
            this->appendStage(builder, (const AppendStage&) expr);
            return LLVMValueRef();
        }
        case Expression::kBinary_Kind:
            return this->compileBinary(builder, (BinaryExpression&) expr);
        case Expression::kBoolLiteral_Kind:
            return LLVMConstInt(fInt1Type, ((BoolLiteral&) expr).fValue, false);
        case Expression::kConstructor_Kind:
            return this->compileConstructor(builder, (Constructor&) expr);
        case Expression::kIntLiteral_Kind:
            return LLVMConstInt(this->getType(expr.fType), ((IntLiteral&) expr).fValue, true);
        case Expression::kFieldAccess_Kind:
            abort();
        case Expression::kFloatLiteral_Kind:
            return LLVMConstReal(this->getType(expr.fType), ((FloatLiteral&) expr).fValue);
        case Expression::kFunctionCall_Kind:
            return this->compileFunctionCall(builder, (FunctionCall&) expr);
        case Expression::kIndex_Kind:
            return this->compileIndex(builder, (IndexExpression&) expr);
        case Expression::kPrefix_Kind:
            return this->compilePrefix(builder, (PrefixExpression&) expr);
        case Expression::kPostfix_Kind:
            return this->compilePostfix(builder, (PostfixExpression&) expr);
        case Expression::kSetting_Kind:
            abort();
        case Expression::kSwizzle_Kind:
            return this->compileSwizzle(builder, (Swizzle&) expr);
        case Expression::kVariableReference_Kind:
            return this->compileVariableReference(builder, (VariableReference&) expr);
        case Expression::kTernary_Kind:
            return this->compileTernary(builder, (TernaryExpression&) expr);
        case Expression::kTypeReference_Kind:
            abort();
        default:
            abort();
    }
    ABORT("unsupported expression: %s\n", expr.description().c_str());
}

void JIT::compileBlock(LLVMBuilderRef builder, const Block& block) {
    for (const auto& stmt : block.fStatements) {
        this->compileStatement(builder, *stmt);
    }
}

void JIT::compileVarDeclarations(LLVMBuilderRef builder, const VarDeclarationsStatement& decls) {
    for (const auto& declStatement : decls.fDeclaration->fVars) {
        const VarDeclaration& decl = (VarDeclaration&) *declStatement;
        LLVMPositionBuilderAtEnd(builder, fAllocaBlock);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, this->getType(decl.fVar->fType),
                                              String(decl.fVar->fName).c_str());
        fVariables[decl.fVar] = alloca;
        LLVMPositionBuilderAtEnd(builder, fCurrentBlock);
        if (decl.fValue) {
            LLVMValueRef result = this->compileExpression(builder, *decl.fValue);
            LLVMBuildStore(builder, result, alloca);
        }
    }
}

void JIT::compileIf(LLVMBuilderRef builder, const IfStatement& i) {
    LLVMValueRef test = this->compileExpression(builder, *i.fTest);
    LLVMBasicBlockRef ifTrue = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "if true");
    LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                              "if merge");
    LLVMBasicBlockRef ifFalse;
    if (i.fIfFalse) {
        ifFalse = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "if false");
    } else {
        ifFalse = merge;
    }
    LLVMBuildCondBr(builder, test, ifTrue, ifFalse);
    this->setBlock(builder, ifTrue);
    this->compileStatement(builder, *i.fIfTrue);
    if (!ends_with_branch(*i.fIfTrue)) {
        LLVMBuildBr(builder, merge);
    }
    if (i.fIfFalse) {
        this->setBlock(builder, ifFalse);
        this->compileStatement(builder, *i.fIfFalse);
        if (!ends_with_branch(*i.fIfFalse)) {
            LLVMBuildBr(builder, merge);
        }
    }
    this->setBlock(builder, merge);
}

void JIT::compileFor(LLVMBuilderRef builder, const ForStatement& f) {
    if (f.fInitializer) {
        this->compileStatement(builder, *f.fInitializer);
    }
    LLVMBasicBlockRef start;
    LLVMBasicBlockRef body = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "for body");
    LLVMBasicBlockRef next = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "for next");
    LLVMBasicBlockRef end = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "for end");
    if (f.fTest) {
        start = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "for test");
        LLVMBuildBr(builder, start);
        this->setBlock(builder, start);
        LLVMValueRef test = this->compileExpression(builder, *f.fTest);
        LLVMBuildCondBr(builder, test, body, end);
    } else {
        start = body;
        LLVMBuildBr(builder, body);
    }
    this->setBlock(builder, body);
    fBreakTarget.push_back(end);
    fContinueTarget.push_back(next);
    this->compileStatement(builder, *f.fStatement);
    fBreakTarget.pop_back();
    fContinueTarget.pop_back();
    if (!ends_with_branch(*f.fStatement)) {
        LLVMBuildBr(builder, next);
    }
    this->setBlock(builder, next);
    if (f.fNext) {
        this->compileExpression(builder, *f.fNext);
    }
    LLVMBuildBr(builder, start);
    this->setBlock(builder, end);
}

void JIT::compileDo(LLVMBuilderRef builder, const DoStatement& d) {
    LLVMBasicBlockRef testBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                "do test");
    LLVMBasicBlockRef body = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                           "do body");
    LLVMBasicBlockRef end = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                          "do end");
    LLVMBuildBr(builder, body);
    this->setBlock(builder, testBlock);
    LLVMValueRef test = this->compileExpression(builder, *d.fTest);
    LLVMBuildCondBr(builder, test, body, end);
    this->setBlock(builder, body);
    fBreakTarget.push_back(end);
    fContinueTarget.push_back(body);
    this->compileStatement(builder, *d.fStatement);
    fBreakTarget.pop_back();
    fContinueTarget.pop_back();
    if (!ends_with_branch(*d.fStatement)) {
        LLVMBuildBr(builder, testBlock);
    }
    this->setBlock(builder, end);
}

void JIT::compileWhile(LLVMBuilderRef builder, const WhileStatement& w) {
    LLVMBasicBlockRef testBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                           "while test");
    LLVMBasicBlockRef body = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                           "while body");
    LLVMBasicBlockRef end = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                          "while end");
    LLVMBuildBr(builder, testBlock);
    this->setBlock(builder, testBlock);
    LLVMValueRef test = this->compileExpression(builder, *w.fTest);
    LLVMBuildCondBr(builder, test, body, end);
    this->setBlock(builder, body);
    fBreakTarget.push_back(end);
    fContinueTarget.push_back(testBlock);
    this->compileStatement(builder, *w.fStatement);
    fBreakTarget.pop_back();
    fContinueTarget.pop_back();
    if (!ends_with_branch(*w.fStatement)) {
        LLVMBuildBr(builder, testBlock);
    }
    this->setBlock(builder, end);
}

void JIT::compileBreak(LLVMBuilderRef builder, const BreakStatement& b) {
    LLVMBuildBr(builder, fBreakTarget.back());
}

void JIT::compileContinue(LLVMBuilderRef builder, const ContinueStatement& b) {
    LLVMBuildBr(builder, fContinueTarget.back());
}

void JIT::compileReturn(LLVMBuilderRef builder, const ReturnStatement& r) {
    if (r.fExpression) {
        LLVMBuildRet(builder, this->compileExpression(builder, *r.fExpression));
    } else {
        LLVMBuildRetVoid(builder);
    }
}

void JIT::compileStatement(LLVMBuilderRef builder, const Statement& stmt) {
    switch (stmt.fKind) {
        case Statement::kBlock_Kind:
            this->compileBlock(builder, (Block&) stmt);
            break;
        case Statement::kBreak_Kind:
            this->compileBreak(builder, (BreakStatement&) stmt);
            break;
        case Statement::kContinue_Kind:
            this->compileContinue(builder, (ContinueStatement&) stmt);
            break;
        case Statement::kDiscard_Kind:
            abort();
        case Statement::kDo_Kind:
            this->compileDo(builder, (DoStatement&) stmt);
            break;
        case Statement::kExpression_Kind:
            this->compileExpression(builder, *((ExpressionStatement&) stmt).fExpression);
            break;
        case Statement::kFor_Kind:
            this->compileFor(builder, (ForStatement&) stmt);
            break;
        case Statement::kGroup_Kind:
            abort();
        case Statement::kIf_Kind:
            this->compileIf(builder, (IfStatement&) stmt);
            break;
        case Statement::kNop_Kind:
            break;
        case Statement::kReturn_Kind:
            this->compileReturn(builder, (ReturnStatement&) stmt);
            break;
        case Statement::kSwitch_Kind:
            abort();
        case Statement::kVarDeclarations_Kind:
            this->compileVarDeclarations(builder, (VarDeclarationsStatement&) stmt);
            break;
        case Statement::kWhile_Kind:
            this->compileWhile(builder, (WhileStatement&) stmt);
            break;
        default:
            abort();
    }
}

void JIT::compileStageFunctionLoop(const FunctionDefinition& f, LLVMValueRef newFunc) {
    // loop over fVectorCount pixels, running the body of the stage function for each of them
    LLVMValueRef oldFunction = fCurrentFunction;
    fCurrentFunction = newFunc;
    std::unique_ptr<LLVMValueRef[]> params(new LLVMValueRef[STAGE_PARAM_COUNT]);
    LLVMGetParams(fCurrentFunction, params.get());
    LLVMValueRef programParam = params.get()[1];
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(fContext);
    LLVMBasicBlockRef oldAllocaBlock = fAllocaBlock;
    LLVMBasicBlockRef oldCurrentBlock = fCurrentBlock;
    fAllocaBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "alloca");
    this->setBlock(builder, fAllocaBlock);
    // temporaries to store the color channel vectors
    LLVMValueRef rVec = LLVMBuildAlloca(builder, fFloat32VectorType, "rVec");
    LLVMBuildStore(builder, params.get()[4], rVec);
    LLVMValueRef gVec = LLVMBuildAlloca(builder, fFloat32VectorType, "gVec");
    LLVMBuildStore(builder, params.get()[5], gVec);
    LLVMValueRef bVec = LLVMBuildAlloca(builder, fFloat32VectorType, "bVec");
    LLVMBuildStore(builder, params.get()[6], bVec);
    LLVMValueRef aVec = LLVMBuildAlloca(builder, fFloat32VectorType, "aVec");
    LLVMBuildStore(builder, params.get()[7], aVec);
    LLVMValueRef color = LLVMBuildAlloca(builder, fFloat32Vector4Type, "color");
    fVariables[f.fDeclaration.fParameters[1]] = LLVMBuildTrunc(builder, params.get()[3], fInt32Type,
                                                               "y->Int32");
    fVariables[f.fDeclaration.fParameters[2]] = color;
    LLVMValueRef ivar = LLVMBuildAlloca(builder, fInt32Type, "i");
    LLVMBuildStore(builder, LLVMConstInt(fInt32Type, 0, false), ivar);
    LLVMBasicBlockRef start = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "start");
    this->setBlock(builder, start);
    LLVMValueRef iload = LLVMBuildLoad(builder, ivar, "load i");
    fVariables[f.fDeclaration.fParameters[0]] = LLVMBuildAdd(builder,
                                                             LLVMBuildTrunc(builder,
                                                                            params.get()[2],
                                                                            fInt32Type,
                                                                            "x->Int32"),
                                                             iload,
                                                             "x");
    LLVMValueRef vectorSize = LLVMConstInt(fInt32Type, fVectorCount, false);
    LLVMValueRef test = LLVMBuildICmp(builder, LLVMIntSLT, iload, vectorSize, "i < vectorSize");
    LLVMBasicBlockRef loopBody = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "body");
    LLVMBasicBlockRef loopEnd = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "end");
    LLVMBuildCondBr(builder, test, loopBody, loopEnd);
    this->setBlock(builder, loopBody);
    LLVMValueRef vec = LLVMGetUndef(fFloat32Vector4Type);
    // extract the r, g, b, and a values from the color channel vectors and store them into "color"
    for (int i = 0; i < 4; ++i) {
        vec = LLVMBuildInsertElement(builder, vec,
                                     LLVMBuildExtractElement(builder,
                                                             params.get()[4 + i],
                                                             iload, "initial"),
                                     LLVMConstInt(fInt32Type, i, false),
                                     "vec build");
    }
    LLVMBuildStore(builder, vec, color);
    // write actual loop body
    this->compileStatement(builder, *f.fBody);
    // extract the r, g, b, and a values from "color" and stick them back into the color channel
    // vectors
    LLVMValueRef colorLoad = LLVMBuildLoad(builder, color, "color load");
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, rVec, "rVec"),
                                          LLVMBuildExtractElement(builder, colorLoad,
                                                                  LLVMConstInt(fInt32Type, 0,
                                                                               false),
                                                                  "rExtract"),
                                          iload, "rInsert"),
                   rVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, gVec, "gVec"),
                                          LLVMBuildExtractElement(builder, colorLoad,
                                                                  LLVMConstInt(fInt32Type, 1,
                                                                               false),
                                                                  "gExtract"),
                                          iload, "gInsert"),
                   gVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, bVec, "bVec"),
                                          LLVMBuildExtractElement(builder, colorLoad,
                                                                  LLVMConstInt(fInt32Type, 2,
                                                                               false),
                                                                  "bExtract"),
                                          iload, "bInsert"),
                   bVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, aVec, "aVec"),
                                          LLVMBuildExtractElement(builder, colorLoad,
                                                                  LLVMConstInt(fInt32Type, 3,
                                                                               false),
                                                                  "aExtract"),
                                          iload, "aInsert"),
                   aVec);
    LLVMValueRef inc = LLVMBuildAdd(builder, iload, LLVMConstInt(fInt32Type, 1, false), "inc i");
    LLVMBuildStore(builder, inc, ivar);
    LLVMBuildBr(builder, start);
    this->setBlock(builder, loopEnd);
    // increment program pointer, call the next stage
    LLVMValueRef rawNextPtr = LLVMBuildLoad(builder, programParam, "next load");
    LLVMTypeRef stageFuncType = LLVMTypeOf(newFunc);
    LLVMValueRef nextPtr = LLVMBuildBitCast(builder, rawNextPtr, stageFuncType, "cast next->func");
    LLVMValueRef nextInc = LLVMBuildIntToPtr(builder,
                                             LLVMBuildAdd(builder,
                                                          LLVMBuildPtrToInt(builder,
                                                                            programParam,
                                                                            fInt64Type,
                                                                            "cast 1"),
                                                          LLVMConstInt(fInt64Type, PTR_SIZE, false),
                                                          "add"),
                                            LLVMPointerType(fInt8PtrType, 0), "cast 2");
    LLVMValueRef args[STAGE_PARAM_COUNT] = {
        params.get()[0],
        nextInc,
        params.get()[2],
        params.get()[3],
        LLVMBuildLoad(builder, rVec, "rVec"),
        LLVMBuildLoad(builder, gVec, "gVec"),
        LLVMBuildLoad(builder, bVec, "bVec"),
        LLVMBuildLoad(builder, aVec, "aVec"),
        params.get()[8],
        params.get()[9],
        params.get()[10],
        params.get()[11]
    };
    LLVMBuildCall(builder, nextPtr, args, STAGE_PARAM_COUNT, "");
    LLVMBuildRetVoid(builder);
    // finish
    LLVMPositionBuilderAtEnd(builder, fAllocaBlock);
    LLVMBuildBr(builder, start);
    LLVMDisposeBuilder(builder);
    if (LLVMVerifyFunction(fCurrentFunction, LLVMPrintMessageAction)) {
        ABORT("verify failed\n");
    }
    fAllocaBlock = oldAllocaBlock;
    fCurrentBlock = oldCurrentBlock;
    fCurrentFunction = oldFunction;
}

// FIXME maybe pluggable code generators? Need to do something to separate all
// of the normal codegen from the vector codegen and break this up into multiple
// classes.

bool JIT::getVectorLValue(LLVMBuilderRef builder, const Expression& e,
                          LLVMValueRef out[CHANNELS]) {
    switch (e.fKind) {
        case Expression::kVariableReference_Kind:
            if (fColorParam == &((VariableReference&) e).fVariable) {
                memcpy(out, fChannels, sizeof(fChannels));
                return true;
            }
            return false;
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) e;
            LLVMValueRef base[CHANNELS];
            if (!this->getVectorLValue(builder, *s.fBase, base)) {
                return false;
            }
            for (size_t i = 0; i < s.fComponents.size(); ++i) {
                out[i] = base[s.fComponents[i]];
            }
            return true;
        }
        default:
            return false;
    }
}

bool JIT::getVectorBinaryOperands(LLVMBuilderRef builder, const Expression& left,
                                  LLVMValueRef outLeft[CHANNELS], const Expression& right,
                                  LLVMValueRef outRight[CHANNELS]) {
    if (!this->compileVectorExpression(builder, left, outLeft)) {
        return false;
    }
    int leftColumns = left.fType.columns();
    int rightColumns = right.fType.columns();
    if (leftColumns == 1 && rightColumns > 1) {
        for (int i = 1; i < rightColumns; ++i) {
            outLeft[i] = outLeft[0];
        }
    }
    if (!this->compileVectorExpression(builder, right, outRight)) {
        return false;
    }
    if (rightColumns == 1 && leftColumns > 1) {
        for (int i = 1; i < leftColumns; ++i) {
            outRight[i] = outRight[0];
        }
    }
    return true;
}

bool JIT::compileVectorBinary(LLVMBuilderRef builder, const BinaryExpression& b,
                              LLVMValueRef out[CHANNELS]) {
    LLVMValueRef left[CHANNELS];
    LLVMValueRef right[CHANNELS];
    #define VECTOR_BINARY(signedOp, unsignedOp, floatOp) {                               \
        if (!this->getVectorBinaryOperands(builder, *b.fLeft, left, *b.fRight, right)) { \
            return false;                                                                \
        }                                                                                \
        for (int i = 0; i < b.fLeft->fType.columns(); ++i) {                             \
            switch (this->typeKind(b.fLeft->fType)) {                                    \
                case kInt_TypeKind:                                                      \
                    out[i] = signedOp(builder, left[i], right[i], "binary");             \
                    break;                                                               \
                case kUInt_TypeKind:                                                     \
                    out[i] = unsignedOp(builder, left[i], right[i], "binary");           \
                    break;                                                               \
                case kFloat_TypeKind:                                                    \
                    out[i] = floatOp(builder, left[i], right[i], "binary");              \
                    break;                                                               \
                case kBool_TypeKind:                                                     \
                    SkASSERT(false);                                                       \
                    break;                                                               \
            }                                                                            \
        }                                                                                \
        return true;                                                                     \
    }
    switch (b.fOperator) {
        case Token::EQ: {
            if (!this->getVectorLValue(builder, *b.fLeft, left)) {
                return false;
            }
            if (!this->compileVectorExpression(builder, *b.fRight, right)) {
                return false;
            }
            int columns = b.fRight->fType.columns();
            for (int i = 0; i < columns; ++i) {
                LLVMBuildStore(builder, right[i], left[i]);
            }
            return true;
        }
        case Token::PLUS:
            VECTOR_BINARY(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
        case Token::MINUS:
            VECTOR_BINARY(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
        case Token::STAR:
            VECTOR_BINARY(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
        case Token::SLASH:
            VECTOR_BINARY(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
        case Token::PERCENT:
            VECTOR_BINARY(LLVMBuildSRem, LLVMBuildURem, LLVMBuildSRem);
        case Token::BITWISEAND:
            VECTOR_BINARY(LLVMBuildAnd, LLVMBuildAnd, LLVMBuildAnd);
        case Token::BITWISEOR:
            VECTOR_BINARY(LLVMBuildOr, LLVMBuildOr, LLVMBuildOr);
        default:
            printf("unsupported operator: %s\n", b.description().c_str());
            return false;
    }
}

bool JIT::compileVectorConstructor(LLVMBuilderRef builder, const Constructor& c,
                                   LLVMValueRef out[CHANNELS]) {
    switch (c.fType.kind()) {
        case Type::kScalar_Kind: {
            SkASSERT(c.fArguments.size() == 1);
            TypeKind from = this->typeKind(c.fArguments[0]->fType);
            TypeKind to = this->typeKind(c.fType);
            LLVMValueRef base[CHANNELS];
            if (!this->compileVectorExpression(builder, *c.fArguments[0], base)) {
                return false;
            }
            #define CONSTRUCT(fn)                                                                \
                out[0] = LLVMGetUndef(LLVMVectorType(this->getType(c.fType), fVectorCount));     \
                for (int i = 0; i < fVectorCount; ++i) {                                         \
                    LLVMValueRef index = LLVMConstInt(fInt32Type, i, false);                     \
                    LLVMValueRef baseVal = LLVMBuildExtractElement(builder, base[0], index,      \
                                                                   "construct extract");         \
                    out[0] = LLVMBuildInsertElement(builder, out[0],                             \
                                                    fn(builder, baseVal, this->getType(c.fType), \
                                                       "cast"),                                  \
                                                    index, "construct insert");                  \
                }                                                                                \
                return true;
            if (kFloat_TypeKind == to) {
                if (kInt_TypeKind == from) {
                    CONSTRUCT(LLVMBuildSIToFP);
                }
                if (kUInt_TypeKind == from) {
                    CONSTRUCT(LLVMBuildUIToFP);
                }
            }
            if (kInt_TypeKind == to) {
                if (kFloat_TypeKind == from) {
                    CONSTRUCT(LLVMBuildFPToSI);
                }
                if (kUInt_TypeKind == from) {
                    return true;
                }
            }
            if (kUInt_TypeKind == to) {
                if (kFloat_TypeKind == from) {
                    CONSTRUCT(LLVMBuildFPToUI);
                }
                if (kInt_TypeKind == from) {
                    return base;
                }
            }
            printf("%s\n", c.description().c_str());
            ABORT("unsupported constructor");
        }
        case Type::kVector_Kind: {
            if (c.fArguments.size() == 1) {
                LLVMValueRef base[CHANNELS];
                if (!this->compileVectorExpression(builder, *c.fArguments[0], base)) {
                    return false;
                }
                for (int i = 0; i < c.fType.columns(); ++i) {
                    out[i] = base[0];
                }
            } else {
                SkASSERT(c.fArguments.size() == (size_t) c.fType.columns());
                for (int i = 0; i < c.fType.columns(); ++i) {
                    LLVMValueRef base[CHANNELS];
                    if (!this->compileVectorExpression(builder, *c.fArguments[i], base)) {
                        return false;
                    }
                    out[i] = base[0];
                }
            }
            return true;
        }
        default:
            break;
    }
    ABORT("unsupported constructor");
}

bool JIT::compileVectorFloatLiteral(LLVMBuilderRef builder,
                                    const FloatLiteral& f,
                                    LLVMValueRef out[CHANNELS]) {
    LLVMValueRef value = LLVMConstReal(this->getType(f.fType), f.fValue);
    LLVMValueRef values[MAX_VECTOR_COUNT];
    for (int i = 0; i < fVectorCount; ++i) {
        values[i] = value;
    }
    out[0] = LLVMConstVector(values, fVectorCount);
    return true;
}


bool JIT::compileVectorSwizzle(LLVMBuilderRef builder, const Swizzle& s,
                               LLVMValueRef out[CHANNELS]) {
    LLVMValueRef all[CHANNELS];
    if (!this->compileVectorExpression(builder, *s.fBase, all)) {
        return false;
    }
    for (size_t i = 0; i < s.fComponents.size(); ++i) {
        out[i] = all[s.fComponents[i]];
    }
    return true;
}

bool JIT::compileVectorVariableReference(LLVMBuilderRef builder, const VariableReference& v,
                                         LLVMValueRef out[CHANNELS]) {
    if (&v.fVariable == fColorParam) {
        for (int i = 0; i < CHANNELS; ++i) {
            out[i] = LLVMBuildLoad(builder, fChannels[i], "variable reference");
        }
        return true;
    }
    return false;
}

bool JIT::compileVectorExpression(LLVMBuilderRef builder, const Expression& expr,
                                  LLVMValueRef out[CHANNELS]) {
    switch (expr.fKind) {
        case Expression::kBinary_Kind:
            return this->compileVectorBinary(builder, (const BinaryExpression&) expr, out);
        case Expression::kConstructor_Kind:
            return this->compileVectorConstructor(builder, (const Constructor&) expr, out);
        case Expression::kFloatLiteral_Kind:
            return this->compileVectorFloatLiteral(builder, (const FloatLiteral&) expr, out);
        case Expression::kSwizzle_Kind:
            return this->compileVectorSwizzle(builder, (const Swizzle&) expr, out);
        case Expression::kVariableReference_Kind:
            return this->compileVectorVariableReference(builder, (const VariableReference&) expr,
                                                        out);
        default:
            return false;
    }
}

bool JIT::compileVectorStatement(LLVMBuilderRef builder, const Statement& stmt) {
    switch (stmt.fKind) {
        case Statement::kBlock_Kind:
            for (const auto& s : ((const Block&) stmt).fStatements) {
                if (!this->compileVectorStatement(builder, *s)) {
                    return false;
                }
            }
            return true;
        case Statement::kExpression_Kind:
            LLVMValueRef result;
            return this->compileVectorExpression(builder,
                                                 *((const ExpressionStatement&) stmt).fExpression,
                                                 &result);
        default:
            return false;
    }
}

bool JIT::compileStageFunctionVector(const FunctionDefinition& f, LLVMValueRef newFunc) {
    LLVMValueRef oldFunction = fCurrentFunction;
    fCurrentFunction = newFunc;
    std::unique_ptr<LLVMValueRef[]> params(new LLVMValueRef[STAGE_PARAM_COUNT]);
    LLVMGetParams(fCurrentFunction, params.get());
    LLVMValueRef programParam = params.get()[1];
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(fContext);
    LLVMBasicBlockRef oldAllocaBlock = fAllocaBlock;
    LLVMBasicBlockRef oldCurrentBlock = fCurrentBlock;
    fAllocaBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "alloca");
    this->setBlock(builder, fAllocaBlock);
    fChannels[0] = LLVMBuildAlloca(builder, fFloat32VectorType, "rVec");
    LLVMBuildStore(builder, params.get()[4], fChannels[0]);
    fChannels[1] = LLVMBuildAlloca(builder, fFloat32VectorType, "gVec");
    LLVMBuildStore(builder, params.get()[5], fChannels[1]);
    fChannels[2] = LLVMBuildAlloca(builder, fFloat32VectorType, "bVec");
    LLVMBuildStore(builder, params.get()[6], fChannels[2]);
    fChannels[3] = LLVMBuildAlloca(builder, fFloat32VectorType, "aVec");
    LLVMBuildStore(builder, params.get()[7], fChannels[3]);
    LLVMBasicBlockRef start = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "start");
    this->setBlock(builder, start);
    bool success = this->compileVectorStatement(builder, *f.fBody);
    if (success) {
        // increment program pointer, call next
        LLVMValueRef rawNextPtr = LLVMBuildLoad(builder, programParam, "next load");
        LLVMTypeRef stageFuncType = LLVMTypeOf(newFunc);
        LLVMValueRef nextPtr = LLVMBuildBitCast(builder, rawNextPtr, stageFuncType,
                                                "cast next->func");
        LLVMValueRef nextInc = LLVMBuildIntToPtr(builder,
                                                 LLVMBuildAdd(builder,
                                                              LLVMBuildPtrToInt(builder,
                                                                                programParam,
                                                                                fInt64Type,
                                                                                "cast 1"),
                                                              LLVMConstInt(fInt64Type, PTR_SIZE,
                                                                           false),
                                                              "add"),
                                                LLVMPointerType(fInt8PtrType, 0), "cast 2");
        LLVMValueRef args[STAGE_PARAM_COUNT] = {
            params.get()[0],
            nextInc,
            params.get()[2],
            params.get()[3],
            LLVMBuildLoad(builder, fChannels[0], "rVec"),
            LLVMBuildLoad(builder, fChannels[1], "gVec"),
            LLVMBuildLoad(builder, fChannels[2], "bVec"),
            LLVMBuildLoad(builder, fChannels[3], "aVec"),
            params.get()[8],
            params.get()[9],
            params.get()[10],
            params.get()[11]
        };
        LLVMBuildCall(builder, nextPtr, args, STAGE_PARAM_COUNT, "");
        LLVMBuildRetVoid(builder);
        // finish
        LLVMPositionBuilderAtEnd(builder, fAllocaBlock);
        LLVMBuildBr(builder, start);
        LLVMDisposeBuilder(builder);
        if (LLVMVerifyFunction(fCurrentFunction, LLVMPrintMessageAction)) {
            ABORT("verify failed\n");
        }
    } else {
        LLVMDeleteBasicBlock(fAllocaBlock);
        LLVMDeleteBasicBlock(start);
    }

    fAllocaBlock = oldAllocaBlock;
    fCurrentBlock = oldCurrentBlock;
    fCurrentFunction = oldFunction;
    return success;
}

LLVMValueRef JIT::compileStageFunction(const FunctionDefinition& f) {
    LLVMTypeRef returnType = fVoidType;
    LLVMTypeRef parameterTypes[12] = { fSizeTType, LLVMPointerType(fInt8PtrType, 0), fSizeTType,
                                       fSizeTType, fFloat32VectorType, fFloat32VectorType,
                                       fFloat32VectorType, fFloat32VectorType, fFloat32VectorType,
                                       fFloat32VectorType, fFloat32VectorType, fFloat32VectorType };
    LLVMTypeRef stageFuncType = LLVMFunctionType(returnType, parameterTypes, 12, false);
    LLVMValueRef result = LLVMAddFunction(fModule,
                                          (String(f.fDeclaration.fName) + "$stage").c_str(),
                                          stageFuncType);
    fColorParam = f.fDeclaration.fParameters[2];
    if (!this->compileStageFunctionVector(f, result)) {
        // vectorization failed, fall back to looping over the pixels
        this->compileStageFunctionLoop(f, result);
    }
    return result;
}

bool JIT::hasStageSignature(const FunctionDeclaration& f) {
    return f.fReturnType == *fProgram->fContext->fVoid_Type &&
           f.fParameters.size() == 3 &&
           f.fParameters[0]->fType == *fProgram->fContext->fInt_Type &&
           f.fParameters[0]->fModifiers.fFlags == 0 &&
           f.fParameters[1]->fType == *fProgram->fContext->fInt_Type &&
           f.fParameters[1]->fModifiers.fFlags == 0 &&
           f.fParameters[2]->fType == *fProgram->fContext->fHalf4_Type &&
           f.fParameters[2]->fModifiers.fFlags == (Modifiers::kIn_Flag | Modifiers::kOut_Flag);
}

LLVMValueRef JIT::compileFunction(const FunctionDefinition& f) {
    if (this->hasStageSignature(f.fDeclaration)) {
        this->compileStageFunction(f);
        // we compile foo$stage *in addition* to compiling foo, as we can't be sure that the intent
        // was to produce an SkJumper stage just because the signature matched or that the function
        // is not otherwise called. May need a better way to handle this.
    }
    LLVMTypeRef returnType = this->getType(f.fDeclaration.fReturnType);
    std::vector<LLVMTypeRef> parameterTypes;
    for (const auto& p : f.fDeclaration.fParameters) {
        LLVMTypeRef type = this->getType(p->fType);
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            type = LLVMPointerType(type, 0);
        }
        parameterTypes.push_back(type);
    }
    fCurrentFunction  = LLVMAddFunction(fModule,
                                        String(f.fDeclaration.fName).c_str(),
                                        LLVMFunctionType(returnType, parameterTypes.data(),
                                                         parameterTypes.size(), false));
    fFunctions[&f.fDeclaration] = fCurrentFunction;

    std::unique_ptr<LLVMValueRef[]> params(new LLVMValueRef[parameterTypes.size()]);
    LLVMGetParams(fCurrentFunction, params.get());
    for (size_t i = 0; i < f.fDeclaration.fParameters.size(); ++i) {
        fVariables[f.fDeclaration.fParameters[i]] = params.get()[i];
    }
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(fContext);
    fAllocaBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "alloca");
    LLVMBasicBlockRef start = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "start");
    fCurrentBlock = start;
    LLVMPositionBuilderAtEnd(builder, fCurrentBlock);
    this->compileStatement(builder, *f.fBody);
    if (!ends_with_branch(*f.fBody)) {
        if (f.fDeclaration.fReturnType == *fProgram->fContext->fVoid_Type) {
            LLVMBuildRetVoid(builder);
        } else {
            LLVMBuildUnreachable(builder);
        }
    }
    LLVMPositionBuilderAtEnd(builder, fAllocaBlock);
    LLVMBuildBr(builder, start);
    LLVMDisposeBuilder(builder);
    if (LLVMVerifyFunction(fCurrentFunction, LLVMPrintMessageAction)) {
        ABORT("verify failed\n");
    }
    return fCurrentFunction;
}

void JIT::createModule() {
    fPromotedParameters.clear();
    fModule = LLVMModuleCreateWithNameInContext("skslmodule", fContext);
    this->loadBuiltinFunctions();
    LLVMTypeRef fold2Params[1] = { fInt1Vector2Type };
    fFoldAnd2Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.and.i1.v2i1",
                                    LLVMFunctionType(fInt1Type, fold2Params, 1, false));
    fFoldOr2Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.or.i1.v2i1",
                                   LLVMFunctionType(fInt1Type, fold2Params, 1, false));
    LLVMTypeRef fold3Params[1] = { fInt1Vector3Type };
    fFoldAnd3Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.and.i1.v3i1",
                                    LLVMFunctionType(fInt1Type, fold3Params, 1, false));
    fFoldOr3Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.or.i1.v3i1",
                                   LLVMFunctionType(fInt1Type, fold3Params, 1, false));
    LLVMTypeRef fold4Params[1] = { fInt1Vector4Type };
    fFoldAnd4Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.and.i1.v4i1",
                                    LLVMFunctionType(fInt1Type, fold4Params, 1, false));
    fFoldOr4Func = LLVMAddFunction(fModule, "llvm.experimental.vector.reduce.or.i1.v4i1",
                                   LLVMFunctionType(fInt1Type, fold4Params, 1, false));
    // LLVM doesn't do void*, have to declare it as int8*
    LLVMTypeRef appendParams[3] = { fInt8PtrType, fInt32Type, fInt8PtrType };
    fAppendFunc = LLVMAddFunction(fModule, "sksl_pipeline_append", LLVMFunctionType(fVoidType,
                                                                                    appendParams,
                                                                                    3,
                                                                                    false));
    LLVMTypeRef appendCallbackParams[2] = { fInt8PtrType, fInt8PtrType };
    fAppendCallbackFunc = LLVMAddFunction(fModule, "sksl_pipeline_append_callback",
                                          LLVMFunctionType(fVoidType, appendCallbackParams, 2,
                                                           false));

    LLVMTypeRef debugParams[3] = { fFloat32Type };
    fDebugFunc = LLVMAddFunction(fModule, "sksl_debug_print", LLVMFunctionType(fVoidType,
                                                                               debugParams,
                                                                               1,
                                                                               false));

    for (const auto& e : *fProgram) {
        if (e.fKind == ProgramElement::kFunction_Kind) {
            this->compileFunction((FunctionDefinition&) e);
        }
    }
}

std::unique_ptr<JIT::Module> JIT::compile(std::unique_ptr<Program> program) {
    fCompiler.optimize(*program);
    fProgram = std::move(program);
    this->createModule();
    this->optimize();
    return std::unique_ptr<Module>(new Module(std::move(fProgram), fSharedModule, fJITStack));
}

void JIT::optimize() {
    LLVMPassManagerBuilderRef pmb = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(pmb, 3);
    LLVMPassManagerRef functionPM = LLVMCreateFunctionPassManagerForModule(fModule);
    LLVMPassManagerBuilderPopulateFunctionPassManager(pmb, functionPM);
    LLVMPassManagerRef modulePM = LLVMCreatePassManager();
    LLVMPassManagerBuilderPopulateModulePassManager(pmb, modulePM);
    LLVMInitializeFunctionPassManager(functionPM);

    LLVMValueRef func = LLVMGetFirstFunction(fModule);
    for (;;) {
        if (!func) {
            break;
        }
        LLVMRunFunctionPassManager(functionPM, func);
        func = LLVMGetNextFunction(func);
    }
    LLVMRunPassManager(modulePM, fModule);
    LLVMDisposePassManager(functionPM);
    LLVMDisposePassManager(modulePM);
    LLVMPassManagerBuilderDispose(pmb);

    std::string error_string;
    if (LLVMLoadLibraryPermanently(nullptr)) {
        ABORT("LLVMLoadLibraryPermanently failed");
    }
    char* defaultTriple = LLVMGetDefaultTargetTriple();
    char* error;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(defaultTriple, &target, &error)) {
        ABORT("LLVMGetTargetFromTriple failed");
    }

    if (!LLVMTargetHasJIT(target)) {
        ABORT("!LLVMTargetHasJIT");
    }
    LLVMTargetMachineRef targetMachine = LLVMCreateTargetMachine(target,
                                                                 defaultTriple,
                                                                 fCPU,
                                                                 nullptr,
                                                                 LLVMCodeGenLevelDefault,
                                                                 LLVMRelocDefault,
                                                                 LLVMCodeModelJITDefault);
    LLVMDisposeMessage(defaultTriple);
    LLVMTargetDataRef dataLayout = LLVMCreateTargetDataLayout(targetMachine);
    LLVMSetModuleDataLayout(fModule, dataLayout);
    LLVMDisposeTargetData(dataLayout);

    fJITStack = LLVMOrcCreateInstance(targetMachine);
    fSharedModule = LLVMOrcMakeSharedModule(fModule);
    LLVMOrcModuleHandle orcModule;
    LLVMOrcAddEagerlyCompiledIR(fJITStack, &orcModule, fSharedModule,
                                (LLVMOrcSymbolResolverFn) resolveSymbol, this);
    LLVMDisposeTargetMachine(targetMachine);
}

void* JIT::Module::getSymbol(const char* name) {
    LLVMOrcTargetAddress result;
    if (LLVMOrcGetSymbolAddress(fJITStack, &result, name)) {
        ABORT("GetSymbolAddress error");
    }
    if (!result) {
        ABORT("symbol not found");
    }
    return (void*) result;
}

void* JIT::Module::getJumperStage(const char* name) {
    return this->getSymbol((String(name) + "$stage").c_str());
}

} // namespace

#endif // SK_LLVM_AVAILABLE

#endif // SKSL_STANDALONE
