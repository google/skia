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
#include "SkSLCFGGenerator.h"
#include "SkSLContext.h"
#include "SkSLErrorReporter.h"
#include "SkSLIRGenerator.h"

#define SK_FRAGCOLOR_BUILTIN           10001
#define SK_IN_BUILTIN                  10002
#define SK_INCOLOR_BUILTIN             10003
#define SK_OUTCOLOR_BUILTIN            10004
#define SK_TRANSFORMEDCOORDS2D_BUILTIN 10005
#define SK_TEXTURESAMPLERS_BUILTIN     10006
#define SK_FRAGCOORD_BUILTIN              15
#define SK_VERTEXID_BUILTIN                5
#define SK_CLIPDISTANCE_BUILTIN            3
#define SK_INVOCATIONID_BUILTIN            8

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
class Compiler : public ErrorReporter {
public:
    enum Flags {
        kNone_Flags = 0,
        // permits static if/switch statements to be used with non-constant tests. This is used when
        // producing H and CPP code; the static tests don't have to have constant values *yet*, but
        // the generated code will contain a static test which then does have to be a constant.
        kPermitInvalidStaticTests_Flag = 1,
    };

    Compiler(Flags flags = kNone_Flags);

    ~Compiler() override;

    std::unique_ptr<Program> convertProgram(Program::Kind kind, String text,
                                            const Program::Settings& settings);

    bool toSPIRV(const Program& program, OutputStream& out);

    bool toSPIRV(const Program& program, String* out);

    bool toGLSL(const Program& program, OutputStream& out);

    bool toGLSL(const Program& program, String* out);

    bool toCPP(const Program& program, String name, OutputStream& out);

    bool toH(const Program& program, String name, OutputStream& out);

    void error(Position position, String msg) override;

    String errorText();

    void writeErrorCount();

    int errorCount() override {
        return fErrorCount;
    }

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

    std::shared_ptr<SymbolTable> fTypes;
    IRGenerator* fIRGenerator;
    String fSkiaVertText; // FIXME store parsed version instead
    int fFlags;

    Context fContext;
    int fErrorCount;
    String fErrorText;
};

} // namespace

#endif
