/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * An 'if' statement.
 */
struct IfStatement : public Statement {
    IfStatement(int offset, bool isStatic, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
    : INHERITED(offset, kIf_Kind)
    , fIsStatic(isStatic)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {}

    int nodeCount() const override {
        return 1 + fTest->nodeCount() + fIfTrue->nodeCount() +
               (fIfFalse ? fIfFalse->nodeCount() : 0);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new IfStatement(fOffset, fIsStatic, fTest->clone(),
                fIfTrue->clone(), fIfFalse ? fIfFalse->clone() : nullptr));
    }

#ifdef SKSL_STANDALONE
    String constructionCode() const override {
        return String::printf("new IfStatement(%d, %d, std::unique_ptr<Expression>(%s), "
                              "std::unique_ptr<Statement>(%s), std::unique_ptr<Statement>(%s))",
                              fOffset, fIsStatic, fTest->constructionCode().c_str(),
                              fIfTrue->constructionCode().c_str(),
                              fIfFalse ? fIfFalse->constructionCode().c_str() : "nullptr");
    }
#endif

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
