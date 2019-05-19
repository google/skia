/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTCALLSUFFIX
#define SKSL_ASTCALLSUFFIX

#include <vector>
#include "src/sksl/ast/SkSLASTSuffix.h"

namespace SkSL {

/**
 * A parenthesized list of arguments following an expression, indicating a function call.
 */
struct ASTCallSuffix : public ASTSuffix {
    ASTCallSuffix(int offset, std::vector<std::unique_ptr<ASTExpression>> arguments)
    : INHERITED(offset, ASTSuffix::kCall_Kind)
    , fArguments(std::move(arguments)) {}

    String description() const override {
        String result("(");
        String separator;
        for (size_t i = 0; i < fArguments.size(); ++i) {
            result += separator;
            separator = ", ";
            result += fArguments[i]->description();
        }
        result += ")";
        return result;
    }

    std::vector<std::unique_ptr<ASTExpression>> fArguments;

    typedef ASTSuffix INHERITED;
};

} // namespace

#endif
