/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_COMPILER
#define SKSL_COMPILER

#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLCFGGenerator.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrShaderVar.h"
#endif

#define SK_FRAGCOLOR_BUILTIN           10001
#define SK_IN_BUILTIN                  10002
#define SK_INCOLOR_BUILTIN             10003
#define SK_OUTCOLOR_BUILTIN            10004
#define SK_TRANSFORMEDCOORDS2D_BUILTIN 10005
#define SK_TEXTURESAMPLERS_BUILTIN     10006
#define SK_OUT_BUILTIN                 10007
#define SK_LASTFRAGCOLOR_BUILTIN       10008
#define SK_MAIN_X_BUILTIN              10009
#define SK_MAIN_Y_BUILTIN              10010
#define SK_WIDTH_BUILTIN               10011
#define SK_HEIGHT_BUILTIN              10012
#define SK_FRAGCOORD_BUILTIN              15
#define SK_CLOCKWISE_BUILTIN              17
#define SK_SAMPLEMASK_BUILTIN             20
#define SK_VERTEXID_BUILTIN               42
#define SK_INSTANCEID_BUILTIN             43
#define SK_CLIPDISTANCE_BUILTIN            3
#define SK_INVOCATIONID_BUILTIN            8
#define SK_POSITION_BUILTIN                0

namespace SkSL {

class ByteCode;
class ExternalValue;
class IRGenerator;

/**
 * Main compiler entry point. This is a traditional compiler design which first parses the .sksl
 * file into an abstract syntax tree (a tree of ASTNodes), then performs semantic analysis to
 * produce a Program (a tree of IRNodes), then feeds the Program into a CodeGenerator to produce
 * compiled output.
 *
 * See the README for information about SkSL.
 */
class SK_API Compiler : public ErrorReporter {
public:
    static constexpr const char* RTADJUST_NAME  = "sk_RTAdjust";
    static constexpr const char* PERVERTEX_NAME = "sk_PerVertex";

    enum Flags {
        kNone_Flags = 0,
        // permits static if/switch statements to be used with non-constant tests. This is used when
        // producing H and CPP code; the static tests don't have to have constant values *yet*, but
        // the generated code will contain a static test which then does have to be a constant.
        kPermitInvalidStaticTests_Flag = 1,
    };

    struct FormatArg {
        enum class Kind {
            kInput,
            kOutput,
            kCoordX,
            kCoordY,
            kUniform,
            kChildProcessor,
            kFunctionName
        };

        FormatArg(Kind kind)
                : fKind(kind) {}

        FormatArg(Kind kind, int index)
                : fKind(kind)
                , fIndex(index) {}

        Kind fKind;

        int fIndex;
    };

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    /**
     * Represents the arguments to GrGLSLShaderBuilder::emitFunction.
     */
    struct GLSLFunction {
        GrSLType fReturnType;
        SkString fName;
        std::vector<GrShaderVar> fParameters;
        SkString fBody;
        std::vector<Compiler::FormatArg> fFormatArgs;
    };
#endif

    Compiler(Flags flags = kNone_Flags);

    ~Compiler() override;

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;

    /**
     * Registers an ExternalValue as a top-level symbol which is visible in the global namespace.
     */
    void registerExternalValue(ExternalValue* value);

    std::unique_ptr<Program> convertProgram(Program::Kind kind, String text,
                                            const Program::Settings& settings);

    bool optimize(Program& program);

    std::unique_ptr<Program> specialize(Program& program,
                    const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs);

    bool toSPIRV(Program& program, OutputStream& out);

    bool toSPIRV(Program& program, String* out);

    bool toGLSL(Program& program, OutputStream& out);

    bool toGLSL(Program& program, String* out);

    bool toMetal(Program& program, OutputStream& out);

    bool toMetal(Program& program, String* out);

    bool toCPP(Program& program, String name, OutputStream& out);

    bool toH(Program& program, String name, OutputStream& out);

    std::unique_ptr<ByteCode> toByteCode(Program& program);

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    bool toPipelineStage(const Program& program, String* out,
                         std::vector<FormatArg>* outFormatArgs,
                         std::vector<GLSLFunction>* outFunctions);
#endif

    /**
     * Takes ownership of the given symbol. It will be destroyed when the compiler is destroyed.
     */
    Symbol* takeOwnership(std::unique_ptr<Symbol> symbol);

    void error(int offset, String msg) override;

    String errorText();

    void writeErrorCount();

    int errorCount() override {
        return fErrorCount;
    }

    Context& context() {
        return *fContext;
    }

    static const char* OperatorName(Token::Kind token);

    static bool IsAssignment(Token::Kind token);

private:
    void processIncludeFile(Program::Kind kind, const char* src, size_t length,
                            std::shared_ptr<SymbolTable> base,
                            std::vector<std::unique_ptr<ProgramElement>>* outElements,
                            std::shared_ptr<SymbolTable>* outSymbolTable);

    void addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                       DefinitionMap* definitions);

    void addDefinitions(const BasicBlock::Node& node, DefinitionMap* definitions);

    void scanCFG(CFG* cfg, BlockId block, std::set<BlockId>* workList);

    void computeDataFlow(CFG* cfg);

    /**
     * Simplifies the expression pointed to by iter (in both the IR and CFG structures), if
     * possible.
     */
    void simplifyExpression(DefinitionMap& definitions,
                            BasicBlock& b,
                            std::vector<BasicBlock::Node>::iterator* iter,
                            std::unordered_set<const Variable*>* undefinedVariables,
                            bool* outUpdated,
                            bool* outNeedsRescan);

    /**
     * Simplifies the statement pointed to by iter (in both the IR and CFG structures), if
     * possible.
     */
    void simplifyStatement(DefinitionMap& definitions,
                           BasicBlock& b,
                           std::vector<BasicBlock::Node>::iterator* iter,
                           std::unordered_set<const Variable*>* undefinedVariables,
                           bool* outUpdated,
                           bool* outNeedsRescan);

    void scanCFG(FunctionDefinition& f);

    Position position(int offset);

    std::map<StringFragment, std::pair<std::unique_ptr<ProgramElement>, bool>> fGPUIntrinsics;
    std::map<StringFragment, std::pair<std::unique_ptr<ProgramElement>, bool>> fInterpreterIntrinsics;
    std::unique_ptr<ASTFile> fGpuIncludeSource;
    std::shared_ptr<SymbolTable> fGpuSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fVertexInclude;
    std::shared_ptr<SymbolTable> fVertexSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fFragmentInclude;
    std::shared_ptr<SymbolTable> fFragmentSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fGeometryInclude;
    std::shared_ptr<SymbolTable> fGeometrySymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fPipelineInclude;
    std::shared_ptr<SymbolTable> fPipelineSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fInterpreterInclude;
    std::shared_ptr<SymbolTable> fInterpreterSymbolTable;

    std::shared_ptr<SymbolTable> fTypes;
    IRGenerator* fIRGenerator;
    int fFlags;

    const String* fSource;
    std::shared_ptr<Context> fContext;
    int fErrorCount;
    String fErrorText;
};

} // namespace

#endif
