/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"

#include <limits>

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

static std::unique_ptr<Expression> eliminate_no_op_boolean(const Expression& left,
                                                           Operator op,
                                                           const Expression& right) {
    bool rightVal = right.as<Literal>().boolValue();

    // Detect no-op Boolean expressions and optimize them away.
    if ((op.kind() == Token::Kind::TK_LOGICALAND && rightVal)  ||  // (expr && true)  -> (expr)
        (op.kind() == Token::Kind::TK_LOGICALOR  && !rightVal) ||  // (expr || false) -> (expr)
        (op.kind() == Token::Kind::TK_LOGICALXOR && !rightVal) ||  // (expr ^^ false) -> (expr)
        (op.kind() == Token::Kind::TK_EQEQ       && rightVal)  ||  // (expr == true)  -> (expr)
        (op.kind() == Token::Kind::TK_NEQ        && !rightVal)) {  // (expr != false) -> (expr)

        return left.clone();
    }

    return nullptr;
}

static std::unique_ptr<Expression> short_circuit_boolean(const Expression& left,
                                                         Operator op,
                                                         const Expression& right) {
    bool leftVal = left.as<Literal>().boolValue();

    // When the literal is on the left, we can sometimes eliminate the other expression entirely.
    if ((op.kind() == Token::Kind::TK_LOGICALAND && !leftVal) ||  // (false && expr) -> (false)
        (op.kind() == Token::Kind::TK_LOGICALOR  && leftVal)) {   // (true  || expr) -> (true)

        return left.clone();
    }

    // We can't eliminate the right-side expression via short-circuit, but we might still be able to
    // simplify away a no-op expression.
    return eliminate_no_op_boolean(right, op, left);
}

static std::unique_ptr<Expression> simplify_constant_equality(const Context& context,
                                                              const Expression& left,
                                                              Operator op,
                                                              const Expression& right) {
    if (op.kind() == Token::Kind::TK_EQEQ || op.kind() == Token::Kind::TK_NEQ) {
        bool equality = (op.kind() == Token::Kind::TK_EQEQ);

        switch (left.compareConstant(right)) {
            case Expression::ComparisonResult::kNotEqual:
                equality = !equality;
                [[fallthrough]];

            case Expression::ComparisonResult::kEqual:
                return Literal::MakeBool(context, left.fLine, equality);

            case Expression::ComparisonResult::kUnknown:
                break;
        }
    }
    return nullptr;
}

static std::unique_ptr<Expression> simplify_vector(const Context& context,
                                                   const Expression& left,
                                                   Operator op,
                                                   const Expression& right) {
    SkASSERT(left.type().isVector());
    SkASSERT(left.type().matches(right.type()));
    const Type& type = left.type();

    // Handle equality operations: == !=
    if (std::unique_ptr<Expression> result = simplify_constant_equality(context, left, op, right)) {
        return result;
    }

    // Handle floating-point arithmetic: + - * /
    using FoldFn = double (*)(double, double);
    FoldFn foldFn;
    switch (op.kind()) {
        case Token::Kind::TK_PLUS:  foldFn = +[](double a, double b) { return a + b; }; break;
        case Token::Kind::TK_MINUS: foldFn = +[](double a, double b) { return a - b; }; break;
        case Token::Kind::TK_STAR:  foldFn = +[](double a, double b) { return a * b; }; break;
        case Token::Kind::TK_SLASH: foldFn = +[](double a, double b) { return a / b; }; break;
        default:
            return nullptr;
    }

    const Type& componentType = type.componentType();
    double minimumValue = -INFINITY, maximumValue = INFINITY;
    if (componentType.isInteger()) {
        minimumValue = componentType.minimumValue();
        maximumValue = componentType.maximumValue();
    }

    ExpressionArray args;
    args.reserve_back(type.columns());
    for (int i = 0; i < type.columns(); i++) {
        double value = foldFn(*left.getConstantValue(i), *right.getConstantValue(i));
        if (value < minimumValue || value > maximumValue) {
            return nullptr;
        }

        args.push_back(Literal::Make(left.fLine, value, &componentType));
    }
    return ConstructorCompound::Make(context, left.fLine, type, std::move(args));
}

static std::unique_ptr<Expression> cast_expression(const Context& context,
                                                   const Expression& expr,
                                                   const Type& type) {
    ExpressionArray ctorArgs;
    ctorArgs.push_back(expr.clone());
    return Constructor::Convert(context, expr.fLine, type, std::move(ctorArgs));
}

bool ConstantFolder::GetConstantInt(const Expression& value, SKSL_INT* out) {
    const Expression* expr = GetConstantValueForVariable(value);
    if (!expr->isIntLiteral()) {
        return false;
    }
    *out = expr->as<Literal>().intValue();
    return true;
}

bool ConstantFolder::GetConstantValue(const Expression& value, double* out) {
    const Expression* expr = GetConstantValueForVariable(value);
    if (!expr->is<Literal>()) {
        return false;
    }
    *out = expr->as<Literal>().value();
    return true;
}

static bool contains_constant_zero(const Expression& expr) {
    int numSlots = expr.type().slotCount();
    for (int index = 0; index < numSlots; ++index) {
        skstd::optional<double> slotVal = expr.getConstantValue(index);
        if (slotVal.has_value() && *slotVal == 0.0) {
            return true;
        }
    }
    return false;
}

static bool is_constant_value(const Expression& expr, double value) {
    int numSlots = expr.type().slotCount();
    for (int index = 0; index < numSlots; ++index) {
        skstd::optional<double> slotVal = expr.getConstantValue(index);
        if (!slotVal.has_value() || *slotVal != value) {
            return false;
        }
    }
    return true;
}

static bool error_on_divide_by_zero(const Context& context, int line, Operator op,
                                    const Expression& right) {
    switch (op.kind()) {
        case Token::Kind::TK_SLASH:
        case Token::Kind::TK_SLASHEQ:
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_PERCENTEQ:
            if (contains_constant_zero(right)) {
                context.fErrors->error(line, "division by zero");
                return true;
            }
            return false;
        default:
            return false;
    }
}

const Expression* ConstantFolder::GetConstantValueForVariable(const Expression& inExpr) {
    for (const Expression* expr = &inExpr;;) {
        if (!expr->is<VariableReference>()) {
            break;
        }
        const VariableReference& varRef = expr->as<VariableReference>();
        if (varRef.refKind() != VariableRefKind::kRead) {
            break;
        }
        const Variable& var = *varRef.variable();
        if (!(var.modifiers().fFlags & Modifiers::kConst_Flag)) {
            break;
        }
        expr = var.initialValue();
        if (!expr) {
            // Function parameters can be const but won't have an initial value.
            break;
        }
        if (expr->isCompileTimeConstant()) {
            return expr;
        }
    }
    // We didn't find a compile-time constant at the end. Return the expression as-is.
    return &inExpr;
}

std::unique_ptr<Expression> ConstantFolder::MakeConstantValueForVariable(
        std::unique_ptr<Expression> expr) {
    const Expression* constantExpr = GetConstantValueForVariable(*expr);
    if (constantExpr != expr.get()) {
        expr = constantExpr->clone();
    }
    return expr;
}

static std::unique_ptr<Expression> simplify_no_op_arithmetic(const Context& context,
                                                             const Expression& left,
                                                             Operator op,
                                                             const Expression& right,
                                                             const Type& resultType) {
    switch (op.kind()) {
        case Token::Kind::TK_PLUS:
            if (is_constant_value(right, 0.0)) {  // x + 0
                return cast_expression(context, left, resultType);
            }
            if (is_constant_value(left, 0.0)) {   // 0 + x
                return cast_expression(context, right, resultType);
            }
            break;

        case Token::Kind::TK_STAR:
            if (is_constant_value(right, 1.0)) {  // x * 1
                return cast_expression(context, left, resultType);
            }
            if (is_constant_value(left, 1.0)) {   // 1 * x
                return cast_expression(context, right, resultType);
            }
            if (is_constant_value(right, 0.0) && !left.hasSideEffects()) {  // x * 0
                return cast_expression(context, right, resultType);
            }
            if (is_constant_value(left, 0.0) && !right.hasSideEffects()) {  // 0 * x
                return cast_expression(context, left, resultType);
            }
            break;

        case Token::Kind::TK_MINUS:
            if (is_constant_value(right, 0.0)) {  // x - 0
                return cast_expression(context, left, resultType);
            }
            if (is_constant_value(left, 0.0)) {   // 0 - x (to `-x`)
                if (std::unique_ptr<Expression> val = cast_expression(context, right, resultType)) {
                    return PrefixExpression::Make(context, Token::Kind::TK_MINUS, std::move(val));
                }
            }
            break;

        case Token::Kind::TK_SLASH:
            if (is_constant_value(right, 1.0)) {  // x / 1
                return cast_expression(context, left, resultType);
            }
            break;

        case Token::Kind::TK_PLUSEQ:
        case Token::Kind::TK_MINUSEQ:
            if (is_constant_value(right, 0.0)) {  // x += 0, x -= 0
                if (std::unique_ptr<Expression> var = cast_expression(context, left, resultType)) {
                    Analysis::UpdateVariableRefKind(var.get(), VariableRefKind::kRead);
                    return var;
                }
            }
            break;

        case Token::Kind::TK_STAREQ:
        case Token::Kind::TK_SLASHEQ:
            if (is_constant_value(right, 1.0)) {  // x *= 1, x /= 1
                if (std::unique_ptr<Expression> var = cast_expression(context, left, resultType)) {
                    Analysis::UpdateVariableRefKind(var.get(), VariableRefKind::kRead);
                    return var;
                }
            }
            break;

        default:
            break;
    }

    return nullptr;
}

template <typename T>
static std::unique_ptr<Expression> fold_float_expression(int line,
                                                         T result,
                                                         const Type* resultType) {
    // If constant-folding this expression would generate a NaN/infinite result, leave it as-is.
    if constexpr (!std::is_same<T, bool>::value) {
        if (!std::isfinite(result)) {
            return nullptr;
        }
    }

    return Literal::Make(line, result, resultType);
}

template <typename T>
static std::unique_ptr<Expression> fold_int_expression(int line,
                                                       T result,
                                                       const Type* resultType) {
    // If constant-folding this expression would overflow the result type, leave it as-is.
    if constexpr (!std::is_same<T, bool>::value) {
        if (result < resultType->minimumValue() || result > resultType->maximumValue()) {
            return nullptr;
        }
    }

    return Literal::Make(line, result, resultType);
}

std::unique_ptr<Expression> ConstantFolder::Simplify(const Context& context,
                                                     int line,
                                                     const Expression& leftExpr,
                                                     Operator op,
                                                     const Expression& rightExpr,
                                                     const Type& resultType) {
    // Replace constant variables with their literal values.
    const Expression* left = GetConstantValueForVariable(leftExpr);
    const Expression* right = GetConstantValueForVariable(rightExpr);

    // If this is the comma operator, the left side is evaluated but not otherwise used in any way.
    // So if the left side has no side effects, it can just be eliminated entirely.
    if (op.kind() == Token::Kind::TK_COMMA && !left->hasSideEffects()) {
        return right->clone();
    }

    // If this is the assignment operator, and both sides are the same trivial expression, this is
    // self-assignment (i.e., `var = var`) and can be reduced to just a variable reference (`var`).
    // This can happen when other parts of the assignment are optimized away.
    if (op.kind() == Token::Kind::TK_EQ && Analysis::IsSameExpressionTree(*left, *right)) {
        return right->clone();
    }

    // Simplify the expression when both sides are constant Boolean literals.
    if (left->isBoolLiteral() && right->isBoolLiteral()) {
        bool leftVal  = left->as<Literal>().boolValue();
        bool rightVal = right->as<Literal>().boolValue();
        bool result;
        switch (op.kind()) {
            case Token::Kind::TK_LOGICALAND: result = leftVal && rightVal; break;
            case Token::Kind::TK_LOGICALOR:  result = leftVal || rightVal; break;
            case Token::Kind::TK_LOGICALXOR: result = leftVal ^  rightVal; break;
            case Token::Kind::TK_EQEQ:       result = leftVal == rightVal; break;
            case Token::Kind::TK_NEQ:        result = leftVal != rightVal; break;
            default: return nullptr;
        }
        return Literal::MakeBool(context, line, result);
    }

    // If the left side is a Boolean literal, apply short-circuit optimizations.
    if (left->isBoolLiteral()) {
        return short_circuit_boolean(*left, op, *right);
    }

    // If the right side is a Boolean literal...
    if (right->isBoolLiteral()) {
        // ... and the left side has no side effects...
        if (!left->hasSideEffects()) {
            // We can reverse the expressions and short-circuit optimizations are still valid.
            return short_circuit_boolean(*right, op, *left);
        }

        // We can't use short-circuiting, but we can still optimize away no-op Boolean expressions.
        return eliminate_no_op_boolean(*left, op, *right);
    }

    if (op.kind() == Token::Kind::TK_EQEQ && Analysis::IsSameExpressionTree(*left, *right)) {
        // With == comparison, if both sides are the same trivial expression, this is self-
        // comparison and is always true. (We are not concerned with NaN.)
        return Literal::MakeBool(context, leftExpr.fLine, /*value=*/true);
    }

    if (op.kind() == Token::Kind::TK_NEQ && Analysis::IsSameExpressionTree(*left, *right)) {
        // With != comparison, if both sides are the same trivial expression, this is self-
        // comparison and is always false. (We are not concerned with NaN.)
        return Literal::MakeBool(context, leftExpr.fLine, /*value=*/false);
    }

    if (error_on_divide_by_zero(context, line, op, *right)) {
        return nullptr;
    }

    // Optimize away no-op arithmetic like `x * 1`, `x *= 1`, `x + 0`, `x * 0`, `0 / x`, etc.
    const Type& leftType = left->type();
    const Type& rightType = right->type();
    if ((leftType.isScalar() || leftType.isVector()) &&
        (rightType.isScalar() || rightType.isVector())) {
        std::unique_ptr<Expression> expr = simplify_no_op_arithmetic(context, *left, op, *right,
                                                                     resultType);
        if (expr) {
            return expr;
        }
    }

    // Other than the cases above, constant folding requires both sides to be constant.
    if (!left->isCompileTimeConstant() || !right->isCompileTimeConstant()) {
        return nullptr;
    }

    // Note that fold_int_expression returns null if the result would overflow its type.
    using SKSL_UINT = uint64_t;
    if (left->isIntLiteral() && right->isIntLiteral()) {
        SKSL_INT leftVal  = left->as<Literal>().intValue();
        SKSL_INT rightVal = right->as<Literal>().intValue();

        #define RESULT(Op)   fold_int_expression(line, \
                                        (SKSL_INT)(leftVal) Op (SKSL_INT)(rightVal), &resultType)
        #define URESULT(Op)  fold_int_expression(line, \
                             (SKSL_INT)((SKSL_UINT)(leftVal) Op (SKSL_UINT)(rightVal)), &resultType)
        switch (op.kind()) {
            case Token::Kind::TK_PLUS:       return URESULT(+);
            case Token::Kind::TK_MINUS:      return URESULT(-);
            case Token::Kind::TK_STAR:       return URESULT(*);
            case Token::Kind::TK_SLASH:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors->error(line, "arithmetic overflow");
                    return nullptr;
                }
                return RESULT(/);
            case Token::Kind::TK_PERCENT:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors->error(line, "arithmetic overflow");
                    return nullptr;
                }
                return RESULT(%);
            case Token::Kind::TK_BITWISEAND: return RESULT(&);
            case Token::Kind::TK_BITWISEOR:  return RESULT(|);
            case Token::Kind::TK_BITWISEXOR: return RESULT(^);
            case Token::Kind::TK_EQEQ:       return RESULT(==);
            case Token::Kind::TK_NEQ:        return RESULT(!=);
            case Token::Kind::TK_GT:         return RESULT(>);
            case Token::Kind::TK_GTEQ:       return RESULT(>=);
            case Token::Kind::TK_LT:         return RESULT(<);
            case Token::Kind::TK_LTEQ:       return RESULT(<=);
            case Token::Kind::TK_SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    // Left-shifting a negative (or really, any signed) value is undefined behavior
                    // in C++, but not GLSL. Do the shift on unsigned values, to avoid UBSAN.
                    return URESULT(<<);
                }
                context.fErrors->error(line, "shift value out of range");
                return nullptr;
            case Token::Kind::TK_SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(>>);
                }
                context.fErrors->error(line, "shift value out of range");
                return nullptr;

            default:
                return nullptr;
        }
        #undef RESULT
        #undef URESULT
    }

    // Perform constant folding on pairs of floating-point literals.
    if (left->isFloatLiteral() && right->isFloatLiteral()) {
        SKSL_FLOAT leftVal  = left->as<Literal>().floatValue();
        SKSL_FLOAT rightVal = right->as<Literal>().floatValue();

        #define RESULT(Op) fold_float_expression(line, leftVal Op rightVal, &resultType)
        switch (op.kind()) {
            case Token::Kind::TK_PLUS:  return RESULT(+);
            case Token::Kind::TK_MINUS: return RESULT(-);
            case Token::Kind::TK_STAR:  return RESULT(*);
            case Token::Kind::TK_SLASH: return RESULT(/);
            case Token::Kind::TK_EQEQ:  return RESULT(==);
            case Token::Kind::TK_NEQ:   return RESULT(!=);
            case Token::Kind::TK_GT:    return RESULT(>);
            case Token::Kind::TK_GTEQ:  return RESULT(>=);
            case Token::Kind::TK_LT:    return RESULT(<);
            case Token::Kind::TK_LTEQ:  return RESULT(<=);
            default:                    return nullptr;
        }
        #undef RESULT
    }

    // Perform constant folding on pairs of vectors.
    if (leftType.isVector() && leftType.matches(rightType)) {
        if (leftType.componentType().isNumber()) {
            return simplify_vector(context, *left, op, *right);
        }
        if (leftType.componentType().isBoolean()) {
            return simplify_constant_equality(context, *left, op, *right);
        }
        return nullptr;
    }

    // Perform constant folding on vectors against scalars, e.g.: half4(2) + 2
    if (leftType.isVector() && leftType.componentType().matches(rightType)) {
        if (rightType.isNumber()) {
            return simplify_vector(context, *left, op, ConstructorSplat(*right, left->type()));
        }
        if (rightType.isBoolean()) {
            return simplify_constant_equality(context, *left, op,
                                              ConstructorSplat(*right, left->type()));
        }
        return nullptr;
    }

    // Perform constant folding on scalars against vectors, e.g.: 2 + half4(2)
    if (rightType.isVector() && rightType.componentType().matches(leftType)) {
        if (leftType.isNumber()) {
            return simplify_vector(context, ConstructorSplat(*left, right->type()), op, *right);
        }
        if (leftType.isBoolean()) {
            return simplify_constant_equality(context, ConstructorSplat(*left, right->type()),
                                              op, *right);
        }
        return nullptr;
    }

    // Perform constant folding on pairs of matrices or arrays.
    if ((leftType.isMatrix() && rightType.isMatrix()) ||
        (leftType.isArray() && rightType.isArray())) {
        return simplify_constant_equality(context, *left, op, *right);
    }

    // We aren't able to constant-fold.
    return nullptr;
}

}  // namespace SkSL
