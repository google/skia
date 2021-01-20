/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLBlock.h"

#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/ir/SkSLBlock.h"

namespace SkSL {

namespace dsl {

std::unique_ptr<SkSL::Statement> DSLBlock::release() {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(fStatements));
}

void DSLBlock::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
