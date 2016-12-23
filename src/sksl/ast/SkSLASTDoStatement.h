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
    ASTDoStatement(Position position, sk_up<ASTStatement> statement, sk_up<ASTExpression> test)
            : INHERITED(position, kDo_Kind)
            , fStatement(std::move(statement))
            , fTest(std::move(test)) {}

    SkString description() const override {
        return "do " + fStatement->description() + " while (" + fTest->description() + ");";
    }

    const sk_up<ASTStatement> fStatement;
    const sk_up<ASTExpression> fTest;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
