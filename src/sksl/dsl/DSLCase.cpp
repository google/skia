/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCase.h"

namespace SkSL {

namespace dsl {

DSLCase::DSLCase(DSLExpression value, SkSL::StatementArray statements, Position pos)
    : fValue(std::move(value))
    , fStatements(std::move(statements))
    , fPosition(pos) {}

DSLCase::DSLCase(DSLExpression value, SkTArray<DSLStatement> statements, Position pos)
    : fValue(std::move(value))
    , fPosition(pos) {
    fStatements.reserve_back(statements.size());
    for (DSLStatement& stmt : statements) {
        fStatements.push_back(stmt.release());
    }
}

DSLCase::DSLCase(DSLCase&& other)
    : fValue(std::move(other.fValue))
    , fStatements(std::move(other.fStatements)) {}

DSLCase::~DSLCase() {}

DSLCase& DSLCase::operator=(DSLCase&& other) {
    fValue.assign(std::move(other.fValue));
    fStatements = std::move(other.fStatements);
    return *this;
}

void DSLCase::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
