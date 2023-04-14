/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BINARYEXPRESSION
#define SKSL_BINARYEXPRESSION

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class Type;
class VariableReference;

/**
 * A binary operation.
 */
class BinaryExpression final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kBinary;

    BinaryExpression(Position pos, std::unique_ptr<Expression> left, Operator op,
                     std::unique_ptr<Expression> right, const Type* type)
        : INHERITED(pos, kIRNodeKind, type)
        , fLeft(std::move(left))
        , fOperator(op)
        , fRight(std::move(right)) {
        // If we are assigning to a VariableReference, ensure that it is set to Write or ReadWrite.
        SkASSERT(!op.isAssignment() || CheckRef(*this->left()));
    }

    // Creates a potentially-simplified form of the expression. Determines the result type
    // programmatically. Typechecks and coerces input expressions; reports errors via ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> left,
                                               Operator op,
                                               std::unique_ptr<Expression> right);

    // Creates a potentially-simplified form of the expression. Determines the result type
    // programmatically. Asserts if the expressions do not typecheck or are otherwise invalid.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> left,
                                            Operator op,
                                            std::unique_ptr<Expression> right);

    // Creates a potentially-simplified form of the expression. Result type is passed in.
    // Asserts if the expressions do not typecheck or are otherwise invalid.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> left,
                                            Operator op,
                                            std::unique_ptr<Expression> right,
                                            const Type* resultType);

    std::unique_ptr<Expression>& left() {
        return fLeft;
    }

    const std::unique_ptr<Expression>& left() const {
        return fLeft;
    }

    std::unique_ptr<Expression>& right() {
        return fRight;
    }

    const std::unique_ptr<Expression>& right() const {
        return fRight;
    }

    Operator getOperator() const {
        return fOperator;
    }

    std::unique_ptr<Expression> clone(Position pos) const override;

    std::string description(OperatorPrecedence parentPrecedence) const override;

    /**
     * If the expression is an assignment like `a = 1` or `a += sin(b)`, returns the
     * VariableReference that will be written to. For other types of expressions, returns null.
     * Complex expressions that contain inner assignments, like `(a = b) * 2`, will return null.
     */
    VariableReference* isAssignmentIntoVariable();

private:
    static bool CheckRef(const Expression& expr);

    std::unique_ptr<Expression> fLeft;
    Operator fOperator;
    std::unique_ptr<Expression> fRight;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
