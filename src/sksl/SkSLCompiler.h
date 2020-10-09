/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_COMPILER
#define SKSL_COMPILER

#include <set>
#include <unordered_set>
#include <vector>
#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLCFGGenerator.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLInliner.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrShaderVar.h"
#endif

#define SK_FRAGCOLOR_BUILTIN           10001
#define SK_IN_BUILTIN                  10002
#define SK_OUTCOLOR_BUILTIN            10004
#define SK_OUT_BUILTIN                 10007
#define SK_LASTFRAGCOLOR_BUILTIN       10008
#define SK_MAIN_COORDS_BUILTIN         10009
#define SK_WIDTH_BUILTIN               10011
#define SK_HEIGHT_BUILTIN              10012
#define SK_FRAGCOORD_BUILTIN              15
#define SK_CLOCKWISE_BUILTIN              17
#define SK_SAMPLEMASK_BUILTIN             20
#define SK_VERTEXID_BUILTIN               42
#define SK_INSTANCEID_BUILTIN             43
#define SK_INVOCATIONID_BUILTIN            8
#define SK_POSITION_BUILTIN                0

class SkBitSet;

namespace SkSL {

class ByteCode;
class ExternalValue;
class IRGenerator;
class IRIntrinsicMap;
struct PipelineStageArgs;

struct LoadedModule {
    std::shared_ptr<SymbolTable>                 fSymbols;
    std::vector<std::unique_ptr<ProgramElement>> fElements;
};

struct ParsedModule {
    std::shared_ptr<SymbolTable>    fSymbols;
    std::shared_ptr<IRIntrinsicMap> fIntrinsics;
};

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

    // An invalid (otherwise unused) character to mark where FormatArgs are inserted
    static constexpr       char  kFormatArgPlaceholder    = '\001';
    static constexpr const char* kFormatArgPlaceholderStr = "\001";

    struct FormatArg {
        enum class Kind {
            kOutput,
            kCoords,
            kUniform,
            kChildProcessor,
            kChildProcessorWithMatrix,
            kFunctionName
        };

        FormatArg(Kind kind)
                : fKind(kind) {}

        FormatArg(Kind kind, int index)
                : fKind(kind)
                , fIndex(index) {}

        Kind fKind;
        int fIndex;
        String fCoords;
    };

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    /**
     * Represents the arguments to GrGLSLShaderBuilder::emitFunction.
     */
    struct GLSLFunction {
        GrSLType fReturnType;
        SkString fName;
        std::vector<GrShaderVar> fParameters;
        String fBody;
        std::vector<Compiler::FormatArg> fFormatArgs;
    };
#endif

    Compiler(Flags flags = kNone_Flags);

    ~Compiler() override;

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;

    /**
     * If externalValues is supplied, those values are registered in the symbol table of the
     * Program, but ownership is *not* transferred. It is up to the caller to keep them alive.
     */
    std::unique_ptr<Program> convertProgram(
            Program::Kind kind,
            String text,
            const Program::Settings& settings,
            const std::vector<std::unique_ptr<ExternalValue>>* externalValues = nullptr);

    bool toSPIRV(Program& program, OutputStream& out);

    bool toSPIRV(Program& program, String* out);

    bool toGLSL(Program& program, OutputStream& out);

    bool toGLSL(Program& program, String* out);

    bool toHLSL(Program& program, String* out);

    bool toMetal(Program& program, OutputStream& out);

    bool toMetal(Program& program, String* out);

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS
    bool toCPP(Program& program, String name, OutputStream& out);

    bool toH(Program& program, String name, OutputStream& out);
#endif

    std::unique_ptr<ByteCode> toByteCode(Program& program);

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    bool toPipelineStage(Program& program, PipelineStageArgs* outArgs);
#endif

    void error(int offset, String msg) override;

    String errorText();

    void writeErrorCount();

    int errorCount() override {
        return fErrorCount;
    }

    Context& context() {
        return *fContext;
    }

    static const char* OperatorName(Token::Kind op);

    // Returns true if op is '=' or any compound assignment operator ('+=', '-=', etc.)
    static bool IsAssignment(Token::Kind op);

    // Given a compound assignment operator, returns the non-assignment version of the operator
    // (e.g. '+=' becomes '+')
    static Token::Kind RemoveAssignment(Token::Kind op);

    // When  SKSL_STANDALONE, fPath is used. (fData, fSize) will be (nullptr, 0)
    // When !SKSL_STANDALONE, fData and fSize are used. fPath will be nullptr.
    struct ModuleData {
        const char*    fPath;

        const uint8_t* fData;
        size_t         fSize;
    };

    static ModuleData MakeModulePath(const char* path) {
        return ModuleData{path, /*fData=*/nullptr, /*fSize=*/0};
    }
    static ModuleData MakeModuleData(const uint8_t* data, size_t size) {
        return ModuleData{/*fPath=*/nullptr, data, size};
    }

    LoadedModule loadModule(Program::Kind kind, ModuleData data, std::shared_ptr<SymbolTable> base);
    ParsedModule parseModule(Program::Kind kind, ModuleData data, const ParsedModule& base);

private:
    const ParsedModule& loadFPModule();
    const ParsedModule& loadGeometryModule();
    const ParsedModule& loadInterpreterModule();
    const ParsedModule& loadPipelineModule();

    const ParsedModule& moduleForProgramKind(Program::Kind kind);

    void addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                       DefinitionMap* definitions);
    void addDefinitions(const BasicBlock::Node& node, DefinitionMap* definitions);

    void scanCFG(CFG* cfg, BlockId block, SkBitSet* processedSet);
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

    /**
     * Optimizes a function based on control flow analysis. Returns true if changes were made.
     */
    bool scanCFG(FunctionDefinition& f);

    /**
     * Optimize every function in the program.
     */
    bool optimize(Program& program);

    Position position(int offset);

    std::shared_ptr<SymbolTable> fRootSymbolTable;

    ParsedModule fRootModule;
    ParsedModule fGPUModule;
    ParsedModule fInterpreterModule;
    ParsedModule fVertexModule;
    ParsedModule fFragmentModule;
    ParsedModule fGeometryModule;
    ParsedModule fPipelineModule;
    ParsedModule fFPModule;

    // holds ModifiersPools belonging to the core includes for lifetime purposes
    std::vector<std::unique_ptr<ModifiersPool>> fModifiers;

    Inliner fInliner;
    std::unique_ptr<IRGenerator> fIRGenerator;
    int fFlags;

    const String* fSource;
    std::shared_ptr<Context> fContext;
    int fErrorCount;
    String fErrorText;

    friend class AutoSource;
};

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
struct PipelineStageArgs {
    String fCode;
    std::vector<Compiler::FormatArg>    fFormatArgs;
    std::vector<Compiler::GLSLFunction> fFunctions;
};
#endif

}  // namespace SkSL

#endif
