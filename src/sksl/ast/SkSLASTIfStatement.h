/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTIFSTATEMENT
#define SKSL_ASTIFSTATEMENT

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * An 'if' statement.
 */
struct ASTIfStatement : public ASTStatement {
    ASTIfStatement(int offset, bool isStatic, std::unique_ptr<ASTExpression> test,
                   std::unique_ptr<ASTStatement> ifTrue, std::unique_ptr<ASTStatement> ifFalse)
    : INHERITED(offset, kIf_Kind)
    , fIsStatic(isStatic)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {}

    String description() const override {
        String result;
        if (fIsStatic) {
            result += "@";
        }
        result += "if (";
        result += fTest->description();
        result += ") ";
        result += fIfTrue->description();
        if (fIfFalse) {
            result += " else ";
            result += fIfFalse->description();
        }
        return result;
    }

    const bool fIsStatic;
    const std::unique_ptr<ASTExpression> fTest;
    const std::unique_ptr<ASTStatement> fIfTrue;
    const std::unique_ptr<ASTStatement> fIfFalse;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
