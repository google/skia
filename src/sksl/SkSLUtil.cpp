/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLUtil.h"

#include "src/sksl/SkSLStringStream.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace SkSL {

#ifdef SKSL_STANDALONE
StandaloneShaderCaps standaloneCaps;
#endif

void sksl_abort() {
#ifdef SKSL_STANDALONE
    abort();
#else
    sk_abort_no_print();
    exit(1);
#endif
}

void write_stringstream(const StringStream& s, OutputStream& out) {
    out.write(s.str().c_str(), s.str().size());
}

bool is_assignment(Token::Kind op) {
    switch (op) {
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::BITWISEOREQ:  // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALANDEQ:
            return true;
        default:
            return false;
    }
}

Token::Kind remove_assignment(Token::Kind op) {
    switch (op) {
        case Token::PLUSEQ:       return Token::PLUS;
        case Token::MINUSEQ:      return Token::MINUS;
        case Token::STAREQ:       return Token::STAR;
        case Token::SLASHEQ:      return Token::SLASH;
        case Token::PERCENTEQ:    return Token::PERCENT;
        case Token::SHLEQ:        return Token::SHL;
        case Token::SHREQ:        return Token::SHR;
        case Token::BITWISEOREQ:  return Token::BITWISEOR;
        case Token::BITWISEXOREQ: return Token::BITWISEXOR;
        case Token::BITWISEANDEQ: return Token::BITWISEAND;
        case Token::LOGICALOREQ:  return Token::LOGICALOR;
        case Token::LOGICALXOREQ: return Token::LOGICALXOR;
        case Token::LOGICALANDEQ: return Token::LOGICALAND;
        default: return op;
    }
}

} // namespace
