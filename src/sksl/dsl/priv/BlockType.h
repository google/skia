/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCKTYPE
#define SKSL_DSL_BLOCKTYPE

#include "src/sksl/dsl/priv/Statement.h"
#include "src/sksl/ir/SkSLBlock.h"

namespace skslcode {

template<class... Statements>
class BlockType : Statement {
public:
    BlockType() {}

    void addStatements(SkSL::StatementArray& statements) const {
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::Block>(/*offset=*/-1,SkSL::StatementArray());
    }
};

template<class First, class... Rest>
class BlockType<First, Rest...> : public BlockType<Rest...> {
public:
    BlockType(First first, Rest... rest)
        : BlockType<Rest...>(rest...)
        , fFirst(first) {}

    void addStatements(SkSL::StatementArray& statements) const {
        statements.push_back(fFirst.statement());
        BlockType<Rest...>::addStatements(statements);
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        SkSL::StatementArray statements;
        this->addStatements(statements);
        return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(statements));
    }

    First fFirst;
};

} // namespace skslcode

#endif
