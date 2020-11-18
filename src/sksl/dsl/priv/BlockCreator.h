/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCKCREATOR
#define SKSL_DSL_BLOCKCREATOR

#include "src/sksl/dsl/Statement.h"
#include "src/sksl/ir/SkSLBlock.h"

namespace SkSL {

namespace dsl {

template<class... Statements>
class BlockCreator {
public:
    BlockCreator() {}

    void addStatements(SkSL::StatementArray& statements) {
    }

    std::unique_ptr<SkSL::Statement> block() const {
        return std::make_unique<SkSL::Block>(/*offset=*/-1,SkSL::StatementArray());
    }
};

template<class First, class... Rest>
class BlockCreator<First, Rest...> : public BlockCreator<Rest...> {
public:
    BlockCreator(First first, Rest... rest)
        : BlockCreator<Rest...>(std::move(rest)...)
        , fFirst(std::move(first)) {}

    void addStatements(SkSL::StatementArray& statements) {
        statements.push_back(Statement(std::move(fFirst)).release());
        BlockCreator<Rest...>::addStatements(statements);
    }

    std::unique_ptr<SkSL::Statement> block() {
        SkSL::StatementArray statements;
        this->addStatements(statements);
        return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(statements));
    }

private:
    First fFirst;
};

} // namespace dsl

} // namespace SkSL

#endif
