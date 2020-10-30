/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "include/private/SkTArray.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A function invocation.
 */
class FunctionCall final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kFunctionCall;

    FunctionCall(int offset, const Type* type, const FunctionDeclaration* function,
                 ExpressionArray arguments)
        : INHERITED(offset, kExpressionKind, type)
        , fFunction(*function)
        , fArguments(std::move(arguments)) {}

    ~FunctionCall() override {}

    const FunctionDeclaration& function() const {
        return fFunction;
    }

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects && (this->function().modifiers().fFlags &
                                                   Modifiers::kHasSideEffects_Flag)) {
            return true;
        }
        for (const auto& arg : this->arguments()) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        ExpressionArray cloned;
        cloned.reserve_back(this->arguments().size());
        for (const auto& arg : this->arguments()) {
            cloned.push_back(arg->clone());
        }
        return std::make_unique<FunctionCall>(fOffset, &this->type(), &this->function(),
                                              std::move(cloned));
    }

    String description() const override {
        String result = String(this->function().name()) + "(";
        String separator;
        for (size_t i = 0; i < this->arguments().size(); i++) {
            result += separator;
            result += this->arguments()[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

private:
    const FunctionDeclaration& fFunction;
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
