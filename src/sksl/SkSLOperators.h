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

    enum Precedence {
        kParentheses_Precedence    =  1,
        kPostfix_Precedence        =  2,
        kPrefix_Precedence         =  3,
        kMultiplicative_Precedence =  4,
        kAdditive_Precedence       =  5,
        kShift_Precedence          =  6,
        kRelational_Precedence     =  7,
        kEquality_Precedence       =  8,
        kBitwiseAnd_Precedence     =  9,
        kBitwiseXor_Precedence     = 10,
        kBitwiseOr_Precedence      = 11,
        kLogicalAnd_Precedence     = 12,
        kLogicalXor_Precedence     = 13,
        kLogicalOr_Precedence      = 14,
        kTernary_Precedence        = 15,
        kAssignment_Precedence     = 16,
        kSequence_Precedence       = 17,
        kTopLevel_Precedence       = kSequence_Precedence
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
