/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLBlock.h"

#include "include/sksl/DSLStatement.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLBlock.h"

#include <utility>

namespace SkSL {

namespace dsl {

DSLBlock::DSLBlock(SkSL::StatementArray statements,
                   std::shared_ptr<SymbolTable> symbols,
                   Position pos)
        : fStatements(std::move(statements))
        , fSymbols(std::move(symbols))
        , fPosition(pos) {}

DSLBlock::DSLBlock(SkTArray<DSLStatement> statements,
                   std::shared_ptr<SymbolTable> symbols,
                   Position pos)
        : fSymbols(std::move(symbols))
        , fPosition(pos) {
    fStatements.reserve_back(statements.size());
    for (DSLStatement& s : statements) {
        fStatements.push_back(s.release());
    }
}

std::unique_ptr<SkSL::Block> DSLBlock::release() {
    return std::make_unique<SkSL::Block>(fPosition, std::move(fStatements),
                                         Block::Kind::kBracedScope, std::move(fSymbols));
}

void DSLBlock::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
