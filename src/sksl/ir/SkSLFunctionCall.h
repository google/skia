/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "SkSLExpression.h"
#include "SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A function invocation.
 */
struct FunctionCall : public Expression {
    FunctionCall(Position position, const FunctionDeclaration& function,
                 std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(position, kFunctionCall_Kind, function.fReturnType)
    , fFunction(std::move(function))
    , fArguments(std::move(arguments)) {}

    std::string description() const override {
        std::string result = fFunction.fName + "(";
        std::string separator = "";
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    const FunctionDeclaration& fFunction;
    const std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
