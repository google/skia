/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLUtil.h"

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLStringStream.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace SkSL {

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU
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
        case Token::Kind::TK_BITWISEANDEQ: // fall through
        case Token::Kind::TK_LOGICALOREQ:  // fall through
        case Token::Kind::TK_LOGICALXOREQ: // fall through
        case Token::Kind::TK_LOGICALANDEQ:
            return true;
        default:
            return false;
    }
}

Token::Kind remove_assignment(Token::Kind op) {
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
        case Token::Kind::TK_LOGICALOREQ:  return Token::Kind::TK_LOGICALOR;
        case Token::Kind::TK_LOGICALXOREQ: return Token::Kind::TK_LOGICALXOR;
        case Token::Kind::TK_LOGICALANDEQ: return Token::Kind::TK_LOGICALAND;
        default: return op;
    }
}

#if !defined(SKSL_STANDALONE)
bool type_to_grsltype(const Context& context, const Type& type, GrSLType* outType) {
    if (type == *context.fFloat_Type)    { *outType = kFloat_GrSLType;    return true; }
    if (type == *context.fHalf_Type)     { *outType = kHalf_GrSLType;     return true; }
    if (type == *context.fFloat2_Type)   { *outType = kFloat2_GrSLType;   return true; }
    if (type == *context.fHalf2_Type)    { *outType = kHalf2_GrSLType;    return true; }
    if (type == *context.fFloat3_Type)   { *outType = kFloat3_GrSLType;   return true; }
    if (type == *context.fHalf3_Type)    { *outType = kHalf3_GrSLType;    return true; }
    if (type == *context.fFloat4_Type)   { *outType = kFloat4_GrSLType;   return true; }
    if (type == *context.fHalf4_Type)    { *outType = kHalf4_GrSLType;    return true; }
    if (type == *context.fFloat2x2_Type) { *outType = kFloat2x2_GrSLType; return true; }
    if (type == *context.fHalf2x2_Type)  { *outType = kHalf2x2_GrSLType;  return true; }
    if (type == *context.fFloat3x3_Type) { *outType = kFloat3x3_GrSLType; return true; }
    if (type == *context.fHalf3x3_Type)  { *outType = kHalf3x3_GrSLType;  return true; }
    if (type == *context.fFloat4x4_Type) { *outType = kFloat4x4_GrSLType; return true; }
    if (type == *context.fHalf4x4_Type)  { *outType = kHalf4x4_GrSLType;  return true; }
    if (type == *context.fVoid_Type)     { *outType = kVoid_GrSLType;     return true; }
    return false;
}
#endif

}  // namespace SkSL
