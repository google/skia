/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

Operator::Precedence Operator::getBinaryPrecedence() const {
    switch (this->kind()) {
        case Kind::TK_STAR:         // fall through
        case Kind::TK_SLASH:        // fall through
        case Kind::TK_PERCENT:      return Precedence::kMultiplicative;
        case Kind::TK_PLUS:         // fall through
        case Kind::TK_MINUS:        return Precedence::kAdditive;
        case Kind::TK_SHL:          // fall through
        case Kind::TK_SHR:          return Precedence::kShift;
        case Kind::TK_LT:           // fall through
        case Kind::TK_GT:           // fall through
        case Kind::TK_LTEQ:         // fall through
        case Kind::TK_GTEQ:         return Precedence::kRelational;
        case Kind::TK_EQEQ:         // fall through
        case Kind::TK_NEQ:          return Precedence::kEquality;
        case Kind::TK_BITWISEAND:   return Precedence::kBitwiseAnd;
        case Kind::TK_BITWISEXOR:   return Precedence::kBitwiseXor;
        case Kind::TK_BITWISEOR:    return Precedence::kBitwiseOr;
        case Kind::TK_LOGICALAND:   return Precedence::kLogicalAnd;
        case Kind::TK_LOGICALXOR:   return Precedence::kLogicalXor;
        case Kind::TK_LOGICALOR:    return Precedence::kLogicalOr;
        case Kind::TK_EQ:           // fall through
        case Kind::TK_PLUSEQ:       // fall through
        case Kind::TK_MINUSEQ:      // fall through
        case Kind::TK_STAREQ:       // fall through
        case Kind::TK_SLASHEQ:      // fall through
        case Kind::TK_PERCENTEQ:    // fall through
        case Kind::TK_SHLEQ:        // fall through
        case Kind::TK_SHREQ:        // fall through
        case Kind::TK_BITWISEANDEQ: // fall through
        case Kind::TK_BITWISEXOREQ: // fall through
        case Kind::TK_BITWISEOREQ:  return Precedence::kAssignment;
        case Kind::TK_COMMA:        return Precedence::kSequence;
        default: SK_ABORT("unsupported binary operator");
    }
}

bool Operator::isOperator() const {
    switch (this->kind()) {
        case Kind::TK_PLUS:
        case Kind::TK_MINUS:
        case Kind::TK_STAR:
        case Kind::TK_SLASH:
        case Kind::TK_PERCENT:
        case Kind::TK_SHL:
        case Kind::TK_SHR:
        case Kind::TK_LOGICALNOT:
        case Kind::TK_LOGICALAND:
        case Kind::TK_LOGICALOR:
        case Kind::TK_LOGICALXOR:
        case Kind::TK_BITWISENOT:
        case Kind::TK_BITWISEAND:
        case Kind::TK_BITWISEOR:
        case Kind::TK_BITWISEXOR:
        case Kind::TK_EQ:
        case Kind::TK_EQEQ:
        case Kind::TK_NEQ:
        case Kind::TK_LT:
        case Kind::TK_GT:
        case Kind::TK_LTEQ:
        case Kind::TK_GTEQ:
        case Kind::TK_PLUSEQ:
        case Kind::TK_MINUSEQ:
        case Kind::TK_STAREQ:
        case Kind::TK_SLASHEQ:
        case Kind::TK_PERCENTEQ:
        case Kind::TK_SHLEQ:
        case Kind::TK_SHREQ:
        case Kind::TK_BITWISEANDEQ:
        case Kind::TK_BITWISEOREQ:
        case Kind::TK_BITWISEXOREQ:
        case Kind::TK_PLUSPLUS:
        case Kind::TK_MINUSMINUS:
        case Kind::TK_COMMA:
            return true;
        default:
            return false;
    }
}

const char* Operator::operatorName() const {
    switch (this->kind()) {
        case Kind::TK_PLUS:         return "+";
        case Kind::TK_MINUS:        return "-";
        case Kind::TK_STAR:         return "*";
        case Kind::TK_SLASH:        return "/";
        case Kind::TK_PERCENT:      return "%";
        case Kind::TK_SHL:          return "<<";
        case Kind::TK_SHR:          return ">>";
        case Kind::TK_LOGICALNOT:   return "!";
        case Kind::TK_LOGICALAND:   return "&&";
        case Kind::TK_LOGICALOR:    return "||";
        case Kind::TK_LOGICALXOR:   return "^^";
        case Kind::TK_BITWISENOT:   return "~";
        case Kind::TK_BITWISEAND:   return "&";
        case Kind::TK_BITWISEOR:    return "|";
        case Kind::TK_BITWISEXOR:   return "^";
        case Kind::TK_EQ:           return "=";
        case Kind::TK_EQEQ:         return "==";
        case Kind::TK_NEQ:          return "!=";
        case Kind::TK_LT:           return "<";
        case Kind::TK_GT:           return ">";
        case Kind::TK_LTEQ:         return "<=";
        case Kind::TK_GTEQ:         return ">=";
        case Kind::TK_PLUSEQ:       return "+=";
        case Kind::TK_MINUSEQ:      return "-=";
        case Kind::TK_STAREQ:       return "*=";
        case Kind::TK_SLASHEQ:      return "/=";
        case Kind::TK_PERCENTEQ:    return "%=";
        case Kind::TK_SHLEQ:        return "<<=";
        case Kind::TK_SHREQ:        return ">>=";
        case Kind::TK_BITWISEANDEQ: return "&=";
        case Kind::TK_BITWISEOREQ:  return "|=";
        case Kind::TK_BITWISEXOREQ: return "^=";
        case Kind::TK_PLUSPLUS:     return "++";
        case Kind::TK_MINUSMINUS:   return "--";
        case Kind::TK_COMMA:        return ",";
        default:
            SK_ABORT("unsupported operator: %d\n", (int) fKind);
    }
}

bool Operator::isAssignment() const {
    switch (this->kind()) {
        case Kind::TK_EQ:           // fall through
        case Kind::TK_PLUSEQ:       // fall through
        case Kind::TK_MINUSEQ:      // fall through
        case Kind::TK_STAREQ:       // fall through
        case Kind::TK_SLASHEQ:      // fall through
        case Kind::TK_PERCENTEQ:    // fall through
        case Kind::TK_SHLEQ:        // fall through
        case Kind::TK_SHREQ:        // fall through
        case Kind::TK_BITWISEOREQ:  // fall through
        case Kind::TK_BITWISEXOREQ: // fall through
        case Kind::TK_BITWISEANDEQ:
            return true;
        default:
            return false;
    }
}

Operator Operator::removeAssignment() const {
    switch (this->kind()) {
        case Kind::TK_PLUSEQ:       return Operator{Kind::TK_PLUS};
        case Kind::TK_MINUSEQ:      return Operator{Kind::TK_MINUS};
        case Kind::TK_STAREQ:       return Operator{Kind::TK_STAR};
        case Kind::TK_SLASHEQ:      return Operator{Kind::TK_SLASH};
        case Kind::TK_PERCENTEQ:    return Operator{Kind::TK_PERCENT};
        case Kind::TK_SHLEQ:        return Operator{Kind::TK_SHL};
        case Kind::TK_SHREQ:        return Operator{Kind::TK_SHR};
        case Kind::TK_BITWISEOREQ:  return Operator{Kind::TK_BITWISEOR};
        case Kind::TK_BITWISEXOREQ: return Operator{Kind::TK_BITWISEXOR};
        case Kind::TK_BITWISEANDEQ: return Operator{Kind::TK_BITWISEAND};
        default: return *this;
    }
}

bool Operator::isLogical() const {
    switch (this->kind()) {
        case Token::Kind::TK_LT:
        case Token::Kind::TK_GT:
        case Token::Kind::TK_LTEQ:
        case Token::Kind::TK_GTEQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isOnlyValidForIntegralTypes() const {
    switch (this->kind()) {
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR:
        case Token::Kind::TK_BITWISEAND:
        case Token::Kind::TK_BITWISEOR:
        case Token::Kind::TK_BITWISEXOR:
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_SHLEQ:
        case Token::Kind::TK_SHREQ:
        case Token::Kind::TK_BITWISEANDEQ:
        case Token::Kind::TK_BITWISEOREQ:
        case Token::Kind::TK_BITWISEXOREQ:
        case Token::Kind::TK_PERCENTEQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isValidForMatrixOrVector() const {
    switch (this->kind()) {
        case Token::Kind::TK_PLUS:
        case Token::Kind::TK_MINUS:
        case Token::Kind::TK_STAR:
        case Token::Kind::TK_SLASH:
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR:
        case Token::Kind::TK_BITWISEAND:
        case Token::Kind::TK_BITWISEOR:
        case Token::Kind::TK_BITWISEXOR:
        case Token::Kind::TK_PLUSEQ:
        case Token::Kind::TK_MINUSEQ:
        case Token::Kind::TK_STAREQ:
        case Token::Kind::TK_SLASHEQ:
        case Token::Kind::TK_PERCENTEQ:
        case Token::Kind::TK_SHLEQ:
        case Token::Kind::TK_SHREQ:
        case Token::Kind::TK_BITWISEANDEQ:
        case Token::Kind::TK_BITWISEOREQ:
        case Token::Kind::TK_BITWISEXOREQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isMatrixMultiply(const Type& left, const Type& right) {
    if (this->kind() != Token::Kind::TK_STAR && this->kind() != Token::Kind::TK_STAREQ) {
        return false;
    }
    if (left.isMatrix()) {
        return right.isMatrix() || right.isVector();
    }
    return left.isVector() && right.isMatrix();
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
bool Operator::determineBinaryType(const Context& context,
                                   const Type& left,
                                   const Type& right,
                                   const Type** outLeftType,
                                   const Type** outRightType,
                                   const Type** outResultType) {
    const bool allowNarrowing = context.fConfig->fSettings.fAllowNarrowingConversions;
    switch (this->kind()) {
        case Token::Kind::TK_EQ:  // left = right
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
            return right.canCoerceTo(left, allowNarrowing);

        case Token::Kind::TK_EQEQ:   // left == right
        case Token::Kind::TK_NEQ: {  // left != right
            CoercionCost rightToLeft = right.coercionCost(left),
                         leftToRight = left.coercionCost(right);
            if (rightToLeft < leftToRight) {
                if (rightToLeft.isPossible(allowNarrowing)) {
                    *outLeftType = &left;
                    *outRightType = &left;
                    *outResultType = context.fTypes.fBool.get();
                    return true;
                }
            } else {
                if (leftToRight.isPossible(allowNarrowing)) {
                    *outLeftType = &right;
                    *outRightType = &right;
                    *outResultType = context.fTypes.fBool.get();
                    return true;
                }
            }
            return false;
        }
        case Token::Kind::TK_LOGICALOR:   // left || right
        case Token::Kind::TK_LOGICALAND:  // left && right
        case Token::Kind::TK_LOGICALXOR:  // left ^^ right
            *outLeftType = context.fTypes.fBool.get();
            *outRightType = context.fTypes.fBool.get();
            *outResultType = context.fTypes.fBool.get();
            return left.canCoerceTo(*context.fTypes.fBool, allowNarrowing) &&
                   right.canCoerceTo(*context.fTypes.fBool, allowNarrowing);

        case Token::Kind::TK_COMMA:  // left, right
            if (left.isOpaque() || right.isOpaque()) {
                return false;
            }
            *outLeftType = &left;
            *outRightType = &right;
            *outResultType = &right;
            return true;

        default:
            break;
    }

    // Boolean types only support the operators listed above (, = == != || && ^^).
    // If we've gotten this far with a boolean, we have an unsupported operator.
    const Type& leftComponentType = left.componentType();
    const Type& rightComponentType = right.componentType();
    if (leftComponentType.isBoolean() || rightComponentType.isBoolean()) {
        return false;
    }

    bool isAssignment = this->isAssignment();
    if (this->isMatrixMultiply(left, right)) {  // left * right
        // Determine final component type.
        if (!this->determineBinaryType(context, left.componentType(), right.componentType(),
                                       outLeftType, outRightType, outResultType)) {
            return false;
        }
        // Convert component type to compound.
        *outLeftType = &(*outResultType)->toCompound(context, left.columns(), left.rows());
        *outRightType = &(*outResultType)->toCompound(context, right.columns(), right.rows());
        int leftColumns = left.columns(), leftRows = left.rows();
        int rightColumns = right.columns(), rightRows = right.rows();
        if (right.isVector()) {
            // `matrix * vector` treats the vector as a column vector; we need to transpose it.
            std::swap(rightColumns, rightRows);
            SkASSERT(rightColumns == 1);
        }
        if (rightColumns > 1) {
            *outResultType = &(*outResultType)->toCompound(context, rightColumns, leftRows);
        } else {
            // The result was a column vector. Transpose it back to a row.
            *outResultType = &(*outResultType)->toCompound(context, leftRows, rightColumns);
        }
        if (isAssignment && ((*outResultType)->columns() != leftColumns ||
                             (*outResultType)->rows() != leftRows)) {
            return false;
        }
        return leftColumns == rightRows;
    }

    bool leftIsVectorOrMatrix = left.isVector() || left.isMatrix();
    bool validMatrixOrVectorOp = this->isValidForMatrixOrVector();

    if (leftIsVectorOrMatrix && validMatrixOrVectorOp && right.isScalar()) {
        // Determine final component type.
        if (!this->determineBinaryType(context, left.componentType(), right,
                                       outLeftType, outRightType, outResultType)) {
            return false;
        }
        // Convert component type to compound.
        *outLeftType = &(*outLeftType)->toCompound(context, left.columns(), left.rows());
        if (!this->isLogical()) {
            *outResultType = &(*outResultType)->toCompound(context, left.columns(), left.rows());
        }
        return true;
    }

    bool rightIsVectorOrMatrix = right.isVector() || right.isMatrix();

    if (!isAssignment && rightIsVectorOrMatrix && validMatrixOrVectorOp && left.isScalar()) {
        // Determine final component type.
        if (!this->determineBinaryType(context, left, right.componentType(),
                                       outLeftType, outRightType, outResultType)) {
            return false;
        }
        // Convert component type to compound.
        *outRightType = &(*outRightType)->toCompound(context, right.columns(), right.rows());
        if (!this->isLogical()) {
            *outResultType = &(*outResultType)->toCompound(context, right.columns(), right.rows());
        }
        return true;
    }

    CoercionCost rightToLeftCost = right.coercionCost(left);
    CoercionCost leftToRightCost = isAssignment ? CoercionCost::Impossible()
                                                : left.coercionCost(right);

    if ((left.isScalar() && right.isScalar()) || (leftIsVectorOrMatrix && validMatrixOrVectorOp)) {
        if (this->isOnlyValidForIntegralTypes()) {
            if (!leftComponentType.isInteger() || !rightComponentType.isInteger()) {
                return false;
            }
        }
        if (rightToLeftCost.isPossible(allowNarrowing) && rightToLeftCost < leftToRightCost) {
            // Right-to-Left conversion is possible and cheaper
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
        } else if (leftToRightCost.isPossible(allowNarrowing)) {
            // Left-to-Right conversion is possible (and at least as cheap as Right-to-Left)
            *outLeftType = &right;
            *outRightType = &right;
            *outResultType = &right;
        } else {
            return false;
        }
        if (this->isLogical()) {
            *outResultType = context.fTypes.fBool.get();
        }
        return true;
    }
    return false;
}

}  // namespace SkSL
