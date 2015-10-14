/* nasmlib.c    library routines for the Netwide Assembler
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the licence given in the file "Licence"
 * distributed in the NASM archive.
 */
#include <util.h>
#include <libyasm/coretype.h>
#include <libyasm/intnum.h>
#include <ctype.h>

#include "nasm.h"
#include "nasmlib.h"
/*#include "insns.h"*/          /* For MAX_KEYWORD */

#define lib_isnumchar(c)   ( isalnum(c) || (c) == '$')
#define numvalue(c)  ((c)>='a' ? (c)-'a'+10 : (c)>='A' ? (c)-'A'+10 : (c)-'0')

yasm_intnum *nasm_readnum (char *str, int *error) 
{
    char *r = str, *q, *p;
    long radix;
    yasm_intnum *intn;
    char save;
    int digit;
    int sign = 0;

    *error = FALSE;

    while (isspace(*r)) r++;           /* find start of number */

    /*
     * If the number came from make_tok_num (as a result of an %assign), it
     * might have a '-' built into it (rather than in a preceeding token).
     */
    if (*r == '-')
    {
        r++;
        sign = 1;
    }

    q = r;

    while (lib_isnumchar(*q)) q++;     /* find end of number */

    /*
     * If it begins 0x, 0X or $, or ends in H, it's in hex. if it
     * ends in Q, it's octal. if it ends in B, it's binary.
     * Otherwise, it's ordinary decimal.
     */
    if (*r=='0' && (r[1]=='x' || r[1]=='X'))
        radix = 16, r += 2;
    else if (*r=='$')
        radix = 16, r++;
    else if (q[-1]=='H' || q[-1]=='h')
        radix = 16 , q--;
    else if (q[-1]=='Q' || q[-1]=='q' || q[-1]=='O' || q[-1]=='o')
        radix = 8 , q--;
    else if (q[-1]=='B' || q[-1]=='b')
        radix = 2 , q--;
    else
        radix = 10;

    /*
     * If this number has been found for us by something other than
     * the ordinary scanners, then it might be malformed by having
     * nothing between the prefix and the suffix. Check this case
     * now.
     */
    if (r >= q) {
        *error = TRUE;
        return yasm_intnum_create_uint(0);
    }

    /* Check for valid number of that radix */
    p = r;
    while (*p && p < q) {
        if (*p<'0' || (*p>'9' && *p<'A') || (digit = numvalue(*p)) >= radix) 
        {
            *error = TRUE;
            return yasm_intnum_create_uint(0);
        }
        p++;
    }

    /* Use intnum to actually do the conversion */
    save = *q;
    *q = '\0';
    switch (radix) {
        case 2:
            intn = yasm_intnum_create_bin(r);
            break;
        case 8:
            intn = yasm_intnum_create_oct(r);
            break;
        case 10:
            intn = yasm_intnum_create_dec(r);
            break;
        case 16:
            intn = yasm_intnum_create_hex(r);
            break;
        default:
            *error = TRUE;
            intn = yasm_intnum_create_uint(0);
            break;
    }
    *q = save;

    if (sign)
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);
    return intn;
}

yasm_intnum *nasm_readstrnum (char *str, size_t length, int *warn) 
{
    char save;
    yasm_intnum *intn;

    *warn = FALSE;

    save = str[length];
    str[length] = '\0';
    intn = yasm_intnum_create_charconst_nasm(str);
    str[length] = save;

    return intn;
}

static char *file_name = NULL;
static long line_number = 0;

char *nasm_src_set_fname(char *newname) 
{
    char *oldname = file_name;
    file_name = newname;
    return oldname;
}

char *nasm_src_get_fname(void)
{
    return file_name;
}

long nasm_src_set_linnum(long newline) 
{
    long oldline = line_number;
    line_number = newline;
    return oldline;
}

long nasm_src_get_linnum(void) 
{
    return line_number;
}

int nasm_src_get(long *xline, char **xname) 
{
    if (!file_name || !*xname || strcmp(*xname, file_name)) 
    {
        nasm_free(*xname);
        *xname = file_name ? nasm_strdup(file_name) : NULL;
        *xline = line_number;
        return -2;
    }
    if (*xline != line_number) 
    {
        long tmp = line_number - *xline;
        *xline = line_number;
        return tmp;
    }
    return 0;
}

void nasm_quote(char **str) 
{
    size_t ln=strlen(*str);
    char q=(*str)[0];
    char *p;
    if (ln>1 && (*str)[ln-1]==q && (q=='"' || q=='\''))
        return;
    q = '"';
    if (strchr(*str,q))
        q = '\'';
    p = nasm_malloc(ln+3);
    strcpy(p+1, *str);
    nasm_free(*str);
    p[ln+1] = p[0] = q;
    p[ln+2] = 0;
    *str = p;
}
    
char *nasm_strcat(const char *one, const char *two) 
{
    char *rslt;
    size_t l1=strlen(one);
    rslt = nasm_malloc(l1+strlen(two)+1);
    strcpy(rslt, one);
    strcpy(rslt+l1, two);
    return rslt;
}
