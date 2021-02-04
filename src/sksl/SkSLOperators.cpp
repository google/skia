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
        case Token::Kind::TK_STAR:         // fall through
        case Token::Kind::TK_SLASH:        // fall through
        case Token::Kind::TK_PERCENT:      return Precedence::kMultiplicative;
        case Token::Kind::TK_PLUS:         // fall through
        case Token::Kind::TK_MINUS:        return Precedence::kAdditive;
        case Token::Kind::TK_SHL:          // fall through
        case Token::Kind::TK_SHR:          return Precedence::kShift;
        case Token::Kind::TK_LT:           // fall through
        case Token::Kind::TK_GT:           // fall through
        case Token::Kind::TK_LTEQ:         // fall through
        case Token::Kind::TK_GTEQ:         return Precedence::kRelational;
        case Token::Kind::TK_EQEQ:         // fall through
        case Token::Kind::TK_NEQ:          return Precedence::kEquality;
        case Token::Kind::TK_BITWISEAND:   return Precedence::kBitwiseAnd;
        case Token::Kind::TK_BITWISEXOR:   return Precedence::kBitwiseXor;
        case Token::Kind::TK_BITWISEOR:    return Precedence::kBitwiseOr;
        case Token::Kind::TK_LOGICALAND:   return Precedence::kLogicalAnd;
        case Token::Kind::TK_LOGICALXOR:   return Precedence::kLogicalXor;
        case Token::Kind::TK_LOGICALOR:    return Precedence::kLogicalOr;
        case Token::Kind::TK_EQ:           // fall through
        case Token::Kind::TK_PLUSEQ:       // fall through
        case Token::Kind::TK_MINUSEQ:      // fall through
        case Token::Kind::TK_STAREQ:       // fall through
        case Token::Kind::TK_SLASHEQ:      // fall through
        case Token::Kind::TK_PERCENTEQ:    // fall through
        case Token::Kind::TK_SHLEQ:        // fall through
        case Token::Kind::TK_SHREQ:        // fall through
        case Token::Kind::TK_BITWISEANDEQ: // fall through
        case Token::Kind::TK_BITWISEXOREQ: // fall through
        case Token::Kind::TK_BITWISEOREQ:  return Precedence::kAssignment;
        case Token::Kind::TK_COMMA:        return Precedence::kSequence;
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
        case Token::Kind::TK_EQ:           // fall through
        case Token::Kind::TK_PLUSEQ:       // fall through
        case Token::Kind::TK_MINUSEQ:      // fall through
        case Token::Kind::TK_STAREQ:       // fall through
        case Token::Kind::TK_SLASHEQ:      // fall through
        case Token::Kind::TK_PERCENTEQ:    // fall through
        case Token::Kind::TK_SHLEQ:        // fall through
        case Token::Kind::TK_SHREQ:        // fall through
        case Token::Kind::TK_BITWISEOREQ:  // fall through
        case Token::Kind::TK_BITWISEXOREQ: // fall through
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
