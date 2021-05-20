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

DSLCase::DSLCase(DSLExpression value, SkTArray<DSLStatement> statements)
    : fValue(std::move(value)) {
    fStatements.reserve_back(statements.count());
    for (DSLStatement& stmt : statements) {
        fStatements.push_back(stmt.release());
    }
}

DSLCase::DSLCase(DSLCase&& other)
    : fValue(std::move(other.fValue))
    , fStatements(std::move(other.fStatements)) {}

DSLCase::~DSLCase() {}

DSLCase& DSLCase::operator=(DSLCase&& other) {
    fValue = std::move(other.fValue);
    fStatements = std::move(other.fStatements);
    return *this;
}

void DSLCase::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
