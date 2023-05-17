/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_STATEMENT
#define SKSL_DSL_STATEMENT

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <utility>

namespace SkSL::dsl {

class DSLExpression;

class DSLStatement {
public:
    DSLStatement() = default;
    ~DSLStatement() = default;

    DSLStatement(DSLStatement&&) = default;
    DSLStatement& operator=(DSLStatement&& other) = default;

    DSLStatement(const DSLStatement&) = delete;
    DSLStatement& operator=(const DSLStatement& other) = delete;

    DSLStatement(DSLExpression expr);

    DSLStatement(std::unique_ptr<SkSL::Statement> stmt, Position pos);
    DSLStatement(std::unique_ptr<SkSL::Statement> stmt);

    Position position() {
        SkASSERT(this->hasValue());
        return fStatement->fPosition;
    }

    void setPosition(Position pos) {
        SkASSERT(this->hasValue());
        fStatement->fPosition = pos;
    }

    bool hasValue() { return fStatement != nullptr; }

    std::unique_ptr<SkSL::Statement> release() {
        SkASSERT(this->hasValue());
        return std::move(fStatement);
    }

    std::unique_ptr<SkSL::Statement> releaseIfPossible() {
        return std::move(fStatement);
    }

private:
    std::unique_ptr<SkSL::Statement> fStatement;
};

}  // namespace SkSL::dsl

#endif
