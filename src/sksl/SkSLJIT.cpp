/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#ifdef SKIA_LLVM_AVAILABLE

#include "SkSLJIT.h"

#include "SkCpu.h"
#include "SkRasterPipeline.h"
#include "../jumper/SkJumper.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionReference.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLProgram.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"

extern "C" void sksl_pipeline_append(SkRasterPipeline* p, int stage, void* ctx) {
    p->append((SkRasterPipeline::StockStage) stage, ctx);
}

#define PTR_SIZE sizeof(void*)

extern "C" void sksl_pipeline_append_callback(SkRasterPipeline* p, void* fn) {
    p->append(fn, nullptr);
}

namespace SkSL {

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

JIT::JIT(Compiler* compiler, std::unique_ptr<Program> program)
: fCompiler(*compiler)
, fProgram(std::move(program)) {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMLinkInMCJIT();
    ASSERT(!SkCpu::Supports(SkCpu::SKX)); // not yet supported
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
    this->compile();
}

JIT::~JIT() {
    LLVMOrcDisposeSharedModuleRef(fSharedModule);
    LLVMOrcDisposeInstance(fJITStack);
    LLVMContextDispose(fContext);
}

uint64_t JIT::resolveSymbol(const char* name, JIT* jit) {
    LLVMOrcTargetAddress result;
    if (!LLVMOrcGetSymbolAddress(jit->fJITStack, &result, name)) {
        if (!strcmp(name, "_sksl_pipeline_append")) {
            result = (uint64_t) &sksl_pipeline_append;
        } else if (!strcmp(name, "_sksl_pipeline_append_callback")) {
            result = (uint64_t) &sksl_pipeline_append_callback;
        } else {
            result = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name);
        }
    }
    ASSERT(result);
    return result;
}

LLVMTypeRef JIT::getType(const Type& type) {
    switch (type.kind()) {
        case Type::kOther_Kind:
            if (type.name() == "void") {
                return fVoidType;
            }
            ASSERT(type.name() == "SkRasterPipeline");
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
            ASSERT(type.name() == "bool");
            return fInt1Type;
        case Type::kArray_Kind:
            return LLVMPointerType(this->getType(type.componentType()), 0);
        default:
            printf("FAIL: %s\n", type.name().c_str());
            ABORT("unsupported type");
    }
}

void JIT::setBlock(LLVMBuilderRef builder, LLVMBasicBlockRef block) {
    fCurrentBlock = block;
    LLVMPositionBuilderAtEnd(builder, block);
}

LLVMValueRef JIT::getLValue(LLVMBuilderRef builder, const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind:
            return fVariables[&((VariableReference&) expr).fVariable];
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) expr;
            LLVMValueRef test = this->compileExpression(builder, *t.fTest);
            LLVMBasicBlockRef trueBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                        "true ? ...");
            LLVMBasicBlockRef falseBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                         "false ? ...");
            LLVMBasicBlockRef merge = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction,
                                                                    "ternary merge");
            LLVMBuildCondBr(builder, test, trueBlock, falseBlock);
            this->setBlock(builder, trueBlock);
            LLVMValueRef ifTrue = this->getLValue(builder, *t.fIfTrue);
            LLVMBuildBr(builder, merge);
            this->setBlock(builder, falseBlock);
            LLVMValueRef ifFalse = this->getLValue(builder, *t.fIfFalse);
            LLVMBuildBr(builder, merge);
            this->setBlock(builder, merge);
            LLVMTypeRef type = LLVMPointerType(this->getType(expr.fType), 0);
            LLVMValueRef phi = LLVMBuildPhi(builder, type, "?");
            LLVMValueRef incomingValues[2] = { ifTrue, ifFalse };
            LLVMBasicBlockRef incomingBlocks[2] = { trueBlock, falseBlock };
            LLVMAddIncoming(phi, incomingValues, incomingBlocks, 2);
            return phi;
        }
        default:
            ABORT("unsupported lvalue");
    }
}

static JIT::TypeKind type_kind(const Type& type) {
    if (type.fName == "int" || type.fName == "short") {
        return JIT::kInt_TypeKind;
    } else if (type.fName == "uint" || type.fName == "ushort") {
        return JIT::kUInt_TypeKind;
    } else if (type.fName == "float" || type.fName == "double") {
        return JIT::kFloat_TypeKind;
    }
    ABORT("unsupported type: %s\n", type.description().c_str());
}

LLVMValueRef JIT::compileBinary(LLVMBuilderRef builder, const BinaryExpression& b) {
    #define BINARY(SFunc, UFunc, FFunc) {                                    \
        LLVMValueRef left = this->compileExpression(builder, *b.fLeft);      \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        switch (type_kind(b.fLeft->fType)) {                                 \
            case kInt_TypeKind:                                              \
                return SFunc(builder, left, right, "binary");                \
            case kUInt_TypeKind:                                             \
                return UFunc(builder, left, right, "binary");                \
            case kFloat_TypeKind:                                            \
                return FFunc(builder, left, right, "binary");                \
            default:                                                         \
                ABORT("unsupported type_kind");                              \
        }                                                                    \
    }
    #define COMPOUND(SFunc, UFunc, FFunc) {                                  \
        LLVMValueRef lvalue = this->getLValue(builder, *b.fLeft);            \
        LLVMValueRef left = LLVMBuildLoad(builder, lvalue, "compound load"); \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        LLVMValueRef result;                                                 \
        switch (type_kind(b.fLeft->fType)) {                                 \
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
                ABORT("unsupported type_kind");                              \
        }                                                                    \
        LLVMBuildStore(builder, result, lvalue);                             \
        return result;                                                       \
    }
    #define COMPARE(SFunc, SOp, UFunc, UOp, FFunc, FOp) {                    \
        LLVMValueRef left = this->compileExpression(builder, *b.fLeft);      \
        LLVMValueRef right = this->compileExpression(builder, *b.fRight);    \
        switch (type_kind(b.fLeft->fType)) {                                 \
            case kInt_TypeKind:                                              \
                return SFunc(builder, SOp, left, right, "binary");           \
            case kUInt_TypeKind:                                             \
                return UFunc(builder, UOp, left, right, "binary");           \
            case kFloat_TypeKind:                                            \
                return FFunc(builder, FOp, left, right, "binary");           \
            default:                                                         \
                ABORT("unsupported type_kind");                              \
        }                                                                    \
    }
    switch (b.fOperator) {
        case Token::EQ: {
            LLVMValueRef lvalue = this->getLValue(builder, *b.fLeft);
            LLVMValueRef result = this->compileExpression(builder, *b.fRight);
            LLVMBuildStore(builder, result, lvalue);
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
            COMPARE(LLVMBuildICmp, LLVMIntEQ,
                    LLVMBuildICmp, LLVMIntEQ,
                    LLVMBuildFCmp, LLVMRealOEQ);
        case Token::NEQ:
            COMPARE(LLVMBuildICmp, LLVMIntNE,
                    LLVMBuildICmp, LLVMIntNE,
                    LLVMBuildFCmp, LLVMRealONE);
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
    LLVMValueRef lvalue = this->getLValue(builder, *p.fOperand);
    LLVMValueRef result = LLVMBuildLoad(builder, lvalue, "postfix");
    LLVMValueRef mod;
    LLVMValueRef one = LLVMConstInt(this->getType(p.fType), 1, false);
    switch (p.fOperator) {
        case Token::PLUSPLUS:
            switch (type_kind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    mod = LLVMBuildAdd(builder, result, one, "++");
                    break;
                case kFloat_TypeKind:
                    mod = LLVMBuildFAdd(builder, result, one, "++");
                    break;
                default:
                    ABORT("unsupported type_kind");
            }
            break;
        case Token::MINUSMINUS:
            switch (type_kind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    mod = LLVMBuildSub(builder, result, one, "--");
                    break;
                case kFloat_TypeKind:
                    mod = LLVMBuildFSub(builder, result, one, "--");
                    break;
                default:
                    ABORT("unsupported type_kind");
            }
            break;
        default:
            ABORT("unsupported postfix op");
    }
    LLVMBuildStore(builder, mod, lvalue);
    return result;
}

LLVMValueRef JIT::compilePrefix(LLVMBuilderRef builder, const PrefixExpression& p) {
    LLVMValueRef one = LLVMConstInt(this->getType(p.fType), 1, false);
    if (Token::LOGICALNOT == p.fOperator) {
        LLVMValueRef base = this->compileExpression(builder, *p.fOperand);
        return LLVMBuildXor(builder, base, one, "!");
    }
    LLVMValueRef lvalue = this->getLValue(builder, *p.fOperand);
    LLVMValueRef raw = LLVMBuildLoad(builder, lvalue, "prefix");
    LLVMValueRef result;
    switch (p.fOperator) {
        case Token::PLUSPLUS:
            switch (type_kind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    result = LLVMBuildAdd(builder, raw, one, "++");
                    break;
                case kFloat_TypeKind:
                    result = LLVMBuildFAdd(builder, raw, one, "++");
                    break;
                default:
                    ABORT("unsupported type_kind");
            }
            break;
        case Token::MINUSMINUS:
            switch (type_kind(p.fType)) {
                case kInt_TypeKind: // fall through
                case kUInt_TypeKind:
                    result = LLVMBuildSub(builder, raw, one, "--");
                    break;
                case kFloat_TypeKind:
                    result = LLVMBuildFSub(builder, raw, one, "--");
                    break;
                default:
                    ABORT("unsupported type_kind");
            }
            break;
        default:
            ABORT("unsupported prefix op");
    }
    LLVMBuildStore(builder, result, lvalue);
    return result;
}

LLVMValueRef JIT::compileVariableReference(LLVMBuilderRef builder, const VariableReference& v) {
    const Variable& var = v.fVariable;
    if (Variable::kParameter_Storage == var.fStorage &&
        !(var.fModifiers.fFlags & Modifiers::kOut_Flag)) {
        return fVariables[&var];
    }
    return LLVMBuildLoad(builder, fVariables[&var], String(var.fName).c_str());
}

void JIT::appendStage(LLVMBuilderRef builder, const AppendStage& a) {
    ASSERT(a.fArguments.size() >= 1);
    ASSERT(a.fArguments[0]->fType == *fCompiler.context().fSkRasterPipeline_Type);
    LLVMValueRef pipeline = this->compileExpression(builder, *a.fArguments[0]);
    LLVMValueRef stage = LLVMConstInt(fInt32Type, a.fStage, 0);
    switch (a.fStage) {
        case SkRasterPipeline::callback: {
            ASSERT(a.fArguments.size() == 2);
            ASSERT(a.fArguments[1]->fKind == Expression::kFunctionReference_Kind);
            const FunctionDeclaration& functionDecl =
                                             *((FunctionReference&) *a.fArguments[1]).fFunctions[0];
            bool found = false;
            for (const auto& pe : fProgram->fElements) {
                if (ProgramElement::kFunction_Kind == pe->fKind) {
                    const FunctionDefinition& def = (const FunctionDefinition&) *pe;
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
            ASSERT(found);
            break;
        }
        default: {
            LLVMValueRef ctx;
            if (a.fArguments.size() == 2) {
                ctx = this->compileExpression(builder, *a.fArguments[1]);
                ctx = LLVMBuildBitCast(builder, ctx, fInt8PtrType, "context cast");
            } else {
                ASSERT(a.fArguments.size() == 1);
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
    if (Type::kScalar_Kind == c.fType.kind()) {
        ASSERT(c.fArguments.size() == 1);
        TypeKind from = type_kind(c.fArguments[0]->fType);
        TypeKind to = type_kind(c.fType);
        LLVMValueRef base = this->compileExpression(builder, *c.fArguments[0]);
        if (kFloat_TypeKind == to) {
            if (kInt_TypeKind == from) {
                return LLVMBuildSIToFP(builder, base, this->getType(c.fType), "cast");
            }
            if (kUInt_TypeKind == from) {
                return LLVMBuildUIToFP(builder, base, this->getType(c.fType), "cast");
            }
        }
        if (kInt_TypeKind == to) {
            if (kFloat_TypeKind == from) {
                return LLVMBuildFPToSI(builder, base, this->getType(c.fType), "cast");
            }
            if (kUInt_TypeKind == from) {
                return base;
            }
        }
        if (kUInt_TypeKind == to) {
            if (kFloat_TypeKind == from) {
                return LLVMBuildFPToUI(builder, base, this->getType(c.fType), "cast");
            }
            if (kInt_TypeKind == from) {
                return base;
            }
        }
    }
    ABORT("unsupported constructor");
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
            abort();
        case Expression::kIndex_Kind:
            return this->compileIndex(builder, (IndexExpression&) expr);
        case Expression::kPrefix_Kind:
            return this->compilePrefix(builder, (PrefixExpression&) expr);
        case Expression::kPostfix_Kind:
            return this->compilePostfix(builder, (PostfixExpression&) expr);
        case Expression::kSetting_Kind:
            abort();
        case Expression::kSwizzle_Kind:
            abort();
        case Expression::kVariableReference_Kind:
            return this->compileVariableReference(builder, (VariableReference&) expr);
        case Expression::kTernary_Kind:
            abort();
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
    this->compileStatement(builder, *f.fStatement);
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

void JIT::compileStatement(LLVMBuilderRef builder, const Statement& stmt) {
    switch (stmt.fKind) {
        case Statement::kBlock_Kind:
            this->compileBlock(builder, (Block&) stmt);
            break;
        case Statement::kBreak_Kind:
            abort();
        case Statement::kContinue_Kind:
            abort();
        case Statement::kDiscard_Kind:
            abort();
        case Statement::kDo_Kind:
            abort();
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
            abort();
        case Statement::kSwitch_Kind:
            abort();
        case Statement::kVarDeclarations_Kind:
            this->compileVarDeclarations(builder, (VarDeclarationsStatement&) stmt);
            break;
        case Statement::kWhile_Kind:
            abort();
        default:
            abort();
    }
}

LLVMValueRef JIT::compileStageFunction(const FunctionDefinition& f) {
    LLVMTypeRef returnType = fVoidType;
    LLVMTypeRef parameterTypes[12] = { fSizeTType, LLVMPointerType(fInt8PtrType, 0), fSizeTType,
                                       fSizeTType, fFloat32VectorType, fFloat32VectorType,
                                       fFloat32VectorType, fFloat32VectorType, fFloat32VectorType,
                                       fFloat32VectorType, fFloat32VectorType, fFloat32VectorType };
    LLVMTypeRef stageFuncType = LLVMFunctionType(returnType, parameterTypes, 12, false);
    LLVMValueRef oldFunction = fCurrentFunction;
    fCurrentFunction  = LLVMAddFunction(fModule, (String(f.fDeclaration.fName) + "$stage").c_str(),
                                        stageFuncType);
    std::unique_ptr<LLVMValueRef> params((LLVMValueRef*) malloc(sizeof(LLVMValueRef*) * 12));
    LLVMGetParams(fCurrentFunction, params.get());
    LLVMValueRef programParam = params.get()[1];
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(fContext);
    LLVMBasicBlockRef oldAllocaBlock = fAllocaBlock;
    LLVMBasicBlockRef oldCurrentBlock = fCurrentBlock;
    fAllocaBlock = LLVMAppendBasicBlockInContext(fContext, fCurrentFunction, "alloca");
    this->setBlock(builder, fAllocaBlock);
    LLVMValueRef rVec = LLVMBuildAlloca(builder, fFloat32VectorType, "rVec");
    LLVMBuildStore(builder, params.get()[4], rVec);
    LLVMValueRef gVec = LLVMBuildAlloca(builder, fFloat32VectorType, "gVec");
    LLVMBuildStore(builder, params.get()[5], gVec);
    LLVMValueRef bVec = LLVMBuildAlloca(builder, fFloat32VectorType, "bVec");
    LLVMBuildStore(builder, params.get()[6], bVec);
    LLVMValueRef aVec = LLVMBuildAlloca(builder, fFloat32VectorType, "aVec");
    LLVMBuildStore(builder, params.get()[7], aVec);
    LLVMValueRef rVal = LLVMBuildAlloca(builder, fFloat32Type, "rVal");
    LLVMValueRef gVal = LLVMBuildAlloca(builder, fFloat32Type, "gVal");
    LLVMValueRef bVal = LLVMBuildAlloca(builder, fFloat32Type, "bVal");
    LLVMValueRef aVal = LLVMBuildAlloca(builder, fFloat32Type, "aVal");
    fVariables[f.fDeclaration.fParameters[1]] = LLVMBuildTrunc(builder, params.get()[3], fInt32Type,
                                                               "y->Int32");
    fVariables[f.fDeclaration.fParameters[2]] = rVal;
    fVariables[f.fDeclaration.fParameters[3]] = gVal;
    fVariables[f.fDeclaration.fParameters[4]] = bVal;
    fVariables[f.fDeclaration.fParameters[5]] = aVal;
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
    LLVMValueRef rInitial = LLVMBuildExtractElement(builder, params.get()[4], iload, "rInitial");
    LLVMValueRef gInitial = LLVMBuildExtractElement(builder, params.get()[5], iload, "gInitial");
    LLVMValueRef bInitial = LLVMBuildExtractElement(builder, params.get()[6], iload, "bInitial");
    LLVMValueRef aInitial = LLVMBuildExtractElement(builder, params.get()[7], iload, "aInitial");
    LLVMBuildStore(builder, rInitial, rVal);
    LLVMBuildStore(builder, gInitial, gVal);
    LLVMBuildStore(builder, bInitial, bVal);
    LLVMBuildStore(builder, aInitial, aVal);
    this->compileStatement(builder, *f.fBody);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, rVec, "rVec"),
                                          LLVMBuildLoad(builder, rVal, "rVal"), iload, "rInsert"),
                   rVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, gVec, "gVec"),
                                          LLVMBuildLoad(builder, gVal, "gVal"), iload, "gInsert"),
                   gVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, bVec, "bVec"),
                                          LLVMBuildLoad(builder, bVal, "bVal"), iload, "bInsert"),
                   bVec);
    LLVMBuildStore(builder,
                   LLVMBuildInsertElement(builder, LLVMBuildLoad(builder, aVec, "aVec"),
                                          LLVMBuildLoad(builder, aVal, "aVal"), iload, "aInsert"),
                   aVec);
    LLVMValueRef inc = LLVMBuildAdd(builder, iload, LLVMConstInt(fInt32Type, 1, false), "inc i");
    LLVMBuildStore(builder, inc, ivar);
    LLVMBuildBr(builder, start);
    this->setBlock(builder, loopEnd);
    // increment program pointer, call next
    LLVMValueRef rawNextPtr = LLVMBuildLoad(builder, programParam, "next load");
    LLVMValueRef nextPtr = LLVMBuildBitCast(builder, rawNextPtr, LLVMPointerType(stageFuncType, 0),
                                            "cast next->func");
    LLVMValueRef nextInc = LLVMBuildIntToPtr(builder,
                                             LLVMBuildAdd(builder,
                                                          LLVMBuildPtrToInt(builder,
                                                                            programParam,
                                                                            fInt64Type,
                                                                            "cast 1"),
                                                          LLVMConstInt(fInt64Type, PTR_SIZE, false),
                                                          "add"),
                                            LLVMPointerType(fInt8PtrType, 0), "cast 2");
    LLVMValueRef args[12] = {
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
    LLVMBuildCall(builder, nextPtr, args, 12, "");
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
    LLVMValueRef result = fCurrentFunction;
    fCurrentFunction = oldFunction;
    return result;
}

bool JIT::hasStageSignature(const FunctionDeclaration& f) {
    return f.fReturnType == *fProgram->fContext->fVoid_Type &&
           f.fParameters.size() == 6 &&
           f.fParameters[0]->fType == *fProgram->fContext->fInt_Type &&
           f.fParameters[0]->fModifiers.fFlags == 0 &&
           f.fParameters[1]->fType == *fProgram->fContext->fInt_Type &&
           f.fParameters[1]->fModifiers.fFlags == 0 &&
           f.fParameters[2]->fType == *fProgram->fContext->fFloat_Type &&
           f.fParameters[2]->fModifiers.fFlags == (Modifiers::kIn_Flag | Modifiers::kOut_Flag) &&
           f.fParameters[3]->fType == *fProgram->fContext->fFloat_Type &&
           f.fParameters[3]->fModifiers.fFlags == (Modifiers::kIn_Flag | Modifiers::kOut_Flag) &&
           f.fParameters[4]->fType == *fProgram->fContext->fFloat_Type &&
           f.fParameters[4]->fModifiers.fFlags == (Modifiers::kIn_Flag | Modifiers::kOut_Flag) &&
           f.fParameters[5]->fType == *fProgram->fContext->fFloat_Type &&
           f.fParameters[5]->fModifiers.fFlags == (Modifiers::kIn_Flag | Modifiers::kOut_Flag);
}

LLVMValueRef JIT::compileFunction(const FunctionDefinition& f) {
    if (this->hasStageSignature(f.fDeclaration)) {
        this->compileStageFunction(f);
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

    std::unique_ptr<LLVMValueRef> params((LLVMValueRef*) malloc(sizeof(LLVMValueRef*) *
                                                                parameterTypes.size()));
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
        LLVMBuildRetVoid(builder);
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
    fModule = LLVMModuleCreateWithNameInContext("skslmodule", fContext);
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

    for (const auto& e : fProgram->fElements) {
        ASSERT(e->fKind == ProgramElement::kFunction_Kind);
        LLVMValueRef fn = this->compileFunction((FunctionDefinition&) *e);
        fFunctions[&((FunctionDefinition&) *e).fDeclaration] = fn;
    }
}

void JIT::compile() {
    fContext = LLVMContextCreate();
    fVoidType = LLVMVoidTypeInContext(fContext);
    fInt1Type = LLVMInt1TypeInContext(fContext);
    fInt8Type = LLVMInt8TypeInContext(fContext);
    fInt8PtrType = LLVMPointerType(fInt8Type, 0);
    fInt32Type = LLVMInt32TypeInContext(fContext);
    fInt64Type = LLVMInt64TypeInContext(fContext);
    fSizeTType = LLVMInt64TypeInContext(fContext);
    fInt32VectorType = LLVMVectorType(fInt32Type, fVectorCount);
    fFloat32Type = LLVMFloatTypeInContext(fContext);
    fFloat32VectorType = LLVMVectorType(fFloat32Type, fVectorCount);
    this->createModule();
    this->optimize();
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
}

void* JIT::getSymbol(const char* name) {
    LLVMOrcTargetAddress result;
    if (LLVMOrcGetSymbolAddress(fJITStack, &result, name)) {
        ABORT("Unable to find symbol in module");
    }
    return (void*) result;
}

void* JIT::getJumperStage(const char* name) {
    return getSymbol((String(name) + "$stage").c_str());
}

} // namespace

#endif // SKIA_AVAILABLE

#endif // SKSL_STANDALONE
