/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLNop.h"

#include <iterator>

namespace SkSL {

static void eliminate_empty_statements(StatementArray* statements) {
    // Remove all the statements which are empty.
    std::unique_ptr<Statement>* iter = std::remove_if(
            statements->begin(), statements->end(), [](const std::unique_ptr<Statement>& stmt) {
                return stmt->isEmpty();
            });

    // Shrink the statement array to match.
    statements->resize(std::distance(statements->begin(), iter));
}

std::unique_ptr<Statement> Block::MakeUnscoped(int offset, StatementArray statements) {
    eliminate_empty_statements(&statements);
    if (statements.empty()) {
        return Nop::Make();
    }
    if (statements.size() == 1) {
        return std::move(statements.front());
    }
    return std::make_unique<Block>(offset, std::move(statements),
                                   /*symbols=*/nullptr, /*isScope=*/false);
}

std::unique_ptr<Block> Block::Make(int offset,
                                   StatementArray statements,
                                   std::shared_ptr<SymbolTable> symbols,
                                   bool isScope) {
    // Nothing to optimize here--eliminate_empty_statements doesn't actually improve the generated
    // code, and we promise to return a Block.
    return std::make_unique<Block>(offset, std::move(statements), std::move(symbols), isScope);
}

std::unique_ptr<Statement> Block::clone() const {
    StatementArray cloned;
    cloned.reserve_back(this->children().size());
    for (const std::unique_ptr<Statement>& stmt : this->children()) {
        cloned.push_back(stmt->clone());
    }
    return std::make_unique<Block>(fOffset,
                                   std::move(cloned),
                                   SymbolTable::WrapIfBuiltin(this->symbolTable()),
                                   this->isScope());
}

String Block::description() const {
    String result;
    if (fIsScope) {
        result += "{";
    }
    for (const std::unique_ptr<Statement>& stmt : this->children()) {
        result += "\n";
        result += stmt->description();
    }
    result += fIsScope ? "\n}\n" : "\n";
    return result;
}

}  // namespace SkSL
