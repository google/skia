/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLOperators.h"

namespace SkSL {

Operator::Precedence Operator::getBinaryPrecedence() const {
    switch (fKind) {
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


const char* Operator::operatorName() const {
    switch (fKind) {
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
    switch (fKind) {
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
    switch (fKind) {
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

}  // namespace SkSL
