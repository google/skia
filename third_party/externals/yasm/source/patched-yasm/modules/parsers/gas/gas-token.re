/*
 * GAS-compatible re2c lexer
 *
 *  Copyright (C) 2005-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <util.h>

#include <libyasm.h>

#include "modules/parsers/gas/gas-parser.h"


#define BSIZE   8192

#define YYCURSOR        cursor
#define YYLIMIT         (s->lim)
#define YYMARKER        (s->ptr)
#define YYFILL(n)       {cursor = fill(parser_gas, cursor);}

#define RETURN(i)       do {s->cur = cursor; parser_gas->tokch = s->tok[0]; \
                         return i;} while (0)

#define SCANINIT()      {s->tok = cursor;}

#define TOK             ((char *)s->tok)
#define TOKLEN          (size_t)(cursor-s->tok)

/* Bridge function to convert byte-oriented parser with line-oriented
 * preprocessor.
 */
static size_t
preproc_input(yasm_parser_gas *parser_gas, /*@out@*/ YYCTYPE *buf,
              size_t max_size)
{
    size_t tot=0;
    while (max_size > 0) {
        size_t n;

        if (!parser_gas->line) {
            parser_gas->line = yasm_preproc_get_line(parser_gas->preproc);
            if (!parser_gas->line)
                return tot; /* EOF */
            parser_gas->linepos = parser_gas->line;
            parser_gas->lineleft = strlen(parser_gas->line) + 1;
            parser_gas->line[parser_gas->lineleft-1] = '\n';
        }

        n = parser_gas->lineleft<max_size ? parser_gas->lineleft : max_size;
        strncpy((char *)buf+tot, parser_gas->linepos, n);

        if (n == parser_gas->lineleft) {
            yasm_xfree(parser_gas->line);
            parser_gas->line = NULL;
        } else {
            parser_gas->lineleft -= n;
            parser_gas->linepos += n;
        }

        tot += n;
        max_size -= n;
    }
    return tot;
}
#if 0
static size_t
fill_input(void *d, unsigned char *buf, size_t max)
{
    return yasm_preproc_input((yasm_preproc *)d, (char *)buf, max);
}
#endif
static YYCTYPE *
fill(yasm_parser_gas *parser_gas, YYCTYPE *cursor)
{
    yasm_scanner *s = &parser_gas->s;
    int first = 0;
    if(!s->eof){
        size_t cnt = s->tok - s->bot;
        if(cnt){
            memmove(s->bot, s->tok, (size_t)(s->lim - s->tok));
            s->tok = s->bot;
            s->ptr -= cnt;
            cursor -= cnt;
            s->lim -= cnt;
        }
        if (!s->bot)
            first = 1;
        if((s->top - s->lim) < BSIZE){
            YYCTYPE *buf = yasm_xmalloc((size_t)(s->lim - s->bot) + BSIZE);
            memcpy(buf, s->tok, (size_t)(s->lim - s->tok));
            s->tok = buf;
            s->ptr = &buf[s->ptr - s->bot];
            cursor = &buf[cursor - s->bot];
            s->lim = &buf[s->lim - s->bot];
            s->top = &s->lim[BSIZE];
            if (s->bot)
                yasm_xfree(s->bot);
            s->bot = buf;
        }
        if((cnt = preproc_input(parser_gas, s->lim, BSIZE)) == 0) {
            s->eof = &s->lim[cnt]; *s->eof++ = '\n';
        }
        s->lim += cnt;
        if (first && parser_gas->save_input) {
            int i;
            YYCTYPE *saveline;
            parser_gas->save_last ^= 1;
            saveline = parser_gas->save_line[parser_gas->save_last];
            /* save next line into cur_line */
            for (i=0; i<79 && &s->tok[i] < s->lim && s->tok[i] != '\n'; i++)
                saveline[i] = s->tok[i];
            saveline[i] = '\0';
        }
    }
    return cursor;
}

static YYCTYPE *
save_line(yasm_parser_gas *parser_gas, YYCTYPE *cursor)
{
    yasm_scanner *s = &parser_gas->s;
    int i = 0;
    YYCTYPE *saveline;

    parser_gas->save_last ^= 1;
    saveline = parser_gas->save_line[parser_gas->save_last];

    /* save next line into cur_line */
    if ((YYLIMIT - YYCURSOR) < 80)
        YYFILL(80);
    for (i=0; i<79 && &cursor[i] < s->lim && cursor[i] != '\n'; i++)
        saveline[i] = cursor[i];
    saveline[i] = '\0';
    return cursor;
}

/* starting size of string buffer */
#define STRBUF_ALLOC_SIZE       128

/* string buffer used when parsing strings/character constants */
static YYCTYPE *strbuf = NULL;

/* length of strbuf (including terminating NULL character) */
static size_t strbuf_size = 0;

static void
strbuf_append(size_t count, YYCTYPE *cursor, yasm_scanner *s, int ch)
{
    if (count >= strbuf_size) {
        strbuf = yasm_xrealloc(strbuf, strbuf_size + STRBUF_ALLOC_SIZE);
        strbuf_size += STRBUF_ALLOC_SIZE;
    }
    strbuf[count] = ch;
}

/*!re2c
  any = [\000-\377];
  digit = [0-9];
  iletter = [a-zA-Z];
  bindigit = [01];
  octdigit = [0-7];
  hexdigit = [0-9a-fA-F];
  ws = [ \t\r];
  dquot = ["];
*/


int
gas_parser_lex(YYSTYPE *lvalp, yasm_parser_gas *parser_gas)
{
    yasm_scanner *s = &parser_gas->s;
    YYCTYPE *cursor = s->cur;
    size_t count;
    YYCTYPE savech;

    /* Handle one token of lookahead */
    if (parser_gas->peek_token != NONE) {
        int tok = parser_gas->peek_token;
        *lvalp = parser_gas->peek_tokval;  /* structure copy */
        parser_gas->tokch = parser_gas->peek_tokch;
        parser_gas->peek_token = NONE;
        return tok;
    }

    /* Catch EOF */
    if (s->eof && cursor == s->eof)
        return 0;

    /* Jump to proper "exclusive" states */
    switch (parser_gas->state) {
        case COMMENT:
            goto comment;
        case SECTION_DIRECTIVE:
            goto section_directive;
        case NASM_FILENAME:
            goto nasm_filename;
        default:
            break;
    }

scan:
    SCANINIT();

    /*!re2c
        /* standard decimal integer */
        ([1-9] digit*) | "0" {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->intn = yasm_intnum_create_dec(TOK);
            s->tok[TOKLEN] = savech;
            RETURN(INTNUM);
        }

        /* 0b10010011 - binary number */
        '0b' bindigit+ {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->intn = yasm_intnum_create_bin(TOK+2);
            s->tok[TOKLEN] = savech;
            RETURN(INTNUM);
        }

        /* 0777 - octal number */
        "0" octdigit+ {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->intn = yasm_intnum_create_oct(TOK);
            s->tok[TOKLEN] = savech;
            RETURN(INTNUM);
        }

        /* 0xAA - hexidecimal number */
        '0x' hexdigit+ {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            /* skip 0 and x */
            lvalp->intn = yasm_intnum_create_hex(TOK+2);
            s->tok[TOKLEN] = savech;
            RETURN(INTNUM);
        }

        /* floating point value */
        [-+]? digit* "." digit+ ('e' [-+]? digit+)? {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->flt = yasm_floatnum_create(TOK);
            s->tok[TOKLEN] = savech;
            RETURN(FLTNUM);
        }
        [-+]? digit+ "." digit* ('e' [-+]? digit+)? {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->flt = yasm_floatnum_create(TOK);
            s->tok[TOKLEN] = savech;
            RETURN(FLTNUM);
        }
        "0" [DdEeFfTt] [-+]? digit* ("." digit*)? ('e' [-+]? digit+)? {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            lvalp->flt = yasm_floatnum_create(TOK+2);
            s->tok[TOKLEN] = savech;
            RETURN(FLTNUM);
        }

        /* character constant values */
        ['] {
            goto charconst;
        }

        /* string constant values */
        dquot {
            goto stringconst;
        }

        /* operators */
        "<<"                    { RETURN(LEFT_OP); }
        ">>"                    { RETURN(RIGHT_OP); }
        "<"                     { RETURN(LEFT_OP); }
        ">"                     { RETURN(RIGHT_OP); }
        [-+|^!*&/~$():@=,]      { RETURN(s->tok[0]); }
        ";"     {
            parser_gas->state = INITIAL;
            RETURN(s->tok[0]);
        }

        /* identifier */
        [a-zA-Z_.][a-zA-Z0-9_$.]* {
            lvalp->str.contents = yasm__xstrndup(TOK, TOKLEN);
            lvalp->str.len = TOKLEN;
            RETURN(ID);
        }

        /* identifier with @ */
        [a-zA-Z_.]([a-zA-Z0-9_$.]*[@][a-zA-Z0-9_$.]*)+ {
            /* if @ not part of ID, move the scanner cursor to the first @ */
            if (!((yasm_objfmt_base *)p_object->objfmt)->module->id_at_ok)
                cursor = (unsigned char *)strchr(TOK, '@');
            lvalp->str.contents = yasm__xstrndup(TOK, TOKLEN);
            lvalp->str.len = TOKLEN;
            RETURN(ID);
        }

        /* register or segment register */
        [%][a-zA-Z0-9]+ {
            savech = s->tok[TOKLEN];
            s->tok[TOKLEN] = '\0';
            if (parser_gas->is_nasm_preproc && strcmp(TOK+1, "line") == 0) {
                s->tok[TOKLEN] = savech;
                RETURN(NASM_LINE_MARKER);
            }

            switch (yasm_arch_parse_check_regtmod
                    (p_object->arch, TOK+1, TOKLEN-1, &lvalp->arch_data)) {
                case YASM_ARCH_REG:
                    s->tok[TOKLEN] = savech;
                    RETURN(REG);
                case YASM_ARCH_REGGROUP:
                    s->tok[TOKLEN] = savech;
                    RETURN(REGGROUP);
                case YASM_ARCH_SEGREG:
                    s->tok[TOKLEN] = savech;
                    RETURN(SEGREG);
                default:
                    break;
            }
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("Unrecognized register name `%s'"), s->tok);
            s->tok[TOKLEN] = savech;
            lvalp->arch_data = 0;
            RETURN(REG);
        }

        /* local label */
        [0-9] ':' {
            /* increment label index */
            parser_gas->local[s->tok[0]-'0']++;
            /* build local label name */
            lvalp->str.contents = yasm_xmalloc(30);
            lvalp->str.len =
                sprintf(lvalp->str.contents, "L%c\001%lu", s->tok[0],
                        parser_gas->local[s->tok[0]-'0']);
            RETURN(LABEL);
        }

        /* local label forward reference */
        [0-9] 'f' {
            /* build local label name */
            lvalp->str.contents = yasm_xmalloc(30);
            lvalp->str.len =
                sprintf(lvalp->str.contents, "L%c\001%lu", s->tok[0],
                        parser_gas->local[s->tok[0]-'0']+1);
            RETURN(ID);
        }

        /* local label backward reference */
        [0-9] 'b' {
            /* build local label name */
            lvalp->str.contents = yasm_xmalloc(30);
            lvalp->str.len =
                sprintf(lvalp->str.contents, "L%c\001%lu", s->tok[0],
                        parser_gas->local[s->tok[0]-'0']);
            RETURN(ID);
        }

        "/*"                    { parser_gas->state = COMMENT; goto comment; }
        "#"                     {
            if (parser_gas->is_cpp_preproc)
            {
                RETURN(CPP_LINE_MARKER);
            } else
                goto line_comment;
        }
        "//"                    { goto line_comment; }

        ws+                     { goto scan; }

        "\n"                    {
            if (parser_gas->save_input)
                cursor = save_line(parser_gas, cursor);
            parser_gas->state = INITIAL;
            RETURN(s->tok[0]);
        }

        any {
            yasm_warn_set(YASM_WARN_UNREC_CHAR,
                          N_("ignoring unrecognized character `%s'"),
                          yasm__conv_unprint(s->tok[0]));
            goto scan;
        }
    */

    /* C-style comment; nesting not supported */
comment:
    SCANINIT();

    /*!re2c
        /* End of comment */
        "*/"    { parser_gas->state = INITIAL; goto scan; }

        "\n"                    {
            if (parser_gas->save_input)
                cursor = save_line(parser_gas, cursor);
            RETURN(s->tok[0]);
        }

        any     {
            if (cursor == s->eof)
                return 0;
            goto comment;
        }
    */

    /* Single line comment. */
line_comment:
    /*!re2c
        (any \ [\n])*   { goto scan; }
    */

    /* .section directive (the section name portion thereof) */
section_directive:
    SCANINIT();

    /*!re2c
        [a-zA-Z0-9_$.-]+ {
            lvalp->str.contents = yasm__xstrndup(TOK, TOKLEN);
            lvalp->str.len = TOKLEN;
            parser_gas->state = INITIAL;
            RETURN(ID);
        }

        dquot                   { goto stringconst; }

        ws+                     { goto section_directive; }

        ","                     {
            parser_gas->state = INITIAL;
            RETURN(s->tok[0]);
        }

        "\n"                    {
            if (parser_gas->save_input)
                cursor = save_line(parser_gas, cursor);
            parser_gas->state = INITIAL;
            RETURN(s->tok[0]);
        }

        any {
            yasm_warn_set(YASM_WARN_UNREC_CHAR,
                          N_("ignoring unrecognized character `%s'"),
                          yasm__conv_unprint(s->tok[0]));
            goto section_directive;
        }
    */

    /* filename portion of nasm preproc %line */
nasm_filename:
    strbuf = yasm_xmalloc(STRBUF_ALLOC_SIZE);
    strbuf_size = STRBUF_ALLOC_SIZE;
    count = 0;

nasm_filename_scan:
    SCANINIT();

    /*!re2c
        "\n" {
            strbuf_append(count++, cursor, s, '\0');
            lvalp->str.contents = (char *)strbuf;
            lvalp->str.len = count;
            parser_gas->state = INITIAL;
            RETURN(STRING);
        }

        ws+ { goto nasm_filename_scan; }

        any {
            if (cursor == s->eof) {
                strbuf_append(count++, cursor, s, '\0');
                lvalp->str.contents = (char *)strbuf;
                lvalp->str.len = count;
                parser_gas->state = INITIAL;
                RETURN(STRING);
            }
            strbuf_append(count++, cursor, s, s->tok[0]);
            goto nasm_filename_scan;
        }
    */

    /* character constant values */
charconst:
    /*TODO*/

    /* string constant values */
stringconst:
    strbuf = yasm_xmalloc(STRBUF_ALLOC_SIZE);
    strbuf_size = STRBUF_ALLOC_SIZE;
    count = 0;

stringconst_scan:
    SCANINIT();

    /*!re2c
        /* Handle escaped character by copying both and continuing. */
        "\\".   {
            if (cursor == s->eof) {
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("unexpected end of file in string"));
                lvalp->str.contents = (char *)strbuf;
                lvalp->str.len = count;
                RETURN(STRING);
            }
            strbuf_append(count++, cursor, s, '\\');
            strbuf_append(count++, cursor, s, s->tok[1]);
            goto stringconst_scan;
        }

        dquot   {
            strbuf_append(count, cursor, s, '\0');
            yasm_unescape_cstring(strbuf, &count);
            lvalp->str.contents = (char *)strbuf;
            lvalp->str.len = count;
            RETURN(STRING);
        }

        any     {
            if (cursor == s->eof) {
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("unexpected end of file in string"));
                lvalp->str.contents = (char *)strbuf;
                lvalp->str.len = count;
                RETURN(STRING);
            }
            strbuf_append(count++, cursor, s, s->tok[0]);
            goto stringconst_scan;
        }
    */
}
