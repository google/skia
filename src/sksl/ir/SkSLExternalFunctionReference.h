/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALFUNCTIONREFERENCE
#define SKSL_EXTERNALFUNCTIONREFERENCE

#include "src/sksl/SkSLExternalFunction.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to an ExternalFunction. This is an intermediate value:
 * ExternalFunctionReferences are always eventually replaced by ExternalFunctionCalls in valid
 * programs.
 */
class ExternalFunctionReference final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kExternalFunctionReference;

    ExternalFunctionReference(int offset, const ExternalFunction* ef)
        : INHERITED(offset, kExpressionKind, &ef->type())
        , fFunction(*ef) {}

    const ExternalFunction& function() const {
        return fFunction;
    }

    bool hasProperty(Property property) const override {
        return property == Property::kSideEffects;
    }

    String description() const override {
        return String(this->function().name());
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ExternalFunctionReference>(fOffset, &this->function());
    }

private:
    const ExternalFunction& fFunction;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
