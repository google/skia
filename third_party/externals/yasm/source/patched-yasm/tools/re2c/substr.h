#ifndef re2c_substr_h
#define re2c_substr_h

#include <stdio.h>
#include <stdlib.h>
#include "tools/re2c/basics.h"

struct SubStr {
    char		*str;
    unsigned int	len;
};

typedef struct SubStr SubStr;

int SubStr_eq(const SubStr *, const SubStr *);

static void SubStr_init_u(SubStr*, unsigned char*, unsigned int);
static SubStr *SubStr_new_u(unsigned char*, unsigned int);

static void SubStr_init(SubStr*, char*, unsigned int);
static SubStr *SubStr_new(char*, unsigned int);

static void SubStr_copy(SubStr*, const SubStr*);
static SubStr *SubStr_new_copy(const SubStr*);

void SubStr_out(const SubStr*, FILE *);
#define SubStr_delete(x)    free(x)

typedef struct SubStr Str;

void Str_init(Str*, const SubStr*);
Str *Str_new(const SubStr*);

void Str_copy(Str*, Str*);
Str *Str_new_copy(Str*);

Str *Str_new_empty(void);
void Str_destroy(Str *);
void Str_delete(Str *);

static void
SubStr_init_u(SubStr *r, unsigned char *s, unsigned int l)
{
    r->str = (char*)s;
    r->len = l;
}

static SubStr *
SubStr_new_u(unsigned char *s, unsigned int l)
{
    SubStr *r = malloc(sizeof(SubStr));
    r->str = (char*)s;
    r->len = l;
    return r;
}

static void
SubStr_init(SubStr *r, char *s, unsigned int l)
{
    r->str = s;
    r->len = l;
}

static SubStr *
SubStr_new(char *s, unsigned int l)
{
    SubStr *r = malloc(sizeof(SubStr));
    r->str = s;
    r->len = l;
    return r;
}

static void
SubStr_copy(SubStr *r, const SubStr *s)
{
    r->str = s->str;
    r->len = s->len;
}

static SubStr *
SubStr_new_copy(const SubStr *s)
{
    SubStr *r = malloc(sizeof(SubStr));
    r->str = s->str;
    r->len = s->len;
    return r;
}

#endif
