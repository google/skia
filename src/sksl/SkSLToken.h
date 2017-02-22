/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_TOKEN
#define SKSL_TOKEN

#include "SkSLPosition.h"
#include "SkSLUtil.h"
 
namespace SkSL {

#undef IN
#undef OUT
#undef CONST

/**
 * Represents a lexical analysis token. Token is generally only used during the parse process, but
 * Token::Kind is also used to represent operator kinds.
 */
struct Token {
    enum Kind {
        END_OF_FILE,
        IDENTIFIER,
        INT_LITERAL,
        FLOAT_LITERAL,
        TRUE_LITERAL,
        FALSE_LITERAL,
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
        IF,
        ELSE,
        FOR,
        WHILE,
        DO,
        SWITCH,
        CASE,
        DEFAULT,
        RETURN,
        BREAK,
        CONTINUE,
        DISCARD,
        IN,
        OUT,
        INOUT,
        CONST,
        LOWP,
        MEDIUMP,
        HIGHP,
        UNIFORM,
        FLAT,
        NOPERSPECTIVE,
        READONLY,
        WRITEONLY,
        COHERENT,
        VOLATILE,
        RESTRICT,
        STRUCT,
        LAYOUT,
        DIRECTIVE,
        PRECISION,
        INVALID_TOKEN
    };

    static SkString OperatorName(Kind kind) {
        switch (kind) {
            case Token::PLUS:         return SkString("+");
            case Token::MINUS:        return SkString("-");
            case Token::STAR:         return SkString("*");
            case Token::SLASH:        return SkString("/");
            case Token::PERCENT:      return SkString("%");
            case Token::SHL:          return SkString("<<");
            case Token::SHR:          return SkString(">>");
            case Token::LOGICALNOT:   return SkString("!");
            case Token::LOGICALAND:   return SkString("&&");
            case Token::LOGICALOR:    return SkString("||");
            case Token::LOGICALXOR:   return SkString("^^");
            case Token::BITWISENOT:   return SkString("~");
            case Token::BITWISEAND:   return SkString("&");
            case Token::BITWISEOR:    return SkString("|");
            case Token::BITWISEXOR:   return SkString("^");
            case Token::EQ:           return SkString("=");
            case Token::EQEQ:         return SkString("==");
            case Token::NEQ:          return SkString("!=");
            case Token::LT:           return SkString("<");
            case Token::GT:           return SkString(">");
            case Token::LTEQ:         return SkString("<=");
            case Token::GTEQ:         return SkString(">=");
            case Token::PLUSEQ:       return SkString("+=");
            case Token::MINUSEQ:      return SkString("-=");
            case Token::STAREQ:       return SkString("*=");
            case Token::SLASHEQ:      return SkString("/=");
            case Token::PERCENTEQ:    return SkString("%=");
            case Token::SHLEQ:        return SkString("<<=");
            case Token::SHREQ:        return SkString(">>=");
            case Token::LOGICALANDEQ: return SkString("&&=");
            case Token::LOGICALOREQ:  return SkString("||=");
            case Token::LOGICALXOREQ: return SkString("^^=");
            case Token::BITWISEANDEQ: return SkString("&=");
            case Token::BITWISEOREQ:  return SkString("|=");
            case Token::BITWISEXOREQ: return SkString("^=");
            case Token::PLUSPLUS:     return SkString("++");
            case Token::MINUSMINUS:   return SkString("--");
            default:
                ABORT("unsupported operator: %d\n", kind); 
        }        
    }

    Token() {
    }

    Token(Position position, Kind kind, SkString text)
    : fPosition(position)
    , fKind(kind)
    , fText(std::move(text)) {}

    static bool IsAssignment(Token::Kind op) {
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

    Position fPosition;
    Kind fKind;
    // will be the empty string unless the token has variable text content (identifiers, numeric
    // literals, and directives)
    SkString fText;
};

} // namespace
#endif
