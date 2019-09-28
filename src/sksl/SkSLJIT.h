/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_JIT
#define SKSL_JIT

#ifdef SK_LLVM_AVAILABLE

#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

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

struct AppendStage;

/**
 * A just-in-time compiler for SkSL code which uses an LLVM backend. Only available when the
 * skia_llvm_path gn arg is set.
 *
 * Example of using SkSLJIT to set up an SkJumper pipeline stage:
 *
 * #ifdef SK_LLVM_AVAILABLE
 *   SkSL::Compiler compiler;
 *   SkSL::Program::Settings settings;
 *   std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
         SkSL::Program::kPipelineStage_Kind,
 *       "void swap(int x, int y, inout float4 color) {"
 *       "    color.rb = color.br;"
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
         * have the correct signature to function as an SkJumper stage (meaning it will actually
         * have a different signature at runtime, accepting vector parameters and operating on
         * multiple pixels simultaneously as is normal for SkJumper stages).
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
    static constexpr int CHANNELS = 4;

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

    LLVMValueRef compileTernary(LLVMBuilderRef builder, const TernaryExpression& t);

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

    // The "Vector" variants of functions attempt to compile a given expression or statement as part
    // of a vectorized SkJumper stage function - that is, with r, g, b, and a each being vectors of
    // fVectorCount floats. So a statement like "color.r = 0;" looks like it modifies a single
    // channel of a single pixel, but the compiled code will actually modify the red channel of
    // fVectorCount pixels at once.
    //
    // As not everything can be vectorized, these calls return a bool to indicate whether they were
    // successful. If anything anywhere in the function cannot be vectorized, the JIT will fall back
    // to looping over the pixels instead.
    //
    // Since we process multiple pixels at once, and each pixel consists of multiple color channels,
    // expressions may effectively result in a vector-of-vectors. We produce zero to four outputs
    // when compiling expression, each of which is a vector, so that e.g. float2(1, 0) actually
    // produces two vectors, one containing all 1s, the other all 0s. The out parameter always
    // allows for 4 channels, but the functions produce 0 to 4 channels depending on the type they
    // are operating on. Thus evaluating "color.rgb" actually fills in out[0] through out[2],
    // leaving out[3] uninitialized.
    // As the number of outputs can be inferred from the type of the expression, it is not
    // explicitly signalled anywhere.
    bool compileVectorBinary(LLVMBuilderRef builder, const BinaryExpression& b,
                             LLVMValueRef out[CHANNELS]);

    bool compileVectorConstructor(LLVMBuilderRef builder, const Constructor& c,
                                  LLVMValueRef out[CHANNELS]);

    bool compileVectorFloatLiteral(LLVMBuilderRef builder, const FloatLiteral& f,
                                   LLVMValueRef out[CHANNELS]);

    bool compileVectorSwizzle(LLVMBuilderRef builder, const Swizzle& s,
                              LLVMValueRef out[CHANNELS]);

    bool compileVectorVariableReference(LLVMBuilderRef builder, const VariableReference& v,
                                        LLVMValueRef out[CHANNELS]);

    bool compileVectorExpression(LLVMBuilderRef builder, const Expression& expr,
                                 LLVMValueRef out[CHANNELS]);

    bool getVectorLValue(LLVMBuilderRef builder, const Expression& e, LLVMValueRef out[CHANNELS]);

    /**
     * Evaluates the left and right operands of a binary operation, promoting one of them to a
     * vector if necessary to make the types match.
     */
    bool getVectorBinaryOperands(LLVMBuilderRef builder, const Expression& left,
                                 LLVMValueRef outLeft[CHANNELS], const Expression& right,
                                 LLVMValueRef outRight[CHANNELS]);

    bool compileVectorStatement(LLVMBuilderRef builder, const Statement& stmt);

    /**
     * Returns true if this function has the signature void(int, int, inout float4) and thus can be
     * used as an SkJumper stage.
     */
    bool hasStageSignature(const FunctionDeclaration& f);

    /**
     * Attempts to compile a vectorized stage function, returning true on success. A stage function
     * of e.g. "color.r = 0;" will produce code which sets the entire red vector to zeros in a
     * single instruction, thus calculating several pixels at once.
     */
    bool compileStageFunctionVector(const FunctionDefinition& f, LLVMValueRef newFunc);

    /**
     * Fallback function which loops over the pixels, for when vectorization fails. A stage function
     * of e.g. "color.r = 0;" will produce a loop which iterates over the entries in the red vector,
     * setting each one to zero individually.
     */
    void compileStageFunctionLoop(const FunctionDefinition& f, LLVMValueRef newFunc);

    /**
     * Called when compiling a function which has the signature of an SkJumper stage. Produces a
     * version of the function which can be plugged into SkJumper (thus having a signature which
     * accepts four vectors, one for each color channel, containing the color data of multiple
     * pixels at once). To go from SkSL code which operates on a single pixel at a time to CPU code
     * which operates on multiple pixels at once, the code is either vectorized using
     * compileStageFunctionVector or wrapped in a loop using compileStageFunctionLoop.
     */
    LLVMValueRef compileStageFunction(const FunctionDefinition& f);

    /**
     * Compiles an SkSL function to an LLVM function. If the function has the signature of an
     * SkJumper stage, it will *also* be compiled by compileStageFunction, resulting in both a stage
     * and non-stage version of the function.
     */
    LLVMValueRef compileFunction(const FunctionDefinition& f);

    void createModule();

    void optimize();

    bool isColorRef(const Expression& expr);

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
    LLVMTypeRef fInt1VectorType;
    LLVMTypeRef fInt1Vector2Type;
    LLVMTypeRef fInt1Vector3Type;
    LLVMTypeRef fInt1Vector4Type;
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
    // Our SkSL stage functions have a single float4 for color, but the actual SkJumper stage
    // function has four separate vectors, one for each channel. These four values are references to
    // the red, green, blue, and alpha vectors respectively.
    LLVMValueRef fChannels[CHANNELS];
    // when processing a stage function, this points to the SkSL color parameter (an inout float4)
    const Variable* fColorParam;
    std::unordered_map<const FunctionDeclaration*, LLVMValueRef> fFunctions;
    std::unordered_map<const Variable*, LLVMValueRef> fVariables;
    // LLVM function parameters are read-only, so when modifying function parameters we need to
    // first promote them to variables. This keeps track of which parameters have been promoted.
    std::set<const Variable*> fPromotedParameters;
    std::vector<LLVMBasicBlockRef> fBreakTarget;
    std::vector<LLVMBasicBlockRef> fContinueTarget;

    LLVMValueRef fFoldAnd2Func;
    LLVMValueRef fFoldOr2Func;
    LLVMValueRef fFoldAnd3Func;
    LLVMValueRef fFoldOr3Func;
    LLVMValueRef fFoldAnd4Func;
    LLVMValueRef fFoldOr4Func;
    LLVMValueRef fAppendFunc;
    LLVMValueRef fAppendCallbackFunc;
    LLVMValueRef fDebugFunc;
};

} // namespace

#endif // SK_LLVM_AVAILABLE

#endif // SKSL_JIT
