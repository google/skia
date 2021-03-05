/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCase.h"

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

namespace dsl {

DSLCase::DSLCase(Value value, SkSL::StatementArray statements)
    : fValue(value)
    , fStatements(std::move(statements)) {}

DSLCase::DSLCase(DSLCase&& other)
    : fValue(other.fValue)
    , fStatements(std::move(other.fStatements)) {}

DSLCase::~DSLCase() {}

void DSLCase::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

std::unique_ptr<Expression> DSLCase::value() {
    if (fValue.fIsDefault) {
        return nullptr;
    }
    return DSLExpression(fValue.fValue).release();
}

} // namespace dsl

} // namespace SkSL
