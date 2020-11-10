/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSL.h"

#include "src/gpu/mock/GrMockCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"

namespace skslcode {

DSLWriter::~DSLWriter() {
    SkASSERT(fVars.count() == 0);
}

SkSL::IRGenerator& DSLWriter::irGenerator() {
    return *fCompiler.fIRGenerator;
}

const SkSL::Context& DSLWriter::context() {
    return this->irGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::symbolTable() {
    SkASSERT(fCompiler.fIRGenerator->fSymbolTable);
    return fCompiler.fIRGenerator->fSymbolTable;
}

SkSL::ModifiersPool::Handle DSLWriter::modifiers(SkSL::Modifiers modifiers) {
    return fCompiler.fIRGenerator->fModifiers->handle(modifiers);
}

SkSL::String DSLWriter::name(const char* name) {
    return name + SkSL::String("_") + SkSL::to_string(++fNameCount);
}

SkSL::StatementArray DSLWriter::pendingDeclarations() {
    SkSL::StatementArray result;
    for (const SkSL::Variable* var : fVars) {
        result.emplace_back(new SkSL::VarDeclaration(var, &var->type(),  SkSL::ExpressionArray(),
                                                     nullptr));
    }
    fVars.reset();
    return result;
}

DSLWriter& DSLWriter::Instance() {
#ifndef SKSL_STANDALONE
    static SkSL::Compiler* compiler = nullptr;
    static DSLWriter* thisNeedsToBeMadeThreadSafe;
    static SkSL::Program::Settings settings;
    static GrMockCaps caps((GrContextOptions()), GrMockOptions());
    if (!compiler) {
        compiler = new SkSL::Compiler(caps.shaderCaps());
        compiler->fInliner.reset(compiler->fContext.get(), compiler->fIRGenerator->fModifiers.get(),
                                 &settings, caps.shaderCaps());
        compiler->fIRGenerator->fSettings = &settings;
        thisNeedsToBeMadeThreadSafe = new DSLWriter(compiler);
    }
    return *thisNeedsToBeMadeThreadSafe;
#else
    SkUNREACHABLE;
#endif
}

std::unique_ptr<SkSL::Statement> Statement(std::unique_ptr<SkSL::Statement> stmt) {
    return std::move(stmt);
}

std::unique_ptr<SkSL::Statement> Statement(Expression expr) {
    return std::make_unique<SkSL::ExpressionStatement>(expr.release());
}

} // namespace skslcode
