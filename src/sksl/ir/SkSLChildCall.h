/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CHILDCALL
#define SKSL_CHILDCALL

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class Type;
class Variable;
enum class OperatorPrecedence : uint8_t;

/**
 * A call to a child effect object (shader, color filter, or blender).
 */
class ChildCall final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kChildCall;

    ChildCall(Position pos, const Type* type, const Variable* child, ExpressionArray arguments)
            : INHERITED(pos, kIRNodeKind, type)
            , fChild(*child)
            , fArguments(std::move(arguments)) {}

    // Creates the child call; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
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

    std::unique_ptr<Expression> clone(Position pos) const override;

    std::string description(OperatorPrecedence) const override;

private:
    const Variable& fChild;
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
