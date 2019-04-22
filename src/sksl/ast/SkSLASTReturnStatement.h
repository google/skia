/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTRETURNSTATEMENT
#define SKSL_ASTRETURNSTATEMENT

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
struct ASTReturnStatement : public ASTStatement {
    // expression may be null
    ASTReturnStatement(int offset, std::unique_ptr<ASTExpression> expression)
    : INHERITED(offset, kReturn_Kind)
    , fExpression(std::move(expression)) {}

    String description() const override {
        String result("return");
        if (fExpression) {
            result += " " + fExpression->description();
        }
        return result + ";";
    }

    const std::unique_ptr<ASTExpression> fExpression;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
