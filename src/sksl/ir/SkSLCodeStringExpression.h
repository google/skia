/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CODESTRINGEXPRESSION
#define SKSL_CODESTRINGEXPRESSION

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents a literal string of SkSL code. This is only valid within SkSL DSL code.
 * TODO(skia:11330): This class is intended as a temporary measure to support a couple of spots
 * within Skia that are currently generating raw strings of code. These will eventually transition
 * to producing Expressions, allowing this class to be deleted.
 */
class CodeStringExpression final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kCodeString;

    CodeStringExpression(String code, const Type* type)
        : INHERITED(/*line=*/-1, kExpressionKind, type)
        , fCode(std::move(code)) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<CodeStringExpression>(fCode, &this->type());
    }

    String description() const override {
        return fCode;
    }

private:
    String fCode;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
