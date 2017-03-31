/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTBLOCK
#define SKSL_ASTBLOCK

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * Represents a curly-braced block of statements.
 */
struct ASTBlock : public ASTStatement {
    ASTBlock(Position position, std::vector<std::unique_ptr<ASTStatement>> statements)
    : INHERITED(position, kBlock_Kind)
    , fStatements(std::move(statements)) {}

    String description() const override {
        String result("{");
        for (size_t i = 0; i < fStatements.size(); i++) {
            result += "\n";
            result += fStatements[i]->description();
        }
        result += "\n}\n";
        return result;
    }

    const std::vector<std::unique_ptr<ASTStatement>> fStatements;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
