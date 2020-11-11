/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "src/gpu/mock/GrMockCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"

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
    for (Var* var : fVars) {
        const SkSL::Variable* skslVar = var->var();
        result.emplace_back(new SkSL::VarDeclaration(skslVar, &skslVar->type(),
                                                     SkSL::ExpressionArray(),
                                                     var->fInitialValue.release()));
    }
    fVars.reset();
    return result;
}

void DSLWriter::setCurrentOutputName(const char* name) {
    sk_OutColor.fName = name;
}

DSLWriter& DSLWriter::Instance() {
#ifndef SKSL_STANDALONE
    static SkSL::Program::Settings settings;
    static GrMockCaps caps((GrContextOptions()), GrMockOptions());
    thread_local static DSLWriter* instance = nullptr;
    thread_local static SkSL::Compiler* compiler = nullptr;
    if (!instance) {
        compiler = new SkSL::Compiler(caps.shaderCaps());
        compiler->fInliner.reset(compiler->fContext.get(), compiler->fIRGenerator->fModifiers.get(),
                                 &settings, caps.shaderCaps());
        compiler->fIRGenerator->fSettings = &settings;
        instance = new DSLWriter(compiler);
    }
    return *instance;
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
