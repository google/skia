/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace skslcode {

DSLWriter::DSLWriter(SkSL::Compiler* compiler)
    : fCompiler(*compiler) {
    SkSL::ParsedModule module = fCompiler.moduleForProgramKind(SkSL::Program::kFragment_Kind);
    const SkSL::Variable* fragColor = static_cast<const SkSL::Variable*>(
                                                                (*module.fSymbols)["sk_FragColor"]);
    fSkFragColorIndex = this->registerBuiltin(fragColor);
}

DSLWriter::~DSLWriter() {}

SkSL::IRGenerator& DSLWriter::irGenerator() {
    return *fCompiler.fIRGenerator;
}

const SkSL::Context& DSLWriter::context() {
    return this->irGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::symbolTable() {
    return fCompiler.fIRGenerator->symbolTable();
}

SkSL::ModifiersPool::Handle DSLWriter::modifiers(SkSL::Modifiers modifiers) {
    return fCompiler.fIRGenerator->modifiersHandle(modifiers);
}

int DSLWriter::declare(std::unique_ptr<SkSL::Variable> var) {
    fVariables.push_back(this->symbolTable()->add(std::move(var)));
    return fVariables.size() - 1;
}

int DSLWriter::registerBuiltin(const SkSL::Variable* var) {
    SkASSERT(var);
    fVariables.push_back(var);
    return fVariables.size() - 1;
}

DSLWriter& DSLWriter::Instance() {
    static SkSL::Compiler compiler;
    static DSLWriter thisShouldBeMadeThreadSafe(&compiler);
    return thisShouldBeMadeThreadSafe;
}

} // namespace skslcode
