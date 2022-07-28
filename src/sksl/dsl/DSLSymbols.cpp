/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLSymbols.h"

#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <type_traits>

namespace SkSL {

namespace dsl {

class DSLVarBase;

void PushSymbolTable() {
    SymbolTable::Push(&ThreadContext::SymbolTable());
}

void PopSymbolTable() {
    SymbolTable::Pop(&ThreadContext::SymbolTable());
}

std::shared_ptr<SymbolTable> CurrentSymbolTable() {
    return ThreadContext::SymbolTable();
}

DSLExpression Symbol(std::string_view name, Position pos) {
    return DSLExpression(ThreadContext::Compiler().convertIdentifier(pos, name), pos);
}

void AddToSymbolTable(DSLVarBase& var, Position pos) {
    const SkSL::Variable* skslVar = DSLWriter::Var(var);
    if (skslVar) {
        CurrentSymbolTable()->addWithoutOwnership(skslVar);
    }
}

} // namespace dsl

} // namespace SkSL
