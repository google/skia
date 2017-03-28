/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTWHILESTATEMENT
#define SKSL_ASTWHILESTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'while' statement.
 */
struct ASTWhileStatement : public ASTStatement {
    ASTWhileStatement(Position position, std::unique_ptr<ASTExpression> test,
                      std::unique_ptr<ASTStatement> statement)
    : INHERITED(position, kWhile_Kind)
    , fTest(std::move(test))
    , fStatement(std::move(statement)) {}

    String description() const override {
        return "while (" + fTest->description() + ") " + fStatement->description();
    }

    const std::unique_ptr<ASTExpression> fTest;
    const std::unique_ptr<ASTStatement> fStatement;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
