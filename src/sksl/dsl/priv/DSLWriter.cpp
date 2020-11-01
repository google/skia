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

DSLWriter::DSLWriter(SkSL::IRGenerator* irGenerator)
    : fIRGenerator(*irGenerator) {}

DSLWriter::~DSLWriter() {}

const SkSL::Context& DSLWriter::context() {
    return fIRGenerator.fContext;
}

const std::shared_ptr<SkSL::SymbolTable> DSLWriter::symbolTable() {
    return fIRGenerator.fSymbolTable;
}

SkSL::ModifiersPool::Handle DSLWriter::modifiers(SkSL::Modifiers modifiers) {
    return fIRGenerator.fModifiers->handle(modifiers);
}

int DSLWriter::declare(std::unique_ptr<SkSL::Variable> var) {
    fVariables.push_back(this->symbolTable()->add(std::move(var)));
    return fVariables.size() - 1;
}

DSLWriter& writer() {
    static SkSL::Compiler compiler;
    static DSLWriter thisShouldBeMadeThreadSafe(&compiler.irGenerator());
    return thisShouldBeMadeThreadSafe;
}

} // namespace skslcode
