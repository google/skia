/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCK
#define SKSL_DSL_BLOCK

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"

#include <memory>

namespace SkSL {

class Statement;

namespace dsl {

class DSLBlock {
public:
    template<class... Statements>
    DSLBlock(Statements... statements) {
        fStatements.reserve_back(sizeof...(statements));
        // in C++17, we could just do:
        // (fStatements.push_back(DSLStatement(statements.release()).release()), ...);
        int unused[] =
            {0,
            (static_cast<void>(fStatements.push_back(DSLStatement(statements.release()).release())),
             0)...};
        static_cast<void>(unused);
    }

    DSLBlock(SkSL::StatementArray statements);

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
