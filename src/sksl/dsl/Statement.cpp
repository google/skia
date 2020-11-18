/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Statement.h"

#include "src/sksl/dsl/DSL.h"

namespace SkSL {

namespace dsl {

Statement::Statement(std::initializer_list<Statement> statements)
    : fStatement(Block(statements).release()) {}

} // namespace dsl

} // namespace SkSL
