/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTFORSTATEMENT
#define SKSL_ASTFORSTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'for' loop. 
 */
struct ASTForStatement : public ASTStatement {
    ASTForStatement(Position position, sk_up<ASTStatement> initializer, sk_up<ASTExpression> test,
                    sk_up<ASTExpression> next, sk_up<ASTStatement> statement)
            : INHERITED(position, kFor_Kind)
            , fInitializer(std::move(initializer))
            , fTest(std::move(test))
            , fNext(std::move(next))
            , fStatement(std::move(statement)) {}

    SkString description() const override {
        SkString result("for (");
        if (fInitializer) {
            result.append(fInitializer->description());
        }
        result += " ";
        if (fTest) {
            result.append(fTest->description());
        }
        result += "; ";
        if (fNext) {
            result.append(fNext->description());
        }
        result += ") ";
        result += fStatement->description();
        return result;
    }

    const sk_up<ASTStatement> fInitializer;
    const sk_up<ASTExpression> fTest;
    const sk_up<ASTExpression> fNext;
    const sk_up<ASTStatement> fStatement;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
