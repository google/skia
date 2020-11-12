/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {
    class Compiler;
    class Context;
    class IRGenerator;
    class SymbolTable;
    class Type;
} // namespace sksl

namespace skslcode {

class DSLWriter {
public:
    ~DSLWriter();

    SkSL::Compiler& compiler() {
        return fCompiler;
    }

    SkSL::IRGenerator& irGenerator();

    const SkSL::Context& context();

    const std::shared_ptr<SkSL::SymbolTable>& symbolTable();

    SkSL::ModifiersPool::Handle modifiers(SkSL::Modifiers modifiers);

    SkSL::String name(const char* name);

    // called by Var's constructor so that we know to add a declaration to the next function
    void addVar(Var* var) {
        fVars.push_back(var);
    }

    SkSL::StatementArray pendingDeclarations();

    std::vector<std::unique_ptr<SkSL::ProgramElement>>& programElements() {
        return fProgramElements;
    }

    GrGLSLFragmentProcessor* currentProcessor() {
        return fCurrentProcessor;
    }

    void setCurrentProcessor(GrGLSLFragmentProcessor* processor) {
        fCurrentProcessor = processor;
    }

    GrGLSLFragmentProcessor::EmitArgs* currentEmitArgs() {
        return fEmitArgs;
    }

    void setCurrentEmitArgs(GrGLSLFragmentProcessor::EmitArgs* emitArgs) {
        fEmitArgs = emitArgs;
    }

    void setCurrentOutputName(const char* name);

    void reset() {
        fNameCount = 0;
    }

    static DSLWriter& Instance();

private:
    DSLWriter(SkSL::Compiler* compiler)
        : fCompiler(*compiler) {
        SkSL::ParsedModule module = fCompiler.moduleForProgramKind(SkSL::Program::kFragment_Kind);
        SkSL::IRGenerator& ir = this->irGenerator();
        ir.fSymbolTable = module.fSymbols;
        ir.pushSymbolTable();
    }

    SkSL::Program::Settings fSettings;
    SkSL::Compiler& fCompiler;
    int fNameCount = 0;
    SkTArray<Var*> fVars;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    GrGLSLFragmentProcessor* fCurrentProcessor = nullptr;
    GrGLSLFragmentProcessor::EmitArgs* fEmitArgs = nullptr;
};

} // namespace skslcode

#endif
