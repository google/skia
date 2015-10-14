#include <string.h>
#include "tools/re2c/substr.h"
#include "tools/re2c/globals.h"

void
SubStr_out(const SubStr *s, FILE *o)
{
    unsigned int i;
    fwrite(s->str, s->len, 1, o);
    for (i=0; i<s->len; i++)
	if (s->str[i] == '\n')
	    oline++;
}

int
SubStr_eq(const SubStr *s1, const SubStr *s2)
{
    return (s1->len == s2->len && memcmp(s1->str, s2->str, s1->len) == 0);
}

void
Str_init(Str *r, const SubStr* s)
{
    SubStr_init(r, malloc(sizeof(char)*s->len), s->len);
    memcpy(r->str, s->str, s->len);
}

Str *
Str_new(const SubStr* s)
{
    Str *r = SubStr_new(malloc(sizeof(char)*s->len), s->len);
    memcpy(r->str, s->str, s->len);
    return r;
}

void
Str_copy(Str *r, Str* s)
{
    SubStr_init(r, s->str, s->len);
    s->str = NULL;
    s->len = 0;
}

Str *
Str_new_copy(Str* s)
{
    Str *r = SubStr_new(s->str, s->len);
    s->str = NULL;
    s->len = 0;
    return r;
}

Str *
Str_new_empty(void)
{
    return SubStr_new(NULL, 0);
}


void Str_delete(Str *s) {
    free(s->str);
    s->str = (char*)-1;
    s->len = (unsigned int)-1;
    free(s);
}
