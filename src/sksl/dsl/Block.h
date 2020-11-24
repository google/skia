/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCK
#define SKSL_DSL_BLOCK

#include "src/sksl/ir/SkSLBlock.h"

namespace SkSL {

namespace dsl {

class Statement;

class Block {
public:
    template<class... Statements>
    Block(Statements... statements) {
        this->addStatements(statements...);
    }

    void append(Statement stmt);

    std::unique_ptr<SkSL::Statement> release() {
        return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(fStatements));
    }

private:
    template<class... Statements>
    void addStatements() {}

    template<class First, class... Rest>
    void addStatements(First first, Rest... rest) {
        fStatements.push_back(first.release());
        this->addStatements(rest...);
    }

    StatementArray fStatements;
};

} // namespace dsl

} // namespace SkSL

#endif
