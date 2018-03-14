/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_JIT
#define SKSL_JIT

#ifdef SK_LLVM_AVAILABLE

#include "ir/SkSLAppendStage.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLBreakStatement.h"
#include "ir/SkSLContinueStatement.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLDoStatement.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLReturnStatement.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"
#include "ir/SkSLWhileStatement.h"

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

/**
 * A just-in-time compiler for SkSL code which uses an LLVM backend. Only available when the
 * skia_llvm_path gn arg is set.
 *
 * Example of using SkSLJIT to set up an SkJumper pipeline stage:
 *
 * #ifdef SK_LLVM_AVAILABLE
 *   SkSL::Compiler compiler;
 *   SkSL::Program::Settings settings;
 *   std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kCPU_Kind,
 *       "void swap(int x, int y, inout float4 color) {"
 *       "    color.rgb = color.bgr;"
 *       "}",
 *       settings);
 *   if (!program) {
 *       printf("%s\n", compiler.errorText().c_str());
 *       abort();
 *   }
 *   SkSL::JIT& jit = *scratch->make<SkSL::JIT>(&compiler);
 *   std::unique_ptr<SkSL::JIT::Module> module = jit.compile(std::move(program));
 *   void* func = module->getJumperStage("swap");
 *   p->append(func, nullptr);
 * #endif
 */
class JIT {
    typedef int StackIndex;

public:
    class Module {
    public:
        /**
         * Returns the address of a symbol in the module.
         */
        void* getSymbol(const char* name);

        /**
         * Returns the address of a function as an SkJumper pipeline stage. The function must have
         * the signature void <name>(int x, int y, inout float4 color). The returned function will
         * have the correct signature to function as an SkJumper stage.
         */
        void* getJumperStage(const char* name);

        ~Module() {
            LLVMOrcDisposeSharedModuleRef(fSharedModule);
        }

    private:
        Module(std::unique_ptr<Program> program,
               LLVMSharedModuleRef sharedModule,
               LLVMOrcJITStackRef jitStack)
        : fProgram(std::move(program))
        , fSharedModule(sharedModule)
        , fJITStack(jitStack) {}

        std::unique_ptr<Program> fProgram;
        LLVMSharedModuleRef fSharedModule;
        LLVMOrcJITStackRef fJITStack;

        friend class JIT;
    };

    JIT(Compiler* compiler);

    ~JIT();

    /**
     * Just-in-time compiles an SkSL program and returns the resulting Module. The JIT must not be
     * destroyed before all of its Modules are destroyed.
     */
    std::unique_ptr<Module> compile(std::unique_ptr<Program> program);

private:
    enum TypeKind {
        kFloat_TypeKind,
        kInt_TypeKind,
        kUInt_TypeKind,
        kBool_TypeKind
    };

    class LValue {
    public:
        virtual ~LValue() {}

        virtual LLVMValueRef load(LLVMBuilderRef builder) = 0;

        virtual void store(LLVMBuilderRef builder, LLVMValueRef value) = 0;
    };

    void addBuiltinFunction(const char* ourName, const char* realName, LLVMTypeRef returnType,
                            std::vector<LLVMTypeRef> parameters);

    void loadBuiltinFunctions();

    void setBlock(LLVMBuilderRef builder, LLVMBasicBlockRef block);

    LLVMTypeRef getType(const Type& type);

    TypeKind typeKind(const Type& type);

    std::unique_ptr<LValue> getLValue(LLVMBuilderRef builder, const Expression& expr);

    void vectorize(LLVMBuilderRef builder, LLVMValueRef* value, int columns);

    void vectorize(LLVMBuilderRef builder, const BinaryExpression& b, LLVMValueRef* left,
                   LLVMValueRef* right);

    LLVMValueRef compileBinary(LLVMBuilderRef builder, const BinaryExpression& b);

    LLVMValueRef compileConstructor(LLVMBuilderRef builder, const Constructor& c);

    LLVMValueRef compileFunctionCall(LLVMBuilderRef builder, const FunctionCall& fc);

    LLVMValueRef compileIndex(LLVMBuilderRef builder, const IndexExpression& v);

    LLVMValueRef compilePostfix(LLVMBuilderRef builder, const PostfixExpression& p);

    LLVMValueRef compilePrefix(LLVMBuilderRef builder, const PrefixExpression& p);

    LLVMValueRef compileSwizzle(LLVMBuilderRef builder, const Swizzle& s);

    LLVMValueRef compileVariableReference(LLVMBuilderRef builder, const VariableReference& v);

    LLVMValueRef compileExpression(LLVMBuilderRef builder, const Expression& expr);

    void appendStage(LLVMBuilderRef builder, const AppendStage& a);

    void compileBlock(LLVMBuilderRef builder, const Block& block);

    void compileBreak(LLVMBuilderRef builder, const BreakStatement& b);

    void compileContinue(LLVMBuilderRef builder, const ContinueStatement& c);

    void compileDo(LLVMBuilderRef builder, const DoStatement& d);

    void compileFor(LLVMBuilderRef builder, const ForStatement& f);

    void compileIf(LLVMBuilderRef builder, const IfStatement& i);

    void compileReturn(LLVMBuilderRef builder, const ReturnStatement& r);

    void compileVarDeclarations(LLVMBuilderRef builder, const VarDeclarationsStatement& decls);

    void compileWhile(LLVMBuilderRef builder, const WhileStatement& w);

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
    std::stack<LLVMBasicBlockRef> fBreakTarget;
    std::stack<LLVMBasicBlockRef> fContinueTarget;

    LLVMValueRef fAppendFunc;
    LLVMValueRef fAppendCallbackFunc;
    LLVMValueRef fDebugFunc;
};

} // namespace

#endif // SK_LLVM_AVAILABLE

#endif // SKSL_JIT
