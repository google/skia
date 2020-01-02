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
    ExternalFunctionCall(int offset, const Type& type, ExternalValue* function,
                         std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(offset, kExternalFunctionCall_Kind, type)
    , fFunction(function)
    , fArguments(std::move(arguments)) {}

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects) {
            return true;
        }
        for (const auto& arg : fArguments) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<Expression>> cloned;
        for (const auto& arg : fArguments) {
            cloned.push_back(arg->clone());
        }
        return std::unique_ptr<Expression>(new ExternalFunctionCall(fOffset,
                                                                    fType,
                                                                    fFunction,
                                                                    std::move(cloned)));
    }

#ifdef SK_DEBUG
    String description() const override {
        String result = String(fFunction->fName) + "(";
        String separator;
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }
#endif

    ExternalValue* fFunction;
    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
