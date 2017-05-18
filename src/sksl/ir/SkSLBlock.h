/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BLOCK
#define SKSL_BLOCK

#include "SkSLStatement.h"
#include "SkSLSymbolTable.h"

namespace SkSL {

/**
 * A block of multiple statements functioning as a single statement.
 */
struct Block : public Statement {
    Block(Position position, std::vector<std::unique_ptr<Statement>> statements,
          const std::shared_ptr<SymbolTable> symbols = nullptr)
    : INHERITED(position, kBlock_Kind)
    , fSymbols(std::move(symbols))
    , fStatements(std::move(statements)) {}

    bool isEmpty() const override {
        for (const auto& s : fStatements) {
            if (!s->isEmpty()) {
                return false;
            }
        }
        return true;
    }

    String description() const override {
        String result("{");
        for (size_t i = 0; i < fStatements.size(); i++) {
            result += "\n";
            result += fStatements[i]->description();
        }
        result += "\n}\n";
        return result;
    }

    // it's important to keep fStatements defined after (and thus destroyed before) fSymbols,
    // because destroying statements can modify reference counts in symbols
    const std::shared_ptr<SymbolTable> fSymbols;
    std::vector<std::unique_ptr<Statement>> fStatements;

    typedef Statement INHERITED;
};

} // namespace

#endif
