/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BLOCK
#define SKSL_BLOCK

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A block of multiple statements functioning as a single statement.
 */
struct Block : public Statement {
    Block(IRGenerator* irGenerator, int offset, std::vector<IRNode::ID> statements,
          const std::shared_ptr<SymbolTable> symbols = nullptr)
    : INHERITED(irGenerator, offset, kBlock_Kind)
    , fSymbols(std::move(symbols))
    , fStatements(std::move(statements)) {}

    bool isEmpty() const override {
        for (const auto& s : fStatements) {
            if (!s.node().isEmpty()) {
                return false;
            }
        }
        return true;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new Block(fIRGenerator, fOffset, fStatements, fSymbols));
    }

    String description() const override {
        String result("{");
        for (size_t i = 0; i < fStatements.size(); i++) {
            result += "\n";
            result += fStatements[i].node().description();
        }
        result += "\n}\n";
        return result;
    }

    // it's important to keep fStatements defined after (and thus destroyed before) fSymbols,
    // because destroying statements can modify reference counts in symbols
    const std::shared_ptr<SymbolTable> fSymbols;
    std::vector<IRNode::ID> fStatements;

    typedef Statement INHERITED;
};

} // namespace

#endif
