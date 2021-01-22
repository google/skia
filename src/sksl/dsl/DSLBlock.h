/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCK
#define SKSL_DSL_BLOCK

#include "include/private/SkTArray.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>

namespace SkSL {

class Statement;

namespace dsl {

class DSLBlock {
public:
    template<class... Statements>
    DSLBlock(Statements... statements) {
        fStatements.reserve_back(sizeof...(statements));
        (fStatements.push_back(DSLStatement(std::move(statements)).release()), ...);
    }

    DSLBlock(SkSL::StatementArray statements)
        : fStatements(std::move(statements)) {}

    void append(DSLStatement stmt);

private:
    std::unique_ptr<SkSL::Statement> release();

    SkSL::StatementArray fStatements;

    friend class DSLStatement;
    friend class DSLFunction;
};

} // namespace dsl

} // namespace SkSL

#endif
