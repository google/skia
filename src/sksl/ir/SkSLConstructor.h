/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents the construction of a compound type, such as "vec2(x, y)".
 */
struct Constructor : public Expression {
    Constructor(Position position, const Type& type,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(position, kConstructor_Kind, type)
    , fArguments(std::move(arguments)) {}

    SkString description() const override {
        SkString result = fType.description() + "(";
        SkString separator;
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isConstant() const override {
        for (size_t i = 0; i < fArguments.size(); i++) {
            if (!fArguments[i]->isConstant()) {
                return false;
            }
        }
        return true;
    }

    const std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
