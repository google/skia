/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * An 'if' statement.
 */
struct IfStatement : public Statement {
    IfStatement(Position position, bool isStatic, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
    : INHERITED(position, kIf_Kind)
    , fIsStatic(isStatic)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {}

    String description() const override {
        String result;
        if (fIsStatic) {
            result += "@";
        }
        result += "if (" + fTest->description() + ") " + fIfTrue->description();
        if (fIfFalse) {
            result += " else " + fIfFalse->description();
        }
        return result;
    }

    bool fIsStatic;
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Statement> fIfTrue;
    // may be null
    std::unique_ptr<Statement> fIfFalse;

    typedef Statement INHERITED;
};

} // namespace

#endif
