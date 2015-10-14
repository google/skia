#ifndef _scanner_h
#define	_scanner_h

#include <stdio.h>
#include "tools/re2c/token.h"

typedef struct Scanner {
    FILE		*in;
    unsigned char	*bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof;
    unsigned int	tchar, tline, cline;
} Scanner;

void Scanner_init(Scanner*, FILE *);
static Scanner *Scanner_new(FILE *);

int Scanner_echo(Scanner*, FILE *);
int Scanner_scan(Scanner*);
void Scanner_fatal(Scanner*, const char*);
static SubStr Scanner_token(Scanner*);
static unsigned int Scanner_line(Scanner*);

static SubStr
Scanner_token(Scanner *s)
{
    SubStr r;
    SubStr_init_u(&r, s->tok, s->cur - s->tok);
    return r;
}

static unsigned int
Scanner_line(Scanner *s)
{
    return s->cline;
}

static Scanner *
Scanner_new(FILE *i)
{
    Scanner *r = malloc(sizeof(Scanner));
    Scanner_init(r, i);
    return r;
}

#endif
