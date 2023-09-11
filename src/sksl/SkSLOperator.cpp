/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLOperator.h"

#include "include/core/SkTypes.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLType.h"

#include <memory>

namespace SkSL {

OperatorPrecedence Operator::getBinaryPrecedence() const {
    switch (this->kind()) {
        case Kind::STAR:         // fall through
        case Kind::SLASH:        // fall through
        case Kind::PERCENT:      return OperatorPrecedence::kMultiplicative;
        case Kind::PLUS:         // fall through
        case Kind::MINUS:        return OperatorPrecedence::kAdditive;
        case Kind::SHL:          // fall through
        case Kind::SHR:          return OperatorPrecedence::kShift;
        case Kind::LT:           // fall through
        case Kind::GT:           // fall through
        case Kind::LTEQ:         // fall through
        case Kind::GTEQ:         return OperatorPrecedence::kRelational;
        case Kind::EQEQ:         // fall through
        case Kind::NEQ:          return OperatorPrecedence::kEquality;
        case Kind::BITWISEAND:   return OperatorPrecedence::kBitwiseAnd;
        case Kind::BITWISEXOR:   return OperatorPrecedence::kBitwiseXor;
        case Kind::BITWISEOR:    return OperatorPrecedence::kBitwiseOr;
        case Kind::LOGICALAND:   return OperatorPrecedence::kLogicalAnd;
        case Kind::LOGICALXOR:   return OperatorPrecedence::kLogicalXor;
        case Kind::LOGICALOR:    return OperatorPrecedence::kLogicalOr;
        case Kind::EQ:           // fall through
        case Kind::PLUSEQ:       // fall through
        case Kind::MINUSEQ:      // fall through
        case Kind::STAREQ:       // fall through
        case Kind::SLASHEQ:      // fall through
        case Kind::PERCENTEQ:    // fall through
        case Kind::SHLEQ:        // fall through
        case Kind::SHREQ:        // fall through
        case Kind::BITWISEANDEQ: // fall through
        case Kind::BITWISEXOREQ: // fall through
        case Kind::BITWISEOREQ:  return OperatorPrecedence::kAssignment;
        case Kind::COMMA:        return OperatorPrecedence::kSequence;
        default: SK_ABORT("unsupported binary operator");
    }
}

const char* Operator::operatorName() const {
    switch (this->kind()) {
        case Kind::PLUS:         return " + ";
        case Kind::MINUS:        return " - ";
        case Kind::STAR:         return " * ";
        case Kind::SLASH:        return " / ";
        case Kind::PERCENT:      return " % ";
        case Kind::SHL:          return " << ";
        case Kind::SHR:          return " >> ";
        case Kind::LOGICALNOT:   return "!";
        case Kind::LOGICALAND:   return " && ";
        case Kind::LOGICALOR:    return " || ";
        case Kind::LOGICALXOR:   return " ^^ ";
        case Kind::BITWISENOT:   return "~";
        case Kind::BITWISEAND:   return " & ";
        case Kind::BITWISEOR:    return " | ";
        case Kind::BITWISEXOR:   return " ^ ";
        case Kind::EQ:           return " = ";
        case Kind::EQEQ:         return " == ";
        case Kind::NEQ:          return " != ";
        case Kind::LT:           return " < ";
        case Kind::GT:           return " > ";
        case Kind::LTEQ:         return " <= ";
        case Kind::GTEQ:         return " >= ";
        case Kind::PLUSEQ:       return " += ";
        case Kind::MINUSEQ:      return " -= ";
        case Kind::STAREQ:       return " *= ";
        case Kind::SLASHEQ:      return " /= ";
        case Kind::PERCENTEQ:    return " %= ";
        case Kind::SHLEQ:        return " <<= ";
        case Kind::SHREQ:        return " >>= ";
        case Kind::BITWISEANDEQ: return " &= ";
        case Kind::BITWISEOREQ:  return " |= ";
        case Kind::BITWISEXOREQ: return " ^= ";
        case Kind::PLUSPLUS:     return "++";
        case Kind::MINUSMINUS:   return "--";
        case Kind::COMMA:        return ", ";
        default: SkUNREACHABLE;
    }
}

std::string_view Operator::tightOperatorName() const {
    std::string_view name = this->operatorName();
    if (skstd::starts_with(name, ' ')) {
        name.remove_prefix(1);
    }
    if (skstd::ends_with(name, ' ')) {
        name.remove_suffix(1);
    }
    return name;
}

bool Operator::isAssignment() const {
    switch (this->kind()) {
        case Kind::EQ:           // fall through
        case Kind::PLUSEQ:       // fall through
        case Kind::MINUSEQ:      // fall through
        case Kind::STAREQ:       // fall through
        case Kind::SLASHEQ:      // fall through
        case Kind::PERCENTEQ:    // fall through
        case Kind::SHLEQ:        // fall through
        case Kind::SHREQ:        // fall through
        case Kind::BITWISEOREQ:  // fall through
        case Kind::BITWISEXOREQ: // fall through
        case Kind::BITWISEANDEQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isCompoundAssignment() const {
    return this->isAssignment() && this->kind() != Kind::EQ;
}

Operator Operator::removeAssignment() const {
    switch (this->kind()) {
        case Kind::PLUSEQ:       return Kind::PLUS;
        case Kind::MINUSEQ:      return Kind::MINUS;
        case Kind::STAREQ:       return Kind::STAR;
        case Kind::SLASHEQ:      return Kind::SLASH;
        case Kind::PERCENTEQ:    return Kind::PERCENT;
        case Kind::SHLEQ:        return Kind::SHL;
        case Kind::SHREQ:        return Kind::SHR;
        case Kind::BITWISEOREQ:  return Kind::BITWISEOR;
        case Kind::BITWISEXOREQ: return Kind::BITWISEXOR;
        case Kind::BITWISEANDEQ: return Kind::BITWISEAND;
        default: return *this;
    }
}

bool Operator::isRelational() const {
    switch (this->kind()) {
        case Kind::LT:
        case Kind::GT:
        case Kind::LTEQ:
        case Kind::GTEQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isOnlyValidForIntegralTypes() const {
    switch (this->kind()) {
        case Kind::SHL:
        case Kind::SHR:
        case Kind::BITWISEAND:
        case Kind::BITWISEOR:
        case Kind::BITWISEXOR:
        case Kind::PERCENT:
        case Kind::SHLEQ:
        case Kind::SHREQ:
        case Kind::BITWISEANDEQ:
        case Kind::BITWISEOREQ:
        case Kind::BITWISEXOREQ:
        case Kind::PERCENTEQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isValidForMatrixOrVector() const {
    switch (this->kind()) {
        case Kind::PLUS:
        case Kind::MINUS:
        case Kind::STAR:
        case Kind::SLASH:
        case Kind::PERCENT:
        case Kind::SHL:
        case Kind::SHR:
        case Kind::BITWISEAND:
        case Kind::BITWISEOR:
        case Kind::BITWISEXOR:
        case Kind::PLUSEQ:
        case Kind::MINUSEQ:
        case Kind::STAREQ:
        case Kind::SLASHEQ:
        case Kind::PERCENTEQ:
        case Kind::SHLEQ:
        case Kind::SHREQ:
        case Kind::BITWISEANDEQ:
        case Kind::BITWISEOREQ:
        case Kind::BITWISEXOREQ:
            return true;
        default:
            return false;
    }
}

bool Operator::isMatrixMultiply(const Type& left, const Type& right) const {
    if (this->kind() != Kind::STAR && this->kind() != Kind::STAREQ) {
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
                                   const Type** outResultType) const {
    const bool allowNarrowing = context.fConfig->fSettings.fAllowNarrowingConversions;
    switch (this->kind()) {
        case Kind::EQ:  // left = right
            if (left.isVoid()) {
                return false;
            }
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
            return right.canCoerceTo(left, allowNarrowing);

        case Kind::EQEQ:   // left == right
        case Kind::NEQ: {  // left != right
            if (left.isVoid() || left.isOpaque()) {
                return false;
            }
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
        case Kind::LOGICALOR:   // left || right
        case Kind::LOGICALAND:  // left && right
        case Kind::LOGICALXOR:  // left ^^ right
            *outLeftType = context.fTypes.fBool.get();
            *outRightType = context.fTypes.fBool.get();
            *outResultType = context.fTypes.fBool.get();
            return left.canCoerceTo(*context.fTypes.fBool, allowNarrowing) &&
                   right.canCoerceTo(*context.fTypes.fBool, allowNarrowing);

        case Operator::Kind::COMMA:  // left, right
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
        if (!this->isRelational()) {
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
        if (!this->isRelational()) {
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
        if (this->isRelational()) {
            *outResultType = context.fTypes.fBool.get();
        }
        return true;
    }
    return false;
}

}  // namespace SkSL
