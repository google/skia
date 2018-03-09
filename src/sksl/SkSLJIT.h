/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_JIT
#define SKSL_JIT

#include "ir/SkSLAppendStage.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"

#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/OrcBindings.h"
#include "llvm-c/Support.h"
#include "llvm-c/Target.h"
#include "llvm-c/Transforms/PassManagerBuilder.h"
#include "llvm-c/Types.h"
#include <stack>

class SkRasterPipeline;

namespace SkSL {

class JIT {
    typedef int StackIndex;

public:
    enum TypeKind {
        kFloat_TypeKind,
        kInt_TypeKind,
        kUInt_TypeKind,
        kBool_TypeKind
    };

    JIT(Compiler* compiler, std::unique_ptr<Program> program);

    ~JIT();

    void* getSymbol(const char* name);

    void* getJumperStage(const char* name);

private:
    class LValue {
    public:
        virtual ~LValue() {}

        virtual LLVMValueRef load(LLVMBuilderRef builder) = 0;

        virtual void store(LLVMBuilderRef builder, LLVMValueRef value) = 0;
    };

    void compile();

    void setBlock(LLVMBuilderRef builder, LLVMBasicBlockRef block);

    LLVMTypeRef getType(const Type& type);

    std::unique_ptr<LValue> getLValue(LLVMBuilderRef builder, const Expression& expr);

    void vectorize(LLVMBuilderRef builder, LLVMValueRef* value, int columns);

    void vectorize(LLVMBuilderRef builder, const BinaryExpression& b, LLVMValueRef* left,
                   LLVMValueRef* right);

    LLVMValueRef compileBinary(LLVMBuilderRef builder, const BinaryExpression& b);

    LLVMValueRef compileConstructor(LLVMBuilderRef builder, const Constructor& c);

    LLVMValueRef compileIndex(LLVMBuilderRef builder, const IndexExpression& v);

    LLVMValueRef compilePostfix(LLVMBuilderRef builder, const PostfixExpression& p);

    LLVMValueRef compilePrefix(LLVMBuilderRef builder, const PrefixExpression& p);

    LLVMValueRef compileSwizzle(LLVMBuilderRef builder, const Swizzle& s);

    LLVMValueRef compileVariableReference(LLVMBuilderRef builder, const VariableReference& v);

    LLVMValueRef compileExpression(LLVMBuilderRef builder, const Expression& expr);

    void appendStage(LLVMBuilderRef builder, const AppendStage& a);

    void compileBlock(LLVMBuilderRef builder, const Block& block);

    void compileFor(LLVMBuilderRef builder, const ForStatement& f);

    void compileIf(LLVMBuilderRef builder, const IfStatement& i);

    void compileVarDeclarations(LLVMBuilderRef builder, const VarDeclarationsStatement& decls);

    void compileStatement(LLVMBuilderRef builder, const Statement& stmt);

    LLVMValueRef compileStageFunction(const FunctionDefinition& f);

    LLVMValueRef compileFunction(const FunctionDefinition& f);

    bool hasStageSignature(const FunctionDeclaration& f);

    void createModule();

    void optimize();

    static uint64_t resolveSymbol(const char* name, JIT* jit);

    const char* fCPU;
    int fVectorCount;
    Compiler& fCompiler;
    std::unique_ptr<Program> fProgram;
    LLVMContextRef fContext;
    LLVMModuleRef fModule;
    LLVMSharedModuleRef fSharedModule;
    LLVMOrcJITStackRef fJITStack;
    LLVMValueRef fCurrentFunction;
    LLVMBasicBlockRef fAllocaBlock;
    LLVMBasicBlockRef fCurrentBlock;
    LLVMTypeRef fVoidType;
    LLVMTypeRef fInt1Type;
    LLVMTypeRef fInt8Type;
    LLVMTypeRef fInt8PtrType;
    LLVMTypeRef fInt32Type;
    LLVMTypeRef fInt32VectorType;
    LLVMTypeRef fInt32Vector2Type;
    LLVMTypeRef fInt32Vector3Type;
    LLVMTypeRef fInt32Vector4Type;
    LLVMTypeRef fInt64Type;
    LLVMTypeRef fSizeTType;
    LLVMTypeRef fFloat32Type;
    LLVMTypeRef fFloat32VectorType;
    LLVMTypeRef fFloat32Vector2Type;
    LLVMTypeRef fFloat32Vector3Type;
    LLVMTypeRef fFloat32Vector4Type;
    std::map<const FunctionDeclaration*, LLVMValueRef> fFunctions;
    std::map<const Variable*, LLVMValueRef> fVariables;

    LLVMValueRef fAppendFunc;
    LLVMValueRef fAppendCallbackFunc;
    LLVMValueRef fDebugFunc;
};

} // namespace

#endif // SKSL_JIT
