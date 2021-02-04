/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLOperators.h"

namespace SkSL {
namespace Operators {

Precedence GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_STAR:         [[fallthrough]];
        case Token::Kind::TK_SLASH:        [[fallthrough]];
        case Token::Kind::TK_PERCENT:      return kMultiplicative_Precedence;
        case Token::Kind::TK_PLUS:         [[fallthrough]];
        case Token::Kind::TK_MINUS:        return kAdditive_Precedence;
        case Token::Kind::TK_SHL:          [[fallthrough]];
        case Token::Kind::TK_SHR:          return kShift_Precedence;
        case Token::Kind::TK_LT:           [[fallthrough]];
        case Token::Kind::TK_GT:           [[fallthrough]];
        case Token::Kind::TK_LTEQ:         [[fallthrough]];
        case Token::Kind::TK_GTEQ:         return kRelational_Precedence;
        case Token::Kind::TK_EQEQ:         [[fallthrough]];
        case Token::Kind::TK_NEQ:          return kEquality_Precedence;
        case Token::Kind::TK_BITWISEAND:   return kBitwiseAnd_Precedence;
        case Token::Kind::TK_BITWISEXOR:   return kBitwiseXor_Precedence;
        case Token::Kind::TK_BITWISEOR:    return kBitwiseOr_Precedence;
        case Token::Kind::TK_LOGICALAND:   return kLogicalAnd_Precedence;
        case Token::Kind::TK_LOGICALXOR:   return kLogicalXor_Precedence;
        case Token::Kind::TK_LOGICALOR:    return kLogicalOr_Precedence;
        case Token::Kind::TK_EQ:           [[fallthrough]];
        case Token::Kind::TK_PLUSEQ:       [[fallthrough]];
        case Token::Kind::TK_MINUSEQ:      [[fallthrough]];
        case Token::Kind::TK_STAREQ:       [[fallthrough]];
        case Token::Kind::TK_SLASHEQ:      [[fallthrough]];
        case Token::Kind::TK_PERCENTEQ:    [[fallthrough]];
        case Token::Kind::TK_SHLEQ:        [[fallthrough]];
        case Token::Kind::TK_SHREQ:        [[fallthrough]];
        case Token::Kind::TK_BITWISEANDEQ: [[fallthrough]];
        case Token::Kind::TK_BITWISEXOREQ: [[fallthrough]];
        case Token::Kind::TK_BITWISEOREQ:  return kAssignment_Precedence;
        case Token::Kind::TK_COMMA:        return kSequence_Precedence;
        default: SK_ABORT("unsupported binary operator");
    }
}


const char* OperatorName(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_PLUS:         return "+";
        case Token::Kind::TK_MINUS:        return "-";
        case Token::Kind::TK_STAR:         return "*";
        case Token::Kind::TK_SLASH:        return "/";
        case Token::Kind::TK_PERCENT:      return "%";
        case Token::Kind::TK_SHL:          return "<<";
        case Token::Kind::TK_SHR:          return ">>";
        case Token::Kind::TK_LOGICALNOT:   return "!";
        case Token::Kind::TK_LOGICALAND:   return "&&";
        case Token::Kind::TK_LOGICALOR:    return "||";
        case Token::Kind::TK_LOGICALXOR:   return "^^";
        case Token::Kind::TK_BITWISENOT:   return "~";
        case Token::Kind::TK_BITWISEAND:   return "&";
        case Token::Kind::TK_BITWISEOR:    return "|";
        case Token::Kind::TK_BITWISEXOR:   return "^";
        case Token::Kind::TK_EQ:           return "=";
        case Token::Kind::TK_EQEQ:         return "==";
        case Token::Kind::TK_NEQ:          return "!=";
        case Token::Kind::TK_LT:           return "<";
        case Token::Kind::TK_GT:           return ">";
        case Token::Kind::TK_LTEQ:         return "<=";
        case Token::Kind::TK_GTEQ:         return ">=";
        case Token::Kind::TK_PLUSEQ:       return "+=";
        case Token::Kind::TK_MINUSEQ:      return "-=";
        case Token::Kind::TK_STAREQ:       return "*=";
        case Token::Kind::TK_SLASHEQ:      return "/=";
        case Token::Kind::TK_PERCENTEQ:    return "%=";
        case Token::Kind::TK_SHLEQ:        return "<<=";
        case Token::Kind::TK_SHREQ:        return ">>=";
        case Token::Kind::TK_BITWISEANDEQ: return "&=";
        case Token::Kind::TK_BITWISEOREQ:  return "|=";
        case Token::Kind::TK_BITWISEXOREQ: return "^=";
        case Token::Kind::TK_PLUSPLUS:     return "++";
        case Token::Kind::TK_MINUSMINUS:   return "--";
        case Token::Kind::TK_COMMA:        return ",";
        default:
            SK_ABORT("unsupported operator: %d\n", (int) op);
    }
}


bool IsAssignment(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_EQ:           [[fallthrough]];
        case Token::Kind::TK_PLUSEQ:       [[fallthrough]];
        case Token::Kind::TK_MINUSEQ:      [[fallthrough]];
        case Token::Kind::TK_STAREQ:       [[fallthrough]];
        case Token::Kind::TK_SLASHEQ:      [[fallthrough]];
        case Token::Kind::TK_PERCENTEQ:    [[fallthrough]];
        case Token::Kind::TK_SHLEQ:        [[fallthrough]];
        case Token::Kind::TK_SHREQ:        [[fallthrough]];
        case Token::Kind::TK_BITWISEOREQ:  [[fallthrough]];
        case Token::Kind::TK_BITWISEXOREQ: [[fallthrough]];
        case Token::Kind::TK_BITWISEANDEQ:
            return true;
        default:
            return false;
    }
}

Token::Kind RemoveAssignment(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_PLUSEQ:       return Token::Kind::TK_PLUS;
        case Token::Kind::TK_MINUSEQ:      return Token::Kind::TK_MINUS;
        case Token::Kind::TK_STAREQ:       return Token::Kind::TK_STAR;
        case Token::Kind::TK_SLASHEQ:      return Token::Kind::TK_SLASH;
        case Token::Kind::TK_PERCENTEQ:    return Token::Kind::TK_PERCENT;
        case Token::Kind::TK_SHLEQ:        return Token::Kind::TK_SHL;
        case Token::Kind::TK_SHREQ:        return Token::Kind::TK_SHR;
        case Token::Kind::TK_BITWISEOREQ:  return Token::Kind::TK_BITWISEOR;
        case Token::Kind::TK_BITWISEXOREQ: return Token::Kind::TK_BITWISEXOR;
        case Token::Kind::TK_BITWISEANDEQ: return Token::Kind::TK_BITWISEAND;
        default: return op;
    }
}

}  // namespace Operators
}  // namespace SkSL
