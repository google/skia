/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLBlock.h"

#include "include/sksl/DSLStatement.h"
#include "src/sksl/ir/SkSLBlock.h"

namespace SkSL {

namespace dsl {

DSLBlock::DSLBlock(SkSL::StatementArray statements)
    : fStatements(std::move(statements)) {}

DSLBlock::~DSLBlock() {
    if (!fStatements.empty()) {
        // This will convert our Block into a DSLStatement, which is then immediately freed.
        // If an FP is being generated, this will naturally incorporate the Block's Statement into
        // our FP. If not, this will assert that unused code wasn't incorporated into the program.
        DSLStatement(this->release());
    }
}

std::unique_ptr<SkSL::Statement> DSLBlock::release() {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(fStatements));
}

void DSLBlock::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
