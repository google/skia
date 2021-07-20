/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLSymbols.h"

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

namespace dsl {

void PushSymbolTable() {
    DSLWriter::IRGenerator().pushSymbolTable();
}

void PopSymbolTable() {
    DSLWriter::IRGenerator().popSymbolTable();
}

std::shared_ptr<SymbolTable> CurrentSymbolTable() {
    return DSLWriter::IRGenerator().symbolTable();
}

DSLPossibleExpression Symbol(skstd::string_view name) {
    return DSLWriter::IRGenerator().convertIdentifier(/*offset=*/-1, name);
}

bool IsType(skstd::string_view name) {
    const SkSL::Symbol* s = (*CurrentSymbolTable())[name];
    return s && s->is<Type>();
}

void AddToSymbolTable(DSLVarBase& var) {
    const SkSL::Variable* skslVar = DSLWriter::Var(var);
    if (skslVar) {
        CurrentSymbolTable()->addWithoutOwnership(skslVar);
    }
}

const String* Retain(String string) {
    return CurrentSymbolTable()->takeOwnershipOfString(std::move(string));
}

} // namespace dsl

} // namespace SkSL
