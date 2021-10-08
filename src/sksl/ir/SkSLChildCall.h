/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CHILDCALL
#define SKSL_CHILDCALL

#include "include/private/SkTArray.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A call to a child effect object (shader, color filter, or blender).
 */
class ChildCall final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kChildCall;

    ChildCall(int line, const Type* type, const Variable* child, ExpressionArray arguments)
            : INHERITED(line, kExpressionKind, type)
            , fChild(*child)
            , fArguments(std::move(arguments)) {}

    // Creates the child call; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type* returnType,
                                            const Variable& child,
                                            ExpressionArray arguments);

    const Variable& child() const {
        return fChild;
    }

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    bool hasProperty(Property property) const override;

    std::unique_ptr<Expression> clone() const override;

    String description() const override;

private:
    const Variable& fChild;
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
