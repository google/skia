/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSLEmptyExpression_DEFINED
#define SkSLEmptyExpression_DEFINED

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * The EmptyExpression is a void-typed expression with nothing inside. EmptyExpressions can exist
 * inside an ExpressionStatement; this construct is functionally equivalent to a Nop.
 */
class EmptyExpression : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kEmpty;

    static std::unique_ptr<Expression> Make(Position pos, const Context& context) {
        return std::make_unique<EmptyExpression>(pos, context.fTypes.fVoid.get());
    }

    EmptyExpression(Position pos, const Type* type)
        : INHERITED(pos, kIRNodeKind, type) {
        SkASSERT(type->isVoid());
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<EmptyExpression>(pos, &this->type());
    }

    std::string description(OperatorPrecedence) const override {
        // There's no way in GLSL to directly emit a void-typed expression.
        // `false` is used here as a placeholder expression; the value of a void-typed expression is
        // never meaningful, so this should be a decent substitute.
        return "false";
    }

private:
    using INHERITED = Expression;
};

} // namespace SkSL

#endif  // SkSLEmptyExpression_DEFINED
