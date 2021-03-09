/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCase.h"

#include "include/private/SkSLStatement.h"

namespace SkSL {

namespace dsl {

DSLCase::DSLCase(DSLExpression value, SkSL::StatementArray statements)
    : fValue(std::move(value))
    , fStatements(std::move(statements)) {}

DSLCase::DSLCase(DSLCase&& other)
    : fValue(std::move(other.fValue))
    , fStatements(std::move(other.fStatements)) {}

DSLCase::~DSLCase() {}

void DSLCase::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
