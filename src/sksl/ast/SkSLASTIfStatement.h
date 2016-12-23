/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTIFSTATEMENT
#define SKSL_ASTIFSTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * An 'if' statement. 
 */
struct ASTIfStatement : public ASTStatement {
    ASTIfStatement(Position position, sk_up<ASTExpression> test, sk_up<ASTStatement> ifTrue,
                   sk_up<ASTStatement> ifFalse)
            : INHERITED(position, kIf_Kind)
            , fTest(std::move(test))
            , fIfTrue(std::move(ifTrue))
            , fIfFalse(std::move(ifFalse)) {}

    SkString description() const override {
        SkString result("if (");
        result += fTest->description();
        result += ") ";
        result += fIfTrue->description();
        if (fIfFalse) {
            result += " else ";
            result += fIfFalse->description();
        }
        return result;        
    }

    const sk_up<ASTExpression> fTest;
    const sk_up<ASTStatement> fIfTrue;
    const sk_up<ASTStatement> fIfFalse;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
