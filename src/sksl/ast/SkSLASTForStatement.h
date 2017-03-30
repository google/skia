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
    ASTForStatement(Position position, std::unique_ptr<ASTStatement> initializer,
                   std::unique_ptr<ASTExpression> test, std::unique_ptr<ASTExpression> next,
                   std::unique_ptr<ASTStatement> statement)
    : INHERITED(position, kFor_Kind)
    , fInitializer(std::move(initializer))
    , fTest(std::move(test))
    , fNext(std::move(next))
    , fStatement(std::move(statement)) {}

    String description() const override {
        String result("for (");
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

    const std::unique_ptr<ASTStatement> fInitializer;
    const std::unique_ptr<ASTExpression> fTest;
    const std::unique_ptr<ASTExpression> fNext;
    const std::unique_ptr<ASTStatement> fStatement;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
