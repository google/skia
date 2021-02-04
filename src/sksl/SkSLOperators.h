/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_OPERATORS
#define SKSL_OPERATORS

#include "src/sksl/SkSLLexer.h"

namespace SkSL {
namespace Operators {

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

    Precedence GetBinaryPrecedence(Token::Kind op);

    const char* OperatorName(Token::Kind op);

    // Returns true if op is '=' or any compound assignment operator ('+=', '-=', etc.)
    bool IsAssignment(Token::Kind op);

    // Given a compound assignment operator, returns the non-assignment version of the operator
    // (e.g. '+=' becomes '+')
    Token::Kind RemoveAssignment(Token::Kind op);

}  // namespace Operators
}  // namespace SkSL

#endif
