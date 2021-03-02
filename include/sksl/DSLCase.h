/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CASE
#define SKSL_DSL_CASE

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"

#include <memory>

namespace SkSL {

class Statement;

namespace dsl {

class DSLCase {
public:
    // An empty expression means 'default:'.
    template<class... Statements>
    DSLCase(DSLExpression value, Statements... statements)
        : fValue(std::move(value)) {
        fStatements.reserve_back(sizeof...(statements));
        (fStatements.push_back(DSLStatement(std::move(statements)).release()), ...);
    }

    DSLCase(DSLExpression value, SkSL::StatementArray statements);

    ~DSLCase();

    void append(DSLStatement stmt);

private:
    DSLExpression fValue;
    SkSL::StatementArray fStatements;

    template<class... Cases>
    friend DSLPossibleStatement Switch(DSLExpression value, Cases... cases);
};

} // namespace dsl

} // namespace SkSL

#endif
