/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_SYMBOLS
#define SKSL_DSL_SYMBOLS

namespace SkSL {
namespace dsl {

// This header provides methods for manually managing symbol tables in DSL code. They should not be
// used by normal hand-written DSL code, where we rely on C++ to manage symbols, but are instead
// needed when DSL objects are being constructed programmatically (as in Parser).

/**
 * Pushes a new symbol table onto the symbol table stack.
 */
void PushSymbolTable();

/**
 * Pops the top symbol table from the stack. As symbol tables are shared pointers, this will only
 * destroy the symbol table if it was never attached to anything (e.g. passed into a Block
 * constructor).
 */
void PopSymbolTable();

} // namespace dsl
} // namespace SkSL

#endif
