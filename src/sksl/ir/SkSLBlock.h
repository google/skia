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
          const std::shared_ptr<SymbolTable> symbols)
    : INHERITED(position, kBlock_Kind)
    , fStatements(std::move(statements))
    , fSymbols(std::move(symbols)) {}

    SkString description() const override {
        SkString result("{");
        for (size_t i = 0; i < fStatements.size(); i++) {
            result += "\n";
            result += fStatements[i]->description();
        }
        result += "\n}\n";
        return result;
    }

    const std::vector<std::unique_ptr<Statement>> fStatements;
    const std::shared_ptr<SymbolTable> fSymbols;

    typedef Statement INHERITED;
};

} // namespace

#endif
