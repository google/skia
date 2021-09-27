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

std::unique_ptr<Statement> Block::MakeUnscoped(int line, StatementArray statements) {
    // If the Block is completely empty, synthesize a Nop.
    if (statements.empty()) {
        return Nop::Make();
    }

    if (statements.size() > 1) {
        // The statement array contains multiple statements, but some of those might be no-ops.
        // If the statement array only contains one real statement, we can return that directly and
        // avoid creating an additional Block node.
        std::unique_ptr<Statement>* foundStatement = nullptr;
        for (std::unique_ptr<Statement>& stmt : statements) {
            if (!stmt->isEmpty()) {
                if (!foundStatement) {
                    // We found a single non-empty statement. Remember it and keep looking.
                    foundStatement = &stmt;
                    continue;
                }
                // We found more than one non-empty statement. We actually do need a Block.
                return std::make_unique<Block>(line, std::move(statements),
                                               /*symbols=*/nullptr, /*isScope=*/false);
            }
        }

        // The array wrapped one valid Statement. Avoid allocating a Block by returning it directly.
        if (foundStatement) {
            return std::move(*foundStatement);
        }

        // The statement array contained nothing but empty statements!
        // In this case, we don't actually need to allocate a Block.
        // We can just return one of those empty statements. Fall through to...
    }

    return std::move(statements.front());
}

std::unique_ptr<Block> Block::Make(int line,
                                   StatementArray statements,
                                   std::shared_ptr<SymbolTable> symbols,
                                   bool isScope) {
    // Nothing to optimize here--eliminating empty statements doesn't actually improve the generated
    // code, and we promise to return a Block.
    return std::make_unique<Block>(line, std::move(statements), std::move(symbols), isScope);
}

std::unique_ptr<Statement> Block::clone() const {
    StatementArray cloned;
    cloned.reserve_back(this->children().size());
    for (const std::unique_ptr<Statement>& stmt : this->children()) {
        cloned.push_back(stmt->clone());
    }
    return std::make_unique<Block>(fLine,
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
