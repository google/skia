/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLBlock.h"

#include "src/sksl/ir/SkSLNop.h"

namespace SkSL {

std::unique_ptr<Statement> Block::Make(Position pos,
                                       StatementArray statements,
                                       Kind kind,
                                       std::unique_ptr<SymbolTable> symbols) {
    // We can't simplify away braces or populated symbol tables.
    if (kind == Kind::kBracedScope || (symbols && symbols->count())) {
        return std::make_unique<Block>(pos, std::move(statements), kind, std::move(symbols));
    }

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
                return std::make_unique<Block>(pos, std::move(statements), kind,
                                               /*symbols=*/nullptr);
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

std::unique_ptr<Block> Block::MakeBlock(Position pos,
                                        StatementArray statements,
                                        Kind kind,
                                        std::unique_ptr<SymbolTable> symbols) {
    // Nothing to optimize here--eliminating empty statements doesn't actually improve the generated
    // code, and we promise to return a Block.
    return std::make_unique<Block>(pos, std::move(statements), kind, std::move(symbols));
}

std::unique_ptr<Statement> Block::MakeCompoundStatement(std::unique_ptr<Statement> existing,
                                                        std::unique_ptr<Statement> additional) {
    // If either of the two Statements is empty, return the other.
    if (!existing || existing->isEmpty()) {
        return additional;
    }
    if (!additional || additional->isEmpty()) {
        return existing;
    }

    // If the existing statement is a compound-statement Block, append the additional statement.
    if (existing->is<Block>()) {
        SkSL::Block& block = existing->as<Block>();
        if (block.blockKind() == Block::Kind::kCompoundStatement) {
            block.children().push_back(std::move(additional));
            return existing;
        }
    }

    // The existing statement was not a compound-statement Block; create one, and put both
    // statements inside of it.
    Position pos = existing->fPosition.rangeThrough(additional->fPosition);
    StatementArray stmts;
    stmts.reserve_exact(2);
    stmts.push_back(std::move(existing));
    stmts.push_back(std::move(additional));
    return Block::Make(pos, std::move(stmts), Block::Kind::kCompoundStatement);
}

std::string Block::description() const {
    std::string result;

    // Write scope markers if this block is a scope, or if the block is empty (since we need to emit
    // something here to make the code valid).
    bool isScope = this->isScope() || this->isEmpty();
    if (isScope) {
        result += "{";
    }
    for (const std::unique_ptr<Statement>& stmt : this->children()) {
        result += "\n";
        result += stmt->description();
    }
    result += isScope ? "\n}\n" : "\n";
    return result;
}

}  // namespace SkSL
