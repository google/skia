/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Block.h"

#include "src/sksl/dsl/Statement.h"

namespace SkSL {

namespace dsl {

void Block::append(Statement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
