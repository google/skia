/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"

#include <limits>

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

static std::unique_ptr<Expression> short_circuit_boolean(const Expression& left,
                                                         Token::Kind op,
                                                         const Expression& right) {
    SkASSERT(left.is<BoolLiteral>());
    bool leftVal = left.as<BoolLiteral>().value();

    if (op == Token::Kind::TK_LOGICALAND) {
        // (true && expr) -> (expr) and (false && expr) -> (false)
        return leftVal ? right.clone()
                       : std::make_unique<BoolLiteral>(left.fOffset, /*value=*/false, &left.type());
    }
    if (op == Token::Kind::TK_LOGICALOR) {
        // (true || expr) -> (true) and (false || expr) -> (expr)
        return leftVal ? std::make_unique<BoolLiteral>(left.fOffset, /*value=*/true, &left.type())
                       : right.clone();
    }
    if (op == Token::Kind::TK_LOGICALXOR && !leftVal) {
        // (false ^^ expr) -> (expr)
        return right.clone();
    }

    return nullptr;
}

template <typename T>
static std::unique_ptr<Expression> simplify_vector(const Context& context,
                                                   const Expression& left,
                                                   Token::Kind op,
                                                   const Expression& right) {
    SkASSERT(left.type().isVector());
    SkASSERT(left.type() == right.type());
    const Type& type = left.type();

    // Handle boolean operations: == !=
    if (op == Token::Kind::TK_EQEQ || op == Token::Kind::TK_NEQ) {
        bool equality = (op == Token::Kind::TK_EQEQ);

        switch (left.compareConstant(right)) {
            case Expression::ComparisonResult::kNotEqual:
                equality = !equality;
                [[fallthrough]];

            case Expression::ComparisonResult::kEqual:
                return std::make_unique<BoolLiteral>(context, left.fOffset, equality);

            case Expression::ComparisonResult::kUnknown:
                return nullptr;
        }
    }

    // Handle floating-point arithmetic: + - * /
    const auto vectorComponentwiseFold = [&](auto foldFn) -> std::unique_ptr<Constructor> {
        const Type& componentType = type.componentType();
        ExpressionArray args;
        args.reserve_back(type.columns());
        for (int i = 0; i < type.columns(); i++) {
            T value = foldFn(left.getVecComponent<T>(i), right.getVecComponent<T>(i));
            args.push_back(std::make_unique<Literal<T>>(left.fOffset, value, &componentType));
        }
        return std::make_unique<Constructor>(left.fOffset, &type, std::move(args));
    };

    const auto isVectorDivisionByZero = [&]() -> bool {
        for (int i = 0; i < type.columns(); i++) {
            if (right.getVecComponent<T>(i) == 0) {
                return true;
            }
        }
        return false;
    };

    switch (op) {
        case Token::Kind::TK_PLUS:  return vectorComponentwiseFold([](T a, T b) { return a + b; });
        case Token::Kind::TK_MINUS: return vectorComponentwiseFold([](T a, T b) { return a - b; });
        case Token::Kind::TK_STAR:  return vectorComponentwiseFold([](T a, T b) { return a * b; });
        case Token::Kind::TK_SLASH: {
            if (isVectorDivisionByZero()) {
                context.fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            }
            return vectorComponentwiseFold([](T a, T b) { return a / b; });
        }
        default:
            return nullptr;
    }
}

static Constructor splat_scalar(const Expression& scalar, const Type& type) {
    SkASSERT(type.isVector());
    SkASSERT(type.componentType() == scalar.type());

    // Use a Constructor to splat the scalar expression across a vector.
    ExpressionArray arg;
    arg.push_back(scalar.clone());
    return Constructor{scalar.fOffset, &type, std::move(arg)};
}

std::unique_ptr<Expression> ConstantFolder::Simplify(const Context& context,
                                                     const Expression& left,
                                                     Token::Kind op,
                                                     const Expression& right) {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short-circuit optimizations to allow the constant to be folded.
    if (left.is<BoolLiteral>() && !right.isCompileTimeConstant()) {
        return short_circuit_boolean(left, op, right);
    }

    if (right.is<BoolLiteral>() && !left.isCompileTimeConstant()) {
        // There aren't side effects in SkSL within expressions, so (left OP right) is equivalent to
        // (right OP left) for short-circuit optimizations
        // TODO: (true || (a=b)) seems to disqualify the above statement. Test this.
        return short_circuit_boolean(right, op, left);
    }

    // Other than the short-circuit cases above, constant folding requires both sides to be constant
    if (!left.isCompileTimeConstant() || !right.isCompileTimeConstant()) {
        return nullptr;
    }

    // Perform constant folding on pairs of Booleans.
    if (left.is<BoolLiteral>() && right.is<BoolLiteral>()) {
        bool leftVal  = left.as<BoolLiteral>().value();
        bool rightVal = right.as<BoolLiteral>().value();
        bool result;
        switch (op) {
            case Token::Kind::TK_LOGICALAND: result = leftVal && rightVal; break;
            case Token::Kind::TK_LOGICALOR:  result = leftVal || rightVal; break;
            case Token::Kind::TK_LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return nullptr;
        }
        return std::make_unique<BoolLiteral>(context, left.fOffset, result);
    }

    // Note that we expressly do not worry about precision and overflow here -- we use the maximum
    // precision to calculate the results and hope the result makes sense.
    // TODO: detect and handle integer overflow properly.
    #define RESULT(t, op) std::make_unique<t ## Literal>(context, left.fOffset, \
                                                         leftVal op rightVal)
    #define URESULT(t, op) std::make_unique<t ## Literal>(context, left.fOffset, \
                                                          (uint64_t) leftVal op   \
                                                          (uint64_t) rightVal)
    if (left.is<IntLiteral>() && right.is<IntLiteral>()) {
        SKSL_INT leftVal  = left.as<IntLiteral>().value();
        SKSL_INT rightVal = right.as<IntLiteral>().value();
        switch (op) {
            case Token::Kind::TK_PLUS:       return URESULT(Int, +);
            case Token::Kind::TK_MINUS:      return URESULT(Int, -);
            case Token::Kind::TK_STAR:       return URESULT(Int, *);
            case Token::Kind::TK_SLASH:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    context.fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, /);
            case Token::Kind::TK_PERCENT:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    context.fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, %);
            case Token::Kind::TK_BITWISEAND: return RESULT(Int,  &);
            case Token::Kind::TK_BITWISEOR:  return RESULT(Int,  |);
            case Token::Kind::TK_BITWISEXOR: return RESULT(Int,  ^);
            case Token::Kind::TK_EQEQ:       return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:        return RESULT(Bool, !=);
            case Token::Kind::TK_GT:         return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ:       return RESULT(Bool, >=);
            case Token::Kind::TK_LT:         return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ:       return RESULT(Bool, <=);
            case Token::Kind::TK_SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  <<);
                }
                context.fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;
            case Token::Kind::TK_SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  >>);
                }
                context.fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;

            default:
                return nullptr;
        }
    }

    // Perform constant folding on pairs of floating-point literals.
    if (left.is<FloatLiteral>() && right.is<FloatLiteral>()) {
        SKSL_FLOAT leftVal  = left.as<FloatLiteral>().value();
        SKSL_FLOAT rightVal = right.as<FloatLiteral>().value();
        switch (op) {
            case Token::Kind::TK_PLUS:  return RESULT(Float, +);
            case Token::Kind::TK_MINUS: return RESULT(Float, -);
            case Token::Kind::TK_STAR:  return RESULT(Float, *);
            case Token::Kind::TK_SLASH:
                if (rightVal) {
                    return RESULT(Float, /);
                }
                context.fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::Kind::TK_EQEQ: return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:  return RESULT(Bool, !=);
            case Token::Kind::TK_GT:   return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ: return RESULT(Bool, >=);
            case Token::Kind::TK_LT:   return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ: return RESULT(Bool, <=);
            default:                   return nullptr;
        }
    }

    // Perform constant folding on pairs of vectors.
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    if (leftType.isVector() && leftType == rightType) {
        if (leftType.componentType().isFloat()) {
            return simplify_vector<SKSL_FLOAT>(context, left, op, right);
        }
        if (leftType.componentType().isInteger()) {
            return simplify_vector<SKSL_INT>(context, left, op, right);
        }
        return nullptr;
    }

    // Perform constant folding on vectors against scalars, e.g.: half4(2) + 2
    if (leftType.isVector() && leftType.componentType() == rightType) {
        if (rightType.isFloat()) {
            return simplify_vector<SKSL_FLOAT>(context, left, op, splat_scalar(right, left.type()));
        }
        if (rightType.isInteger()) {
            return simplify_vector<SKSL_INT>(context, left, op, splat_scalar(right, left.type()));
        }
        return nullptr;
    }

    // Perform constant folding on scalars against vectors, e.g.: 2 + half4(2)
    if (rightType.isVector() && rightType.componentType() == leftType) {
        if (leftType.isFloat()) {
            return simplify_vector<SKSL_FLOAT>(context, splat_scalar(left, right.type()), op,
                                               right);
        }
        if (leftType.isInteger()) {
            return simplify_vector<SKSL_INT>(context, splat_scalar(left, right.type()), op, right);
        }
        return nullptr;
    }

    // Perform constant folding on pairs of matrices.
    if (leftType.isMatrix() && rightType.isMatrix()) {
        bool equality;
        switch (op) {
            case Token::Kind::TK_EQEQ:
                equality = true;
                break;
            case Token::Kind::TK_NEQ:
                equality = false;
                break;
            default:
                return nullptr;
        }

        switch (left.compareConstant(right)) {
            case Expression::ComparisonResult::kNotEqual:
                equality = !equality;
                [[fallthrough]];

            case Expression::ComparisonResult::kEqual:
                return std::make_unique<BoolLiteral>(context, left.fOffset, equality);

            case Expression::ComparisonResult::kUnknown:
                return nullptr;
        }
    }

    // We aren't able to constant-fold.
    #undef RESULT
    #undef URESULT
    return nullptr;
}

}  // namespace SkSL
