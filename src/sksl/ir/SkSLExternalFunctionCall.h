/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALFUNCTIONCALL
#define SKSL_EXTERNALFUNCTIONCALL

#include "include/private/SkTArray.h"
#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An external function invocation.
 */
class ExternalFunctionCall : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kExternalFunctionCall;

    ExternalFunctionCall(int offset, const ExternalValue* function, ExpressionArray arguments)
    : INHERITED(offset, kExpressionKind, ExternalValueData{&function->callReturnType(), function}) {
        fExpressionChildren = std::move(arguments);
    }

    const Type& type() const override {
        return *this->externalValueData().fType;
    }

    ExpressionArray& arguments() {
        return fExpressionChildren;
    }

    const ExpressionArray& arguments() const {
        return fExpressionChildren;
    }

    const ExternalValue& function() const {
        return *this->externalValueData().fValue;
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
        ExpressionArray cloned;
        cloned.reserve_back(this->arguments().size());
        for (const auto& arg : this->arguments()) {
            cloned.push_back(arg->clone());
        }
        return std::make_unique<ExternalFunctionCall>(fOffset, &this->function(),
                                                      std::move(cloned));
    }

    String description() const override {
        String result = String(this->function().name()) + "(";
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
