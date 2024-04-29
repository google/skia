/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <cstdint>
#include <float.h>
#include <limits>
#include <optional>
#include <string>
#include <utility>

using namespace skia_private;

namespace SkSL {

static bool is_vec_or_mat(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kMatrix:
        case Type::TypeKind::kVector:
            return true;

        default:
            return false;
    }
}

static std::unique_ptr<Expression> eliminate_no_op_boolean(Position pos,
                                                           const Expression& left,
                                                           Operator op,
                                                           const Expression& right) {
    bool rightVal = right.as<Literal>().boolValue();

    // Detect no-op Boolean expressions and optimize them away.
    if ((op.kind() == Operator::Kind::LOGICALAND && rightVal)  ||  // (expr && true)  -> (expr)
        (op.kind() == Operator::Kind::LOGICALOR  && !rightVal) ||  // (expr || false) -> (expr)
        (op.kind() == Operator::Kind::LOGICALXOR && !rightVal) ||  // (expr ^^ false) -> (expr)
        (op.kind() == Operator::Kind::EQEQ       && rightVal)  ||  // (expr == true)  -> (expr)
        (op.kind() == Operator::Kind::NEQ        && !rightVal)) {  // (expr != false) -> (expr)

        return left.clone(pos);
    }

    return nullptr;
}

static std::unique_ptr<Expression> short_circuit_boolean(Position pos,
                                                         const Expression& left,
                                                         Operator op,
                                                         const Expression& right) {
    bool leftVal = left.as<Literal>().boolValue();

    // When the literal is on the left, we can sometimes eliminate the other expression entirely.
    if ((op.kind() == Operator::Kind::LOGICALAND && !leftVal) ||  // (false && expr) -> (false)
        (op.kind() == Operator::Kind::LOGICALOR  && leftVal)) {   // (true  || expr) -> (true)

        return left.clone(pos);
    }

    // We can't eliminate the right-side expression via short-circuit, but we might still be able to
    // simplify away a no-op expression.
    return eliminate_no_op_boolean(pos, right, op, left);
}

static std::unique_ptr<Expression> simplify_constant_equality(const Context& context,
                                                              Position pos,
                                                              const Expression& left,
                                                              Operator op,
                                                              const Expression& right) {
    if (op.kind() == Operator::Kind::EQEQ || op.kind() == Operator::Kind::NEQ) {
        bool equality = (op.kind() == Operator::Kind::EQEQ);

        switch (left.compareConstant(right)) {
            case Expression::ComparisonResult::kNotEqual:
                equality = !equality;
                [[fallthrough]];

            case Expression::ComparisonResult::kEqual:
                return Literal::MakeBool(context, pos, equality);

            case Expression::ComparisonResult::kUnknown:
                break;
        }
    }
    return nullptr;
}

static std::unique_ptr<Expression> simplify_matrix_multiplication(const Context& context,
                                                                  Position pos,
                                                                  const Expression& left,
                                                                  const Expression& right,
                                                                  int leftColumns,
                                                                  int leftRows,
                                                                  int rightColumns,
                                                                  int rightRows) {
    const Type& componentType = left.type().componentType();
    SkASSERT(componentType.matches(right.type().componentType()));

    // Fetch the left matrix.
    double leftVals[4][4];
    for (int c = 0; c < leftColumns; ++c) {
        for (int r = 0; r < leftRows; ++r) {
            leftVals[c][r] = *left.getConstantValue((c * leftRows) + r);
        }
    }
    // Fetch the right matrix.
    double rightVals[4][4];
    for (int c = 0; c < rightColumns; ++c) {
        for (int r = 0; r < rightRows; ++r) {
            rightVals[c][r] = *right.getConstantValue((c * rightRows) + r);
        }
    }

    SkASSERT(leftColumns == rightRows);
    int outColumns   = rightColumns,
        outRows      = leftRows;

    double args[16];
    int argIndex = 0;
    for (int c = 0; c < outColumns; ++c) {
        for (int r = 0; r < outRows; ++r) {
            // Compute a dot product for this position.
            double val = 0;
            for (int dotIdx = 0; dotIdx < leftColumns; ++dotIdx) {
                val += leftVals[dotIdx][r] * rightVals[c][dotIdx];
            }

            if (val >= -FLT_MAX && val <= FLT_MAX) {
                args[argIndex++] = val;
            } else {
                // The value is outside the 32-bit float range, or is NaN; do not optimize.
                return nullptr;
            }
        }
    }

    if (outColumns == 1) {
        // Matrix-times-vector conceptually makes a 1-column N-row matrix, but we return vecN.
        std::swap(outColumns, outRows);
    }

    const Type& resultType = componentType.toCompound(context, outColumns, outRows);
    return ConstructorCompound::MakeFromConstants(context, pos, resultType, args);
}

static std::unique_ptr<Expression> simplify_matrix_times_matrix(const Context& context,
                                                                Position pos,
                                                                const Expression& left,
                                                                const Expression& right) {
    const Type& leftType = left.type();
    const Type& rightType = right.type();

    SkASSERT(leftType.isMatrix());
    SkASSERT(rightType.isMatrix());

    return simplify_matrix_multiplication(context, pos, left, right,
                                          leftType.columns(), leftType.rows(),
                                          rightType.columns(), rightType.rows());
}

static std::unique_ptr<Expression> simplify_vector_times_matrix(const Context& context,
                                                                Position pos,
                                                                const Expression& left,
                                                                const Expression& right) {
    const Type& leftType = left.type();
    const Type& rightType = right.type();

    SkASSERT(leftType.isVector());
    SkASSERT(rightType.isMatrix());

    return simplify_matrix_multiplication(context, pos, left, right,
                                          /*leftColumns=*/leftType.columns(), /*leftRows=*/1,
                                          rightType.columns(), rightType.rows());
}

static std::unique_ptr<Expression> simplify_matrix_times_vector(const Context& context,
                                                                Position pos,
                                                                const Expression& left,
                                                                const Expression& right) {
    const Type& leftType = left.type();
    const Type& rightType = right.type();

    SkASSERT(leftType.isMatrix());
    SkASSERT(rightType.isVector());

    return simplify_matrix_multiplication(context, pos, left, right,
                                          leftType.columns(), leftType.rows(),
                                          /*rightColumns=*/1, /*rightRows=*/rightType.columns());
}

static std::unique_ptr<Expression> simplify_componentwise(const Context& context,
                                                          Position pos,
                                                          const Expression& left,
                                                          Operator op,
                                                          const Expression& right) {
    SkASSERT(is_vec_or_mat(left.type()));
    SkASSERT(left.type().matches(right.type()));
    const Type& type = left.type();

    // Handle equality operations: == !=
    if (std::unique_ptr<Expression> result = simplify_constant_equality(context, pos, left, op,
            right)) {
        return result;
    }

    // Handle floating-point arithmetic: + - * /
    using FoldFn = double (*)(double, double);
    FoldFn foldFn;
    switch (op.kind()) {
        case Operator::Kind::PLUS:  foldFn = +[](double a, double b) { return a + b; }; break;
        case Operator::Kind::MINUS: foldFn = +[](double a, double b) { return a - b; }; break;
        case Operator::Kind::STAR:  foldFn = +[](double a, double b) { return a * b; }; break;
        case Operator::Kind::SLASH: foldFn = +[](double a, double b) { return a / b; }; break;
        default:
            return nullptr;
    }

    const Type& componentType = type.componentType();
    SkASSERT(componentType.isNumber());

    double minimumValue = componentType.minimumValue();
    double maximumValue = componentType.maximumValue();

    double args[16];
    int numSlots = type.slotCount();
    for (int i = 0; i < numSlots; i++) {
        double value = foldFn(*left.getConstantValue(i), *right.getConstantValue(i));
        if (value < minimumValue || value > maximumValue) {
            return nullptr;
        }
        args[i] = value;
    }
    return ConstructorCompound::MakeFromConstants(context, pos, type, args);
}

static std::unique_ptr<Expression> splat_scalar(const Context& context,
                                                const Expression& scalar,
                                                const Type& type) {
    if (type.isVector()) {
        return ConstructorSplat::Make(context, scalar.fPosition, type, scalar.clone());
    }
    if (type.isMatrix()) {
        int numSlots = type.slotCount();
        ExpressionArray splatMatrix;
        splatMatrix.reserve_exact(numSlots);
        for (int index = 0; index < numSlots; ++index) {
            splatMatrix.push_back(scalar.clone());
        }
        return ConstructorCompound::Make(context, scalar.fPosition, type, std::move(splatMatrix));
    }
    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

static std::unique_ptr<Expression> cast_expression(const Context& context,
                                                   Position pos,
                                                   const Expression& expr,
                                                   const Type& type) {
    SkASSERT(type.componentType().matches(expr.type().componentType()));
    if (expr.type().isScalar()) {
        if (type.isMatrix()) {
            return ConstructorDiagonalMatrix::Make(context, pos, type, expr.clone());
        }
        if (type.isVector()) {
            return ConstructorSplat::Make(context, pos, type, expr.clone());
        }
    }
    if (type.matches(expr.type())) {
        return expr.clone(pos);
    }
    // We can't cast matrices into vectors or vice-versa.
    return nullptr;
}

static std::unique_ptr<Expression> zero_expression(const Context& context,
                                                   Position pos,
                                                   const Type& type) {
    std::unique_ptr<Expression> zero = Literal::Make(pos, 0.0, &type.componentType());
    if (type.isScalar()) {
        return zero;
    }
    if (type.isVector()) {
        return ConstructorSplat::Make(context, pos, type, std::move(zero));
    }
    if (type.isMatrix()) {
        return ConstructorDiagonalMatrix::Make(context, pos, type, std::move(zero));
    }
    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

static std::unique_ptr<Expression> negate_expression(const Context& context,
                                                     Position pos,
                                                     const Expression& expr,
                                                     const Type& type) {
    std::unique_ptr<Expression> ctor = cast_expression(context, pos, expr, type);
    return ctor ? PrefixExpression::Make(context, pos, Operator::Kind::MINUS, std::move(ctor))
                : nullptr;
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
        std::optional<double> slotVal = expr.getConstantValue(index);
        if (slotVal.has_value() && *slotVal == 0.0) {
            return true;
        }
    }
    return false;
}

bool ConstantFolder::IsConstantSplat(const Expression& expr, double value) {
    int numSlots = expr.type().slotCount();
    for (int index = 0; index < numSlots; ++index) {
        std::optional<double> slotVal = expr.getConstantValue(index);
        if (!slotVal.has_value() || *slotVal != value) {
            return false;
        }
    }
    return true;
}

// Returns true if the expression is a square diagonal matrix containing `value`.
static bool is_constant_diagonal(const Expression& expr, double value) {
    SkASSERT(expr.type().isMatrix());
    int columns = expr.type().columns();
    int rows = expr.type().rows();
    if (columns != rows) {
        return false;
    }
    int slotIdx = 0;
    for (int c = 0; c < columns; ++c) {
        for (int r = 0; r < rows; ++r) {
            double expectation = (c == r) ? value : 0;
            std::optional<double> slotVal = expr.getConstantValue(slotIdx++);
            if (!slotVal.has_value() || *slotVal != expectation) {
                return false;
            }
        }
    }
    return true;
}

// Returns true if the expression is a scalar, vector, or diagonal matrix containing `value`.
static bool is_constant_value(const Expression& expr, double value) {
    return expr.type().isMatrix() ? is_constant_diagonal(expr, value)
                                  : ConstantFolder::IsConstantSplat(expr, value);
}

// The expression represents the right-hand side of a division op. If the division can be
// strength-reduced into multiplication by a reciprocal, returns that reciprocal as an expression.
// Note that this only supports literal values with safe-to-use reciprocals, and returns null if
// Expression contains anything else.
static std::unique_ptr<Expression> make_reciprocal_expression(const Context& context,
                                                              const Expression& right) {
    if (right.type().isMatrix() || !right.type().componentType().isFloat()) {
        return nullptr;
    }
    // Verify that each slot contains a finite, non-zero literal, take its reciprocal.
    double values[4];
    int nslots = right.type().slotCount();
    for (int index = 0; index < nslots; ++index) {
        std::optional<double> value = right.getConstantValue(index);
        if (!value) {
            return nullptr;
        }
        *value = sk_ieee_double_divide(1.0, *value);
        if (*value >= -FLT_MAX && *value <= FLT_MAX && *value != 0.0) {
            // The reciprocal can be represented safely as a finite 32-bit float.
            values[index] = *value;
        } else {
            // The value is outside the 32-bit float range, or is NaN; do not optimize.
            return nullptr;
        }
    }
    // Turn the expression array into a compound constructor. (If this is a single-slot expression,
    // this will return the literal as-is.)
    return ConstructorCompound::MakeFromConstants(context, right.fPosition, right.type(), values);
}

static bool error_on_divide_by_zero(const Context& context, Position pos, Operator op,
                                    const Expression& right) {
    switch (op.kind()) {
        case Operator::Kind::SLASH:
        case Operator::Kind::SLASHEQ:
        case Operator::Kind::PERCENT:
        case Operator::Kind::PERCENTEQ:
            if (contains_constant_zero(right)) {
                context.fErrors->error(pos, "division by zero");
                return true;
            }
            return false;
        default:
            return false;
    }
}

const Expression* ConstantFolder::GetConstantValueOrNull(const Expression& inExpr) {
    const Expression* expr = &inExpr;
    while (expr->is<VariableReference>()) {
        const VariableReference& varRef = expr->as<VariableReference>();
        if (varRef.refKind() != VariableRefKind::kRead) {
            return nullptr;
        }
        const Variable& var = *varRef.variable();
        if (!var.modifierFlags().isConst()) {
            return nullptr;
        }
        expr = var.initialValue();
        if (!expr) {
            // Generally, const variables must have initial values. However, function parameters are
            // an exception; they can be const but won't have an initial value.
            return nullptr;
        }
    }
    return Analysis::IsCompileTimeConstant(*expr) ? expr : nullptr;
}

const Expression* ConstantFolder::GetConstantValueForVariable(const Expression& inExpr) {
    const Expression* expr = GetConstantValueOrNull(inExpr);
    return expr ? expr : &inExpr;
}

std::unique_ptr<Expression> ConstantFolder::MakeConstantValueForVariable(
        Position pos, std::unique_ptr<Expression> inExpr) {
    const Expression* expr = GetConstantValueOrNull(*inExpr);
    return expr ? expr->clone(pos) : std::move(inExpr);
}

static bool is_scalar_op_matrix(const Expression& left, const Expression& right) {
    return left.type().isScalar() && right.type().isMatrix();
}

static bool is_matrix_op_scalar(const Expression& left, const Expression& right) {
    return is_scalar_op_matrix(right, left);
}

static std::unique_ptr<Expression> simplify_arithmetic(const Context& context,
                                                       Position pos,
                                                       const Expression& left,
                                                       Operator op,
                                                       const Expression& right,
                                                       const Type& resultType) {
    switch (op.kind()) {
        case Operator::Kind::PLUS:
            if (!is_scalar_op_matrix(left, right) &&
                ConstantFolder::IsConstantSplat(right, 0.0)) {  // x + 0
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, left,
                                                                       resultType)) {
                    return expr;
                }
            }
            if (!is_matrix_op_scalar(left, right) &&
                ConstantFolder::IsConstantSplat(left, 0.0)) {  // 0 + x
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, right,
                                                                       resultType)) {
                    return expr;
                }
            }
            break;

        case Operator::Kind::STAR:
            if (is_constant_value(right, 1.0)) {  // x * 1
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, left,
                                                                       resultType)) {
                    return expr;
                }
            }
            if (is_constant_value(left, 1.0)) {   // 1 * x
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, right,
                                                                       resultType)) {
                    return expr;
                }
            }
            if (is_constant_value(right, 0.0) && !Analysis::HasSideEffects(left)) {  // x * 0
                return zero_expression(context, pos, resultType);
            }
            if (is_constant_value(left, 0.0) && !Analysis::HasSideEffects(right)) {  // 0 * x
                return zero_expression(context, pos, resultType);
            }
            if (is_constant_value(right, -1.0)) {  // x * -1 (to `-x`)
                if (std::unique_ptr<Expression> expr = negate_expression(context, pos, left,
                                                                         resultType)) {
                    return expr;
                }
            }
            if (is_constant_value(left, -1.0)) {  // -1 * x (to `-x`)
                if (std::unique_ptr<Expression> expr = negate_expression(context, pos, right,
                                                                         resultType)) {
                    return expr;
                }
            }
            break;

        case Operator::Kind::MINUS:
            if (!is_scalar_op_matrix(left, right) &&
                ConstantFolder::IsConstantSplat(right, 0.0)) {  // x - 0
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, left,
                                                                       resultType)) {
                    return expr;
                }
            }
            if (!is_matrix_op_scalar(left, right) &&
                ConstantFolder::IsConstantSplat(left, 0.0)) {  // 0 - x
                if (std::unique_ptr<Expression> expr = negate_expression(context, pos, right,
                                                                         resultType)) {
                    return expr;
                }
            }
            break;

        case Operator::Kind::SLASH:
            if (!is_scalar_op_matrix(left, right) &&
                ConstantFolder::IsConstantSplat(right, 1.0)) {  // x / 1
                if (std::unique_ptr<Expression> expr = cast_expression(context, pos, left,
                                                                       resultType)) {
                    return expr;
                }
            }
            if (!left.type().isMatrix()) {  // convert `x / 2` into `x * 0.5`
                if (std::unique_ptr<Expression> expr = make_reciprocal_expression(context, right)) {
                    return BinaryExpression::Make(context, pos, left.clone(), Operator::Kind::STAR,
                                                  std::move(expr));
                }
            }
            break;

        case Operator::Kind::PLUSEQ:
        case Operator::Kind::MINUSEQ:
            if (ConstantFolder::IsConstantSplat(right, 0.0)) {  // x += 0, x -= 0
                if (std::unique_ptr<Expression> var = cast_expression(context, pos, left,
                                                                      resultType)) {
                    Analysis::UpdateVariableRefKind(var.get(), VariableRefKind::kRead);
                    return var;
                }
            }
            break;

        case Operator::Kind::STAREQ:
            if (is_constant_value(right, 1.0)) {  // x *= 1
                if (std::unique_ptr<Expression> var = cast_expression(context, pos, left,
                                                                      resultType)) {
                    Analysis::UpdateVariableRefKind(var.get(), VariableRefKind::kRead);
                    return var;
                }
            }
            break;

        case Operator::Kind::SLASHEQ:
            if (ConstantFolder::IsConstantSplat(right, 1.0)) {  // x /= 1
                if (std::unique_ptr<Expression> var = cast_expression(context, pos, left,
                                                                      resultType)) {
                    Analysis::UpdateVariableRefKind(var.get(), VariableRefKind::kRead);
                    return var;
                }
            }
            if (std::unique_ptr<Expression> expr = make_reciprocal_expression(context, right)) {
                return BinaryExpression::Make(context, pos, left.clone(), Operator::Kind::STAREQ,
                                              std::move(expr));
            }
            break;

        default:
            break;
    }

    return nullptr;
}

// The expression must be scalar, and represents the right-hand side of a division op. It can
// contain anything, not just literal values. This returns the binary expression `1.0 / expr`. The
// expression might be further simplified by the constant folding, if possible.
static std::unique_ptr<Expression> one_over_scalar(const Context& context,
                                                   const Expression& right) {
    SkASSERT(right.type().isScalar());
    Position pos = right.fPosition;
    return BinaryExpression::Make(context, pos,
                                  Literal::Make(pos, 1.0, &right.type()),
                                  Operator::Kind::SLASH,
                                  right.clone());
}

static std::unique_ptr<Expression> simplify_matrix_division(const Context& context,
                                                            Position pos,
                                                            const Expression& left,
                                                            Operator op,
                                                            const Expression& right,
                                                            const Type& resultType) {
    // Convert matrix-over-scalar `x /= y` into `x *= (1.0 / y)`. This generates better
    // code in SPIR-V and Metal, and should be roughly equivalent elsewhere.
    switch (op.kind()) {
        case OperatorKind::SLASH:
        case OperatorKind::SLASHEQ:
            if (left.type().isMatrix() && right.type().isScalar()) {
                Operator multiplyOp = op.isAssignment() ? OperatorKind::STAREQ
                                                        : OperatorKind::STAR;
                return BinaryExpression::Make(context, pos,
                                              left.clone(),
                                              multiplyOp,
                                              one_over_scalar(context, right));
            }
            break;

        default:
            break;
    }

    return nullptr;
}

static std::unique_ptr<Expression> fold_expression(Position pos,
                                                   double result,
                                                   const Type* resultType) {
    if (resultType->isNumber()) {
        if (result >= resultType->minimumValue() && result <= resultType->maximumValue()) {
            // This result will fit inside its type.
        } else {
            // The value is outside the range or is NaN (all if-checks fail); do not optimize.
            return nullptr;
        }
    }

    return Literal::Make(pos, result, resultType);
}

static std::unique_ptr<Expression> fold_two_constants(const Context& context,
                                                      Position pos,
                                                      const Expression* left,
                                                      Operator op,
                                                      const Expression* right,
                                                      const Type& resultType) {
    SkASSERT(Analysis::IsCompileTimeConstant(*left));
    SkASSERT(Analysis::IsCompileTimeConstant(*right));
    const Type& leftType = left->type();
    const Type& rightType = right->type();

    // Handle pairs of integer literals.
    if (left->isIntLiteral() && right->isIntLiteral()) {
        using SKSL_UINT = uint64_t;
        SKSL_INT leftVal  = left->as<Literal>().intValue();
        SKSL_INT rightVal = right->as<Literal>().intValue();

        // Note that fold_expression returns null if the result would overflow its type.
        #define RESULT(Op)   fold_expression(pos, (SKSL_INT)(leftVal) Op \
                                                  (SKSL_INT)(rightVal), &resultType)
        #define URESULT(Op)  fold_expression(pos, (SKSL_INT)((SKSL_UINT)(leftVal) Op \
                                                  (SKSL_UINT)(rightVal)), &resultType)
        switch (op.kind()) {
            case Operator::Kind::PLUS:       return URESULT(+);
            case Operator::Kind::MINUS:      return URESULT(-);
            case Operator::Kind::STAR:       return URESULT(*);
            case Operator::Kind::SLASH:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors->error(pos, "arithmetic overflow");
                    return nullptr;
                }
                return RESULT(/);

            case Operator::Kind::PERCENT:
                if (leftVal == std::numeric_limits<SKSL_INT>::min() && rightVal == -1) {
                    context.fErrors->error(pos, "arithmetic overflow");
                    return nullptr;
                }
                return RESULT(%);

            case Operator::Kind::BITWISEAND: return RESULT(&);
            case Operator::Kind::BITWISEOR:  return RESULT(|);
            case Operator::Kind::BITWISEXOR: return RESULT(^);
            case Operator::Kind::EQEQ:       return RESULT(==);
            case Operator::Kind::NEQ:        return RESULT(!=);
            case Operator::Kind::GT:         return RESULT(>);
            case Operator::Kind::GTEQ:       return RESULT(>=);
            case Operator::Kind::LT:         return RESULT(<);
            case Operator::Kind::LTEQ:       return RESULT(<=);
            case Operator::Kind::SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    // Left-shifting a negative (or really, any signed) value is undefined behavior
                    // in C++, but not in GLSL. Do the shift on unsigned values to avoid triggering
                    // an UBSAN error.
                    return URESULT(<<);
                }
                context.fErrors->error(pos, "shift value out of range");
                return nullptr;

            case Operator::Kind::SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(>>);
                }
                context.fErrors->error(pos, "shift value out of range");
                return nullptr;

            default:
                break;
        }
        #undef RESULT
        #undef URESULT

        return nullptr;
    }

    // Handle pairs of floating-point literals.
    if (left->isFloatLiteral() && right->isFloatLiteral()) {
        SKSL_FLOAT leftVal  = left->as<Literal>().floatValue();
        SKSL_FLOAT rightVal = right->as<Literal>().floatValue();

        #define RESULT(Op) fold_expression(pos, leftVal Op rightVal, &resultType)
        switch (op.kind()) {
            case Operator::Kind::PLUS:  return RESULT(+);
            case Operator::Kind::MINUS: return RESULT(-);
            case Operator::Kind::STAR:  return RESULT(*);
            case Operator::Kind::SLASH: return RESULT(/);
            case Operator::Kind::EQEQ:  return RESULT(==);
            case Operator::Kind::NEQ:   return RESULT(!=);
            case Operator::Kind::GT:    return RESULT(>);
            case Operator::Kind::GTEQ:  return RESULT(>=);
            case Operator::Kind::LT:    return RESULT(<);
            case Operator::Kind::LTEQ:  return RESULT(<=);
            default:                    break;
        }
        #undef RESULT

        return nullptr;
    }

    // Perform matrix multiplication.
    if (op.kind() == Operator::Kind::STAR) {
        if (leftType.isMatrix() && rightType.isMatrix()) {
            return simplify_matrix_times_matrix(context, pos, *left, *right);
        }
        if (leftType.isVector() && rightType.isMatrix()) {
            return simplify_vector_times_matrix(context, pos, *left, *right);
        }
        if (leftType.isMatrix() && rightType.isVector()) {
            return simplify_matrix_times_vector(context, pos, *left, *right);
        }
    }

    // Perform constant folding on pairs of vectors/matrices.
    if (is_vec_or_mat(leftType) && leftType.matches(rightType)) {
        return simplify_componentwise(context, pos, *left, op, *right);
    }

    // Perform constant folding on vectors/matrices against scalars, e.g.: half4(2) + 2
    if (rightType.isScalar() && is_vec_or_mat(leftType) &&
        leftType.componentType().matches(rightType)) {
        return simplify_componentwise(context, pos,
                                      *left, op, *splat_scalar(context, *right, left->type()));
    }

    // Perform constant folding on scalars against vectors/matrices, e.g.: 2 + half4(2)
    if (leftType.isScalar() && is_vec_or_mat(rightType) &&
        rightType.componentType().matches(leftType)) {
        return simplify_componentwise(context, pos,
                                      *splat_scalar(context, *left, right->type()), op, *right);
    }

    // Perform constant folding on pairs of matrices, arrays or structs.
    if ((leftType.isMatrix() && rightType.isMatrix()) ||
        (leftType.isArray() && rightType.isArray()) ||
        (leftType.isStruct() && rightType.isStruct())) {
        return simplify_constant_equality(context, pos, *left, op, *right);
    }

    // We aren't able to constant-fold these expressions.
    return nullptr;
}

std::unique_ptr<Expression> ConstantFolder::Simplify(const Context& context,
                                                     Position pos,
                                                     const Expression& leftExpr,
                                                     Operator op,
                                                     const Expression& rightExpr,
                                                     const Type& resultType) {
    // Replace constant variables with their literal values.
    const Expression* left = GetConstantValueForVariable(leftExpr);
    const Expression* right = GetConstantValueForVariable(rightExpr);

    // If this is the assignment operator, and both sides are the same trivial expression, this is
    // self-assignment (i.e., `var = var`) and can be reduced to just a variable reference (`var`).
    // This can happen when other parts of the assignment are optimized away.
    if (op.kind() == Operator::Kind::EQ && Analysis::IsSameExpressionTree(*left, *right)) {
        return right->clone(pos);
    }

    // Simplify the expression when both sides are constant Boolean literals.
    if (left->isBoolLiteral() && right->isBoolLiteral()) {
        bool leftVal  = left->as<Literal>().boolValue();
        bool rightVal = right->as<Literal>().boolValue();
        bool result;
        switch (op.kind()) {
            case Operator::Kind::LOGICALAND: result = leftVal && rightVal; break;
            case Operator::Kind::LOGICALOR:  result = leftVal || rightVal; break;
            case Operator::Kind::LOGICALXOR: result = leftVal ^  rightVal; break;
            case Operator::Kind::EQEQ:       result = leftVal == rightVal; break;
            case Operator::Kind::NEQ:        result = leftVal != rightVal; break;
            default: return nullptr;
        }
        return Literal::MakeBool(context, pos, result);
    }

    // If the left side is a Boolean literal, apply short-circuit optimizations.
    if (left->isBoolLiteral()) {
        return short_circuit_boolean(pos, *left, op, *right);
    }

    // If the right side is a Boolean literal...
    if (right->isBoolLiteral()) {
        // ... and the left side has no side effects...
        if (!Analysis::HasSideEffects(*left)) {
            // We can reverse the expressions and short-circuit optimizations are still valid.
            return short_circuit_boolean(pos, *right, op, *left);
        }

        // We can't use short-circuiting, but we can still optimize away no-op Boolean expressions.
        return eliminate_no_op_boolean(pos, *left, op, *right);
    }

    if (op.kind() == Operator::Kind::EQEQ && Analysis::IsSameExpressionTree(*left, *right)) {
        // With == comparison, if both sides are the same trivial expression, this is self-
        // comparison and is always true. (We are not concerned with NaN.)
        return Literal::MakeBool(context, pos, /*value=*/true);
    }

    if (op.kind() == Operator::Kind::NEQ && Analysis::IsSameExpressionTree(*left, *right)) {
        // With != comparison, if both sides are the same trivial expression, this is self-
        // comparison and is always false. (We are not concerned with NaN.)
        return Literal::MakeBool(context, pos, /*value=*/false);
    }

    if (error_on_divide_by_zero(context, pos, op, *right)) {
        return nullptr;
    }

    // Perform full constant folding when both sides are compile-time constants.
    bool leftSideIsConstant = Analysis::IsCompileTimeConstant(*left);
    bool rightSideIsConstant = Analysis::IsCompileTimeConstant(*right);
    if (leftSideIsConstant && rightSideIsConstant) {
        return fold_two_constants(context, pos, left, op, right, resultType);
    }

    if (context.fConfig->fSettings.fOptimize) {
        // If just one side is constant, we might still be able to simplify arithmetic expressions
        // like `x * 1`, `x *= 1`, `x + 0`, `x * 0`, `0 / x`, etc.
        if (leftSideIsConstant || rightSideIsConstant) {
            if (std::unique_ptr<Expression> expr = simplify_arithmetic(context, pos, *left, op,
                                                                       *right, resultType)) {
                return expr;
            }
        }

        // We can simplify some forms of matrix division even when neither side is constant.
        if (std::unique_ptr<Expression> expr = simplify_matrix_division(context, pos, *left, op,
                                                                        *right, resultType)) {
            return expr;
        }
    }

    // We aren't able to constant-fold.
    return nullptr;
}

}  // namespace SkSL
