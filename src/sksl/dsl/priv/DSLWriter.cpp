/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSL.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"

namespace skslcode {

SkSL::Compiler& DSLWriter::compiler() {
    return fCompiler;
}

SkSL::IRGenerator& DSLWriter::irGenerator() {
    return *fCompiler.fIRGenerator;
}

const SkSL::Context& DSLWriter::context() {
    return this->irGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::symbolTable() {
    return fCompiler.fIRGenerator->fSymbolTable;
}

SkSL::ModifiersPool::Handle DSLWriter::modifiers(SkSL::Modifiers modifiers) {
    return fCompiler.fIRGenerator->fModifiers->handle(modifiers);
}

SkSL::String DSLWriter::name(const char* name) {
    return name + SkSL::String("_") + SkSL::to_string(++fNameCount);
}

DSLWriter& DSLWriter::Instance() {
    static SkSL::Compiler compiler;
    static DSLWriter thisShouldBeMadeThreadSafe(&compiler);
    return thisShouldBeMadeThreadSafe;
}

std::unique_ptr<SkSL::Statement> Statement(std::unique_ptr<SkSL::Statement> stmt) {
    return std::move(stmt);
}

std::unique_ptr<SkSL::Statement> Statement(Expression expr) {
    return std::make_unique<SkSL::ExpressionStatement>(expr.release());
}

} // namespace skslcode
