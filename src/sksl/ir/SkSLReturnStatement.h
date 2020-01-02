/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RETURNSTATEMENT
#define SKSL_RETURNSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
struct ReturnStatement : public Statement {
    ReturnStatement(int offset)
    : INHERITED(offset, kReturn_Kind) {}

    ReturnStatement(std::unique_ptr<Expression> expression)
    : INHERITED(expression->fOffset, kReturn_Kind)
    , fExpression(std::move(expression)) {}

    std::unique_ptr<Statement> clone() const override {
        if (fExpression) {
            return std::unique_ptr<Statement>(new ReturnStatement(fExpression->clone()));
        }
        return std::unique_ptr<Statement>(new ReturnStatement(fOffset));
    }

#ifdef SK_DEBUG
    String description() const override {
        if (fExpression) {
            return "return " + fExpression->description() + ";";
        } else {
            return String("return;");
        }
    }
#endif

    std::unique_ptr<Expression> fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
