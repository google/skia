/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALVALUEREFERENCE
#define SKSL_EXTERNALVALUEREFERENCE

#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to an ExternalValue.
 */
class ExternalValueReference : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kExternalValue;

    ExternalValueReference(int offset, const ExternalValue* ev)
    : INHERITED(offset, kExpressionKind, ExternalValueData{&ev->type(), ev}) {}

    const Type& type() const override {
        return *this->externalValueData().fType;
    }

    const ExternalValue& value() const {
        return *this->externalValueData().fValue;
    }

    bool hasProperty(Property property) const override {
        return property == Property::kSideEffects;
    }

    String description() const override {
        return String(this->value().name());
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new ExternalValueReference(fOffset, &this->value()));
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
