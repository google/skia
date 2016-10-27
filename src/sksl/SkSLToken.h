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
        STRUCT,
        LAYOUT,
        DIRECTIVE,
        PRECISION,
        INVALID_TOKEN
    };

    static std::string OperatorName(Kind kind) {
        switch (kind) {
            case Token::PLUS:         return "+";
            case Token::MINUS:        return "-";
            case Token::STAR:         return "*";
            case Token::SLASH:        return "/";
            case Token::PERCENT:      return "%";
            case Token::SHL:          return "<<";
            case Token::SHR:          return ">>";
            case Token::LOGICALNOT:   return "!";
            case Token::LOGICALAND:   return "&&";
            case Token::LOGICALOR:    return "||";
            case Token::LOGICALXOR:   return "^^";
            case Token::BITWISENOT:   return "~";
            case Token::BITWISEAND:   return "&";
            case Token::BITWISEOR:    return "|";
            case Token::BITWISEXOR:   return "^";
            case Token::EQ:           return "=";
            case Token::EQEQ:         return "==";
            case Token::NEQ:          return "!=";
            case Token::LT:           return "<";
            case Token::GT:           return ">";
            case Token::LTEQ:         return "<=";
            case Token::GTEQ:         return ">=";
            case Token::PLUSEQ:       return "+=";
            case Token::MINUSEQ:      return "-=";
            case Token::STAREQ:       return "*=";
            case Token::SLASHEQ:      return "/=";
            case Token::PERCENTEQ:    return "%=";
            case Token::SHLEQ:        return "<<=";
            case Token::SHREQ:        return ">>=";
            case Token::LOGICALANDEQ: return "&&=";
            case Token::LOGICALOREQ:  return "||=";
            case Token::LOGICALXOREQ: return "^^=";
            case Token::BITWISEANDEQ: return "&=";
            case Token::BITWISEOREQ:  return "|=";
            case Token::BITWISEXOREQ: return "^=";
            case Token::PLUSPLUS:     return "++";
            case Token::MINUSMINUS:   return "--";
            default:
                ABORT("unsupported operator: %d\n", kind); 
        }        
    }

    Token() {
    }

    Token(Position position, Kind kind, std::string text)
    : fPosition(position)
    , fKind(kind)
    , fText(std::move(text)) {}

    Position fPosition;
    Kind fKind;
    // will be the empty string unless the token has variable text content (identifiers, numeric
    // literals, and directives)
    std::string fText;
};

} // namespace
#endif
