/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"

namespace SkSL {

/**
 * Used as a sentinel expression during dataflow analysis, when an exact value for a variable can't
 * be determined at compile-time.
 */
class DefinedExpression final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kDefined;

    DefinedExpression(const Type* type)
    : INHERITED(/*offset=*/-1, kExpressionKind, type) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    String description() const override {
        return "<defined>";
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<DefinedExpression>(&this->type());
    }

    using INHERITED = Expression;
};

Context::Context(ErrorReporter& errors, const ShaderCapsClass& caps)
        : fErrors(errors)
        , fCaps(caps)
        , fDefined_Expression(std::make_unique<DefinedExpression>(fTypes.fInvalid.get())) {}

}  // namespace SkSL

