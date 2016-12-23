/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_RETURNSTATEMENT
#define SKSL_RETURNSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
struct ReturnStatement : public Statement {
    ReturnStatement(Position position)
    : INHERITED(position, kReturn_Kind) {}

    ReturnStatement(sk_up<Expression> expression)
            : INHERITED(expression->fPosition, kReturn_Kind), fExpression(std::move(expression)) {}

    SkString description() const override {
        if (fExpression) {
            return "return " + fExpression->description() + ";";
        } else {
            return SkString("return;");
        }
    }

    const sk_up<Expression> fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
