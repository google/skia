/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLStatement.h"

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLNop.h"

namespace SkSL::dsl {

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Statement> stmt) : fStatement(std::move(stmt)) {
    SkASSERT(this->hasValue());
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Statement> stmt, Position pos)
        : fStatement(stmt ? std::move(stmt) : SkSL::Nop::Make()) {
    if (pos.valid() && !fStatement->fPosition.valid()) {
        fStatement->fPosition = pos;
    }
}

}  // namespace SkSL::dsl
