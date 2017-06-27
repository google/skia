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
        WHITESPACE,
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
        ARROW,
        COLONCOLON,
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
        BUFFER,
        HASSIDEEFFECTS,
        STRUCT,
        LAYOUT,
        DIRECTIVE,
        SECTION,
        PRECISION,
        LOCATION,
        OFFSET,
        BINDING,
        INDEX,
        SET,
        BUILTIN,
        INPUT_ATTACHMENT_INDEX,
        ORIGIN_UPPER_LEFT,
        OVERRIDE_COVERAGE,
        BLEND_SUPPORT_ALL_EQUATIONS,
        PUSH_CONSTANT,
        POINTS,
        LINES,
        LINE_STRIP,
        LINES_ADJACENCY,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLES_ADJACENCY,
        MAX_VERTICES,
        INVOCATIONS,
        WHEN,
        KEY,
        INVALID_TOKEN
    };

    static String OperatorName(Kind kind) {
        switch (kind) {
            case Token::PLUS:         return String("+");
            case Token::MINUS:        return String("-");
            case Token::STAR:         return String("*");
            case Token::SLASH:        return String("/");
            case Token::PERCENT:      return String("%");
            case Token::SHL:          return String("<<");
            case Token::SHR:          return String(">>");
            case Token::LOGICALNOT:   return String("!");
            case Token::LOGICALAND:   return String("&&");
            case Token::LOGICALOR:    return String("||");
            case Token::LOGICALXOR:   return String("^^");
            case Token::BITWISENOT:   return String("~");
            case Token::BITWISEAND:   return String("&");
            case Token::BITWISEOR:    return String("|");
            case Token::BITWISEXOR:   return String("^");
            case Token::EQ:           return String("=");
            case Token::EQEQ:         return String("==");
            case Token::NEQ:          return String("!=");
            case Token::LT:           return String("<");
            case Token::GT:           return String(">");
            case Token::LTEQ:         return String("<=");
            case Token::GTEQ:         return String(">=");
            case Token::PLUSEQ:       return String("+=");
            case Token::MINUSEQ:      return String("-=");
            case Token::STAREQ:       return String("*=");
            case Token::SLASHEQ:      return String("/=");
            case Token::PERCENTEQ:    return String("%=");
            case Token::SHLEQ:        return String("<<=");
            case Token::SHREQ:        return String(">>=");
            case Token::LOGICALANDEQ: return String("&&=");
            case Token::LOGICALOREQ:  return String("||=");
            case Token::LOGICALXOREQ: return String("^^=");
            case Token::BITWISEANDEQ: return String("&=");
            case Token::BITWISEOREQ:  return String("|=");
            case Token::BITWISEXOREQ: return String("^=");
            case Token::PLUSPLUS:     return String("++");
            case Token::MINUSMINUS:   return String("--");
            case Token::COMMA:        return String(",");
            default:
                ABORT("unsupported operator: %d\n", kind);
        }
    }

    Token() {
    }

    Token(Position position, Kind kind, String text)
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
    String fText;
};

} // namespace
#endif
