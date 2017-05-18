/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTDOSTATEMENT
#define SKSL_ASTDOSTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'do' loop.
 */
struct ASTDoStatement : public ASTStatement {
    ASTDoStatement(Position position, std::unique_ptr<ASTStatement> statement,
                   std::unique_ptr<ASTExpression> test)
    : INHERITED(position, kDo_Kind)
    , fStatement(std::move(statement))
    , fTest(std::move(test)) {}

    String description() const override {
        return "do " + fStatement->description() + " while (" + fTest->description() + ");";
    }

    const std::unique_ptr<ASTStatement> fStatement;
    const std::unique_ptr<ASTExpression> fTest;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
