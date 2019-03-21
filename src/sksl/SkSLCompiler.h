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
#include "ir/SkSLProgram.h"
#include "ir/SkSLSymbolTable.h"
#include "SkSLByteCode.h"
#include "SkSLCFGGenerator.h"
#include "SkSLContext.h"
#include "SkSLErrorReporter.h"
#include "SkSLLexer.h"

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
#define SK_VERTEXID_BUILTIN               42
#define SK_INSTANCEID_BUILTIN             43
#define SK_CLIPDISTANCE_BUILTIN            3
#define SK_INVOCATIONID_BUILTIN            8
#define SK_POSITION_BUILTIN                0

namespace SkSL {

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
            kUniform,
            kChildProcessor
        };

        FormatArg(Kind kind)
                : fKind(kind) {}

        FormatArg(Kind kind, int index)
                : fKind(kind)
                , fIndex(index) {}

        Kind fKind;

        int fIndex;
    };

    Compiler(Flags flags = kNone_Flags);

    ~Compiler() override;

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

    bool toPipelineStage(const Program& program, String* out,
                         std::vector<FormatArg>* outFormatArgs);

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

    std::vector<std::unique_ptr<ProgramElement>> fVertexInclude;
    std::shared_ptr<SymbolTable> fVertexSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fFragmentInclude;
    std::shared_ptr<SymbolTable> fFragmentSymbolTable;
    std::vector<std::unique_ptr<ProgramElement>> fGeometryInclude;
    std::shared_ptr<SymbolTable> fGeometrySymbolTable;

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
