/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SKSL_Lexer
#define SKSL_Lexer

#include <inttypes.h>
#include <stddef.h>

#undef IN
#undef OUT
#undef INVALID

namespace SkSL {

struct Token {
    enum Kind {
        END_OF_FILE,
        INT_LITERAL,
        FLOAT_LITERAL,
        TRUE_LITERAL,
        FALSE_LITERAL,
        IF,
        STATIC_IF,
        ELSE,
        FOR,
        WHILE,
        DO,
        SWITCH,
        STATIC_SWITCH,
        CASE,
        DEFAULT,
        BREAK,
        CONTINUE,
        DISCARD,
        RETURN,
        IN,
        OUT,
        INOUT,
        UNIFORM,
        CONST,
        LOWP,
        MEDIUMP,
        HIGHP,
        FLAT,
        NOPERSPECTIVE,
        READONLY,
        WRITEONLY,
        COHERENT,
        VOLATILE,
        RESTRICT,
        BUFFER,
        HASSIDEEFFECTS,
        STRUCT,
        LAYOUT,
        PRECISION,
        IDENTIFIER,
        DIRECTIVE,
        SECTION,
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,
        DOT,
        COMMA,
        PLUSPLUS,
        MINUSMINUS,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT,
        SHL,
        SHR,
        BITWISEOR,
        BITWISEXOR,
        BITWISEAND,
        BITWISENOT,
        LOGICALOR,
        LOGICALXOR,
        LOGICALAND,
        LOGICALNOT,
        QUESTION,
        COLON,
        EQ,
        EQEQ,
        NEQ,
        GT,
        LT,
        GTEQ,
        LTEQ,
        PLUSEQ,
        MINUSEQ,
        STAREQ,
        SLASHEQ,
        PERCENTEQ,
        SHLEQ,
        SHREQ,
        BITWISEOREQ,
        BITWISEXOREQ,
        BITWISEANDEQ,
        LOGICALOREQ,
        LOGICALXOREQ,
        LOGICALANDEQ,
        SEMICOLON,
        ARROW,
        COLONCOLON,
        WHITESPACE,
        LINE_COMMENT,
        BLOCK_COMMENT,
        INVALID
    };

    Token()
    : fKind(Kind::INVALID)
    , fOffset(-1)
    , fLength(-1) {}

    Token(Kind kind, int offset, int length)
    : fKind(kind)
    , fOffset(offset)
    , fLength(length) {}

    Kind fKind;
    int fOffset;
    int fLength;
};

class Lexer {
public:

    void start(const char* text, size_t length) {
        fText = text;
        fLength = length;
        fOffset = 0;
    }

    Token next();

private:
    const char* fText;
    int fLength;
    int fOffset;
};

} // namespace

#endif
