#ifndef re2c_parse_h
#define re2c_parse_h

#include <stdio.h>
#include "tools/re2c/scanner.h"
#include "tools/re2c/re.h"

typedef struct Symbol {
    struct Symbol		*next;
    Str			name;
    RegExp		*re;
} Symbol;

void Symbol_init(Symbol *, const SubStr*);
static Symbol *Symbol_new(const SubStr*);
Symbol *Symbol_find(const SubStr*);

void line_source(FILE *, unsigned int);
void parse(FILE *, FILE *);

static Symbol *
Symbol_new(const SubStr *str)
{
    Symbol *r = malloc(sizeof(Symbol));
    Symbol_init(r, str);
    return r;
}

#endif
