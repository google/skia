/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BLOCK
#define SKSL_BLOCK

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A block of multiple statements functioning as a single statement.
 */
class Block final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kBlock;

    Block(int offset, StatementArray statements,
          const std::shared_ptr<SymbolTable> symbols = nullptr, bool isScope = true)
    : INHERITED(offset, kStatementKind)
    , fChildren(std::move(statements))
    , fSymbolTable(std::move(symbols))
    , fIsScope(isScope) {}

    // Make always makes a real Block object. This is important because many callers rely on Blocks
    // specifically; e.g. a function body must be a scoped Block, nothing else will do.
    static std::unique_ptr<Block> Make(int offset,
                                       StatementArray statements,
                                       std::shared_ptr<SymbolTable> symbols = nullptr,
                                       bool isScope = true);

    // An unscoped Block is just a collection of Statements. For a single-statement Block,
    // MakeUnscoped will return the Statement as-is. For an empty Block, MakeUnscoped returns Nop.
    static std::unique_ptr<Statement> MakeUnscoped(int offset, StatementArray statements);

    const StatementArray& children() const {
        return fChildren;
    }

    StatementArray& children() {
        return fChildren;
    }

    bool isScope() const {
        return fIsScope;
    }

    void setIsScope(bool isScope) {
        fIsScope = isScope;
    }

    std::shared_ptr<SymbolTable> symbolTable() const {
        return fSymbolTable;
    }

    bool isEmpty() const override {
        for (const std::unique_ptr<Statement>& stmt : this->children()) {
            if (!stmt->isEmpty()) {
                return false;
            }
        }
        return true;
    }

    std::unique_ptr<Statement> clone() const override;

    String description() const override;

private:
    StatementArray fChildren;
    std::shared_ptr<SymbolTable> fSymbolTable;
    // If isScope is false, this is just a group of statements rather than an actual language-level
    // block. This allows us to pass around multiple statements as if they were a single unit, with
    // no semantic impact.
    bool fIsScope;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
