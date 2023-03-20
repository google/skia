/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BLOCK
#define SKSL_BLOCK

#include "include/private/SkSLDefines.h"
#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLPosition.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class SymbolTable;

/**
 * A block of multiple statements functioning as a single statement.
 */
class Block final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kBlock;

    // "kBracedScope" represents an actual language-level block. Other kinds of block are used to
    // pass around multiple statements as if they were a single unit, with no semantic impact.
    enum class Kind {
        kUnbracedBlock,      // Represents a group of statements without curly braces.
        kBracedScope,        // Represents a language-level Block, with curly braces.
        kCompoundStatement,  // A block which conceptually represents a single statement, such as
                             // `int a, b;`. (SkSL represents this internally as two statements:
                             // `int a; int b;`) Allowed to optimize away to its interior Statement.
                             // Treated as a single statement by the debugger.
    };

    Block(Position pos, StatementArray statements,
          Kind kind = Kind::kBracedScope, const std::shared_ptr<SymbolTable> symbols = nullptr)
    : INHERITED(pos, kIRNodeKind)
    , fChildren(std::move(statements))
    , fBlockKind(kind)
    , fSymbolTable(std::move(symbols)) {}

    // Make is allowed to simplify compound statements. For a single-statement unscoped Block,
    // Make can return the Statement as-is. For an empty unscoped Block, Make can return Nop.
    static std::unique_ptr<Statement> Make(Position pos,
                                           StatementArray statements,
                                           Kind kind = Kind::kBracedScope,
                                           std::shared_ptr<SymbolTable> symbols = nullptr);

    // MakeBlock always makes a real Block object. This is important because many callers rely on
    // Blocks specifically; e.g. a function body must be a scoped Block, nothing else will do.
    static std::unique_ptr<Block> MakeBlock(Position pos,
                                            StatementArray statements,
                                            Kind kind = Kind::kBracedScope,
                                            std::shared_ptr<SymbolTable> symbols = nullptr);

    const StatementArray& children() const {
        return fChildren;
    }

    StatementArray& children() {
        return fChildren;
    }

    bool isScope() const {
        return fBlockKind == Kind::kBracedScope;
    }

    Kind blockKind() const {
        return fBlockKind;
    }

    void setBlockKind(Kind kind) {
        fBlockKind = kind;
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

    std::string description() const override;

private:
    StatementArray fChildren;
    Kind fBlockKind;
    std::shared_ptr<SymbolTable> fSymbolTable;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
