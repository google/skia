/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLPrefixExpression.h"

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

static ExpressionArray negate_operands(const Context& context, ExpressionArray operands);

static std::unique_ptr<Expression> negate_operand(const Context& context,
                                                  std::unique_ptr<Expression> operand) {
    const Expression* value = ConstantFolder::GetConstantValueForVariable(*operand);
    switch (value->kind()) {
        case Expression::Kind::kFloatLiteral:
            // Convert -floatLiteral(1) to floatLiteral(-1).
            return FloatLiteral::Make(operand->fOffset,
                                      -value->as<FloatLiteral>().value(),
                                      &value->type());

        case Expression::Kind::kIntLiteral:
            // Convert -intLiteral(1) to intLiteral(-1).
            return IntLiteral::Make(operand->fOffset,
                                    -value->as<IntLiteral>().value(),
                                    &value->type());

        case Expression::Kind::kPrefix:
            if (context.fConfig->fSettings.fOptimize) {
                // Convert `-(-expression)` into `expression`.
                PrefixExpression& prefix = operand->as<PrefixExpression>();
                if (prefix.getOperator().kind() == Token::Kind::TK_MINUS) {
                    return std::move(prefix.operand());
                }
            }
            break;

        case Expression::Kind::kConstructorArray:
            // Convert `-array[N](literal, ...)` into `array[N](-literal, ...)`.
            if (context.fConfig->fSettings.fOptimize && value->isCompileTimeConstant()) {
                ConstructorArray& ctor = operand->as<ConstructorArray>();
                return ConstructorArray::Make(
                        context, ctor.fOffset, ctor.type(),
                        negate_operands(context, std::move(ctor.arguments())));
            }
            break;

        case Expression::Kind::kConstructorDiagonalMatrix:
            // Convert `-matrix(literal)` into `matrix(-literal)`.
            if (context.fConfig->fSettings.fOptimize && value->isCompileTimeConstant()) {
                ConstructorDiagonalMatrix& ctor = operand->as<ConstructorDiagonalMatrix>();
                return ConstructorDiagonalMatrix::Make(
                        context, ctor.fOffset, ctor.type(),
                        negate_operand(context, std::move(ctor.argument())));
            }
            break;

        case Expression::Kind::kConstructorSplat:
            // Convert `-vector(literal)` into `vector(-literal)`.
            if (context.fConfig->fSettings.fOptimize && value->isCompileTimeConstant()) {
                ConstructorSplat& ctor = operand->as<ConstructorSplat>();
                return ConstructorSplat::Make(context, ctor.fOffset, ctor.type(),
                                              negate_operand(context, std::move(ctor.argument())));
            }
            break;

        case Expression::Kind::kConstructorCompound:
            // Convert `-vecN(literal, ...)` into `vecN(-literal, ...)`.
            if (context.fConfig->fSettings.fOptimize && value->isCompileTimeConstant()) {
                ConstructorCompound& ctor = operand->as<ConstructorCompound>();
                return ConstructorCompound::Make(
                        context, ctor.fOffset, ctor.type(),
                        negate_operands(context, std::move(ctor.arguments())));
            }
            break;

        default:
            break;
    }

    // No simplified form; convert expression to Prefix(TK_MINUS, expression).
    return std::make_unique<PrefixExpression>(Token::Kind::TK_MINUS, std::move(operand));
}

static ExpressionArray negate_operands(const Context& context, ExpressionArray operands) {
    for (std::unique_ptr<Expression>& arg : operands) {
        arg = negate_operand(context, std::move(arg));
    }
    return operands;
}

static std::unique_ptr<Expression> logical_not_operand(const Context& context,
                                                       std::unique_ptr<Expression> operand) {
    const Expression* value = ConstantFolder::GetConstantValueForVariable(*operand);
    switch (value->kind()) {
        case Expression::Kind::kBoolLiteral: {
            // Convert !boolLiteral(true) to boolLiteral(false).
            const BoolLiteral& b = value->as<BoolLiteral>();
            return BoolLiteral::Make(operand->fOffset, !b.value(), &operand->type());
        }
        case Expression::Kind::kPrefix:
            if (context.fConfig->fSettings.fOptimize) {
                // Convert `!(!expression)` into `expression`.
                PrefixExpression& prefix = operand->as<PrefixExpression>();
                if (prefix.getOperator().kind() == Token::Kind::TK_LOGICALNOT) {
                    return std::move(prefix.operand());
                }
            }
            break;

        default:
            break;
    }

    // No simplified form; convert expression to Prefix(TK_LOGICALNOT, expression).
    return std::make_unique<PrefixExpression>(Token::Kind::TK_LOGICALNOT, std::move(operand));
}

std::unique_ptr<Expression> PrefixExpression::Convert(const Context& context,
                                                      Operator op,
                                                      std::unique_ptr<Expression> base) {
    const Type& baseType = base->type();
    switch (op.kind()) {
        case Token::Kind::TK_PLUS:
            if (!baseType.componentType().isNumber()) {
                context.errors().error(base->fOffset,
                                       "'+' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            break;

        case Token::Kind::TK_MINUS:
            if (!baseType.componentType().isNumber()) {
                context.errors().error(base->fOffset,
                                       "'-' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            break;

        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS:
            if (!baseType.isNumber()) {
                context.errors().error(base->fOffset,
                                       String("'") + op.operatorName() + "' cannot operate on '" +
                                       baseType.displayName() + "'");
                return nullptr;
            }
            if (!Analysis::MakeAssignmentExpr(base.get(), VariableReference::RefKind::kReadWrite,
                                              &context.errors())) {
                return nullptr;
            }
            break;

        case Token::Kind::TK_LOGICALNOT:
            if (!baseType.isBoolean()) {
                context.errors().error(base->fOffset,
                                       String("'") + op.operatorName() + "' cannot operate on '" +
                                       baseType.displayName() + "'");
                return nullptr;
            }
            break;

        case Token::Kind::TK_BITWISENOT:
            if (context.fConfig->strictES2Mode()) {
                // GLSL ES 1.00, Section 5.1
                context.errors().error(
                        base->fOffset,
                        String("operator '") + op.operatorName() + "' is not allowed");
                return nullptr;
            }
            if (!baseType.isInteger()) {
                context.errors().error(base->fOffset,
                                       String("'") + op.operatorName() + "' cannot operate on '" +
                                       baseType.displayName() + "'");
                return nullptr;
            }
            if (baseType.isLiteral()) {
                // The expression `~123` is no longer a literal; coerce to the actual type.
                base = baseType.scalarTypeForLiteral().coerceExpression(std::move(base), context);
            }
            break;

        default:
            SK_ABORT("unsupported prefix operator");
    }

    return PrefixExpression::Make(context, op, std::move(base));
}

std::unique_ptr<Expression> PrefixExpression::Make(const Context& context, Operator op,
                                                   std::unique_ptr<Expression> base) {
    switch (op.kind()) {
        case Token::Kind::TK_PLUS:
            SkASSERT(base->type().componentType().isNumber());
            return base;

        case Token::Kind::TK_MINUS:
            SkASSERT(base->type().componentType().isNumber());
            return negate_operand(context, std::move(base));

        case Token::Kind::TK_LOGICALNOT:
            SkASSERT(base->type().isBoolean());
            return logical_not_operand(context, std::move(base));

        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS:
            SkASSERT(base->type().isNumber());
            SkASSERT(Analysis::IsAssignable(*base));
            break;

        case Token::Kind::TK_BITWISENOT:
            SkASSERT(!context.fConfig->strictES2Mode());
            SkASSERT(base->type().isInteger());
            SkASSERT(!base->type().isLiteral());
            break;

        default:
            SkDEBUGFAILF("unsupported prefix operator: %s", op.operatorName());
    }

    return std::make_unique<PrefixExpression>(op, std::move(base));
}

}  // namespace SkSL
