/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALFUNCTIONCALL
#define SKSL_EXTERNALFUNCTIONCALL

#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An external function invocation.
 */
struct ExternalFunctionCall : public Expression {
    static constexpr Kind kExpressionKind = Kind::kExternalFunctionCall;

    ExternalFunctionCall(int offset, const Type* type, const ExternalValue* function,
                         std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(offset, kExpressionKind, ExternalValueData{type, function}) {
        fExpressionChildren = std::move(arguments);
    }

    std::vector<std::unique_ptr<Expression>>& arguments() {
        return fExpressionChildren;
    }

    const std::vector<std::unique_ptr<Expression>>& arguments() const {
        return fExpressionChildren;
    }

    const ExternalValue* function() const {
        return this->externalValueData().fValue;
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects) {
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
        std::vector<std::unique_ptr<Expression>> cloned;
        for (const auto& arg : this->arguments()) {
            cloned.push_back(arg->clone());
        }
        return std::unique_ptr<Expression>(new ExternalFunctionCall(fOffset,
                                                                    &this->type(),
                                                                    this->function(),
                                                                    std::move(cloned)));
    }

    String description() const override {
        String result = String(this->function()->fName) + "(";
        String separator;
        for (const std::unique_ptr<Expression>& arg : this->arguments()) {
            result += separator;
            result += arg->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
