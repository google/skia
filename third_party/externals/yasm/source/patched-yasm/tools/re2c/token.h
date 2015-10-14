#ifndef re2c_token_h
#define	re2c_token_h

#include "substr.h"

typedef struct Token {
    Str			text;
    unsigned int	line;
} Token;

static void Token_init(Token *, SubStr, unsigned int);
static Token *Token_new(SubStr, unsigned int);

static void
Token_init(Token *r, SubStr t, unsigned int l)
{
    Str_copy(&r->text, &t);
    r->line = l;
}

static Token *
Token_new(SubStr t, unsigned int l)
{
    Token *r = malloc(sizeof(Token));
    Str_init(&r->text, &t);
    r->line = l;
    return r;
}

#endif
