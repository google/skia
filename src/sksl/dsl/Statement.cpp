/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Statement.h"

#include "src/sksl/dsl/Block.h"
#include "src/sksl/ir/SkSLStatement.h"


namespace SkSL {

namespace dsl {

Statement::Statement(Block block)
    : fStatement(block.release()) {}

} // namespace dsl

} // namespace SkSL
