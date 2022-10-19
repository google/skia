/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLSymbols.h"

#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {
namespace dsl {

void PushSymbolTable() {
    SymbolTable::Push(&ThreadContext::SymbolTable());
}

void PopSymbolTable() {
    SymbolTable::Pop(&ThreadContext::SymbolTable());
}

} // namespace dsl
} // namespace SkSL
