/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_OPERATORS
#define SKSL_OPERATORS

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLLexer.h"

namespace SkSL {

class Context;
class Type;

class Operator {
public:
    using Kind = Token::Kind;

    // Allow implicit conversion from Token::Kind, since this is just a utility wrapper on top.
    Operator(Token::Kind t) : fKind(t) {
        SkASSERTF(this->isOperator(), "token-kind %d is not an operator", (int)fKind);
    }

    enum class Precedence {
        kParentheses    =  1,
        kPostfix        =  2,
        kPrefix         =  3,
        kMultiplicative =  4,
        kAdditive       =  5,
        kShift          =  6,
        kRelational     =  7,
        kEquality       =  8,
        kBitwiseAnd     =  9,
        kBitwiseXor     = 10,
        kBitwiseOr      = 11,
        kLogicalAnd     = 12,
        kLogicalXor     = 13,
        kLogicalOr      = 14,
        kTernary        = 15,
        kAssignment     = 16,
        kSequence       = 17,
        kTopLevel       = kSequence
    };

    Token::Kind kind() const { return fKind; }

    Precedence getBinaryPrecedence() const;

    const char* operatorName() const;

    // Returns true if op is '=' or any compound assignment operator ('+=', '-=', etc.)
    bool isAssignment() const;

    // Given a compound assignment operator, returns the non-assignment version of the operator
    // (e.g. '+=' becomes '+')
    Operator removeAssignment() const;

    /**
     * Defines the set of logical (comparison) operators:
     *     <  <=  >  >=
     */
    bool isLogical() const;

    /**
     * Defines the set of operators which are only valid on integral types:
     *   <<  <<=  >>  >>=  &  &=  |  |=  ^  ^=  %  %=
     */
    bool isOnlyValidForIntegralTypes() const;

    /**
     * Defines the set of operators which perform vector/matrix math.
     *   +  +=  -  -=  *  *=  /  /=  %  %=  <<  <<=  >>  >>=  &  &=  |  |=  ^  ^=
     */
    bool isValidForMatrixOrVector() const;

    /*
     * Defines the set of operators allowed by The OpenGL ES Shading Language 1.00, Section 5.1.
     * The set of illegal (reserved) operators are the ones that only make sense with integral
     * types. This is not a coincidence: It's because ES2 doesn't require 'int' to be anything but
     * syntactic sugar for floats with truncation after each operation.
     */
    bool isAllowedInStrictES2Mode() const {
        return !this->isOnlyValidForIntegralTypes();
    }

    /**
     * Determines the operand and result types of a binary expression. Returns true if the
     * expression is legal, false otherwise. If false, the values of the out parameters are
     * undefined.
     */
    bool determineBinaryType(const Context& context,
                             const Type& left,
                             const Type& right,
                             const Type** outLeftType,
                             const Type** outRightType,
                             const Type** outResultType);

private:
    bool isOperator() const;
    bool isMatrixMultiply(const Type& left, const Type& right);

    Kind fKind;
};

}  // namespace SkSL

#endif
