/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_COMPILER
#define SKSL_COMPILER

#include <set>
#include <vector>
#include "ir/SkSLProgram.h"
#include "ir/SkSLSymbolTable.h"
#include "SkSLCFGGenerator.h"
#include "SkSLContext.h"
#include "SkSLErrorReporter.h"
#include "SkSLIRGenerator.h"

#define SK_FRAGCOLOR_BUILTIN    10001
#define SK_IN_BUILTIN           10002
#define SK_FRAGCOORD_BUILTIN       15
#define SK_VERTEXID_BUILTIN         5
#define SK_CLIPDISTANCE_BUILTIN     3
#define SK_INVOCATIONID_BUILTIN     8

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
    Compiler();

    ~Compiler() override;

    std::unique_ptr<Program> convertProgram(Program::Kind kind, SkString text,
                                            const Program::Settings& settings);

    bool toSPIRV(const Program& program, SkWStream& out);

    bool toSPIRV(const Program& program, SkString* out);

    bool toGLSL(const Program& program, SkWStream& out);

    bool toGLSL(const Program& program, SkString* out);

    void error(Position position, SkString msg) override;

    SkString errorText();

    void writeErrorCount();

    int errorCount() override {
        return fErrorCount;
    }

private:
    void addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                       DefinitionMap* definitions);

    void addDefinitions(const BasicBlock::Node& node, DefinitionMap* definitions);

    void scanCFG(CFG* cfg, BlockId block, std::set<BlockId>* workList);

    void scanCFG(const FunctionDefinition& f);

    void internalConvertProgram(SkString text,
                                Modifiers::Flag* defaultPrecision,
                                std::vector<std::unique_ptr<ProgramElement>>* result);

    std::shared_ptr<SymbolTable> fTypes;
    IRGenerator* fIRGenerator;
    SkString fSkiaVertText; // FIXME store parsed version instead

    Context fContext;
    int fErrorCount;
    SkString fErrorText;
};

} // namespace

#endif
