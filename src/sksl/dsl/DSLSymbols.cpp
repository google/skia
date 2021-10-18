/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLSymbols.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

namespace dsl {

static bool is_type_in_symbol_table(skstd::string_view name, SkSL::SymbolTable* symbols) {
    const SkSL::Symbol* s = (*symbols)[name];
    return s && s->is<Type>();
}

void PushSymbolTable() {
    SymbolTable::Push(&ThreadContext::SymbolTable());
}

void PopSymbolTable() {
    SymbolTable::Pop(&ThreadContext::SymbolTable());
}

std::shared_ptr<SymbolTable> CurrentSymbolTable() {
    return ThreadContext::SymbolTable();
}

DSLPossibleExpression Symbol(skstd::string_view name, PositionInfo pos) {
    return ThreadContext::Compiler().convertIdentifier(pos.line(), name);
}

bool IsType(skstd::string_view name) {
    return is_type_in_symbol_table(name, CurrentSymbolTable().get());
}

bool IsBuiltinType(skstd::string_view name) {
    return is_type_in_symbol_table(name, CurrentSymbolTable()->builtinParent());
}

void AddToSymbolTable(DSLVarBase& var, PositionInfo pos) {
    const SkSL::Variable* skslVar = DSLWriter::Var(var);
    if (skslVar) {
        CurrentSymbolTable()->addWithoutOwnership(skslVar);
    }
    ThreadContext::ReportErrors(pos);
}

const String* Retain(String string) {
    return CurrentSymbolTable()->takeOwnershipOfString(std::move(string));
}

} // namespace dsl

} // namespace SkSL
