/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CASE
#define SKSL_DSL_CASE

#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"

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

    DSLCase(DSLExpression value, SkSL::StatementArray statements)
        : fValue(std::move(value))
        , fStatements(std::move(statements)) {}

    void append(DSLStatement stmt) {
        fStatements.push_back(stmt.release());
    }

//private:
    DSLExpression fValue;
    SkSL::StatementArray fStatements;
};

} // namespace dsl

} // namespace SkSL

#endif
