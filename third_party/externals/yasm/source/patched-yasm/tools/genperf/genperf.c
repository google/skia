/*
 *
 * Generate Minimal Perfect Hash (genperf)
 *
 *  Copyright (C) 2006-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "tools/genperf/perfect.h"
#include "libyasm/compat-queue.h"
#include "libyasm/coretype.h"
#include "libyasm/errwarn.h"

typedef STAILQ_HEAD(slist, sval) slist;
typedef struct sval {
    STAILQ_ENTRY(sval) link;
    char *str;
} sval;

typedef STAILQ_HEAD(keyword_list, keyword) keyword_list;
typedef struct keyword {
    STAILQ_ENTRY(keyword) link;
    char *name;
    char *args;
    unsigned int line;
} keyword;

static unsigned int cur_line = 1;
static int errors = 0;

static void
report_error(const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "%u: ", cur_line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    errors++;
}

void
yasm__fatal(const char *message, ...)
{
    abort();
}

/* make the c output for the perfect hash tab array */
static void
make_c_tab(
    FILE *f,
    bstuff *tab,        /* table indexed by b */
    ub4 smax,           /* range of scramble[] */
    ub4 blen,           /* b in 0..blen-1, power of 2 */
    ub4 *scramble)      /* used in final hash */
{
    ub4   i;
    /* table for the mapping for the perfect hash */
    if (blen >= USE_SCRAMBLE) {
        /* A way to make the 1-byte values in tab bigger */
        if (smax > UB2MAXVAL+1) {
            fprintf(f, "  static const unsigned long scramble[] = {\n");
            for (i=0; i<=UB1MAXVAL; i+=4)
                fprintf(f, "    0x%.8lx, 0x%.8lx, 0x%.8lx, 0x%.8lx,\n",
                    scramble[i+0], scramble[i+1], scramble[i+2], scramble[i+3]);
        } else {
            fprintf(f, "  static const unsigned short scramble[] = {\n");
            for (i=0; i<=UB1MAXVAL; i+=8)
                fprintf(f, 
"    0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx,\n",
                    scramble[i+0], scramble[i+1], scramble[i+2], scramble[i+3],
                    scramble[i+4], scramble[i+5], scramble[i+6], scramble[i+7]);
        }
        fprintf(f, "  };\n");
        fprintf(f, "\n");
    }

    if (blen > 0) {
        /* small adjustments to _a_ to make values distinct */
        if (smax <= UB1MAXVAL+1 || blen >= USE_SCRAMBLE)
            fprintf(f, "  static const unsigned char ");
        else
            fprintf(f, "  static const unsigned short ");
        fprintf(f, "tab[] = {\n");

        if (blen < 16) {
            for (i=0; i<blen; ++i)
                fprintf(f, "%3ld,", scramble[tab[i].val_b]);
        } else if (blen <= 1024) {
            for (i=0; i<blen; i+=16)
                fprintf(f, "    %ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
                    scramble[tab[i+0].val_b], scramble[tab[i+1].val_b], 
                    scramble[tab[i+2].val_b], scramble[tab[i+3].val_b], 
                    scramble[tab[i+4].val_b], scramble[tab[i+5].val_b], 
                    scramble[tab[i+6].val_b], scramble[tab[i+7].val_b], 
                    scramble[tab[i+8].val_b], scramble[tab[i+9].val_b], 
                    scramble[tab[i+10].val_b], scramble[tab[i+11].val_b], 
                    scramble[tab[i+12].val_b], scramble[tab[i+13].val_b], 
                    scramble[tab[i+14].val_b], scramble[tab[i+15].val_b]); 
        } else if (blen < USE_SCRAMBLE) {
            for (i=0; i<blen; i+=8)
                fprintf(f, "    %ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
                    scramble[tab[i+0].val_b], scramble[tab[i+1].val_b], 
                    scramble[tab[i+2].val_b], scramble[tab[i+3].val_b], 
                    scramble[tab[i+4].val_b], scramble[tab[i+5].val_b], 
                    scramble[tab[i+6].val_b], scramble[tab[i+7].val_b]); 
        } else {
            for (i=0; i<blen; i+=16)
                fprintf(f, "    %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
                    tab[i+0].val_b, tab[i+1].val_b, 
                    tab[i+2].val_b, tab[i+3].val_b, 
                    tab[i+4].val_b, tab[i+5].val_b, 
                    tab[i+6].val_b, tab[i+7].val_b, 
                    tab[i+8].val_b, tab[i+9].val_b, 
                    tab[i+10].val_b, tab[i+11].val_b, 
                    tab[i+12].val_b, tab[i+13].val_b, 
                    tab[i+14].val_b, tab[i+15].val_b); 
        }
        fprintf(f, "  };\n");
        fprintf(f, "\n");
    }
}

static void
perfect_gen(FILE *out, const char *lookup_function_name,
            const char *struct_name, keyword_list *kws,
            const char *filename)
{
    ub4 nkeys;
    key *keys;
    hashform form;
    bstuff *tab;                /* table indexed by b */
    hstuff *tabh;               /* table indexed by hash value */
    ub4 smax;           /* scramble[] values in 0..smax-1, a power of 2 */
    ub4 alen;                   /* a in 0..alen-1, a power of 2 */
    ub4 blen;                   /* b in 0..blen-1, a power of 2 */
    ub4 salt;                   /* a parameter to the hash function */
    gencode final;              /* code for final hash */
    ub4 i;
    ub4 scramble[SCRAMBLE_LEN]; /* used in final hash function */
    char buf[10][80];           /* buffer for generated code */
    char *buf2[10];             /* also for generated code */
    keyword *kw;

    /* perfect hash configuration */
    form.mode = NORMAL_HM;
    form.hashtype = STRING_HT;
    form.perfect = MINIMAL_HP;
    form.speed = SLOW_HS;

    /* set up code for final hash */
    final.line = buf2;
    final.used = 0;
    final.len  = 10;
    for (i=0; i<10; i++)
        final.line[i] = buf[i];

    /* build list of keys */
    nkeys = 0;
    keys = NULL;
    STAILQ_FOREACH(kw, kws, link) {
        key *k = yasm_xmalloc(sizeof(key));

        k->name_k = yasm__xstrdup(kw->name);
        k->len_k = (ub4)strlen(kw->name);
        k->next_k = keys;
        keys = k;
        nkeys++;
    }

    /* find the hash */
    findhash(&tab, &tabh, &alen, &blen, &salt, &final, 
             scramble, &smax, keys, nkeys, &form);

    /* The hash function beginning */
    fprintf(out, "static const struct %s *\n", struct_name);
    fprintf(out, "%s(const char *key, size_t len)\n", lookup_function_name);
    fprintf(out, "{\n");

    /* output the dir table: this should loop up to smax for NORMAL_HP,
     * or up to pakd.nkeys for MINIMAL_HP.
     */
    fprintf(out, "  static const struct %s pd[%lu] = {\n", struct_name, nkeys);
    for (i=0; i<nkeys; i++) {
        if (tabh[i].key_h) {
            STAILQ_FOREACH(kw, kws, link) {
                if (strcmp(kw->name, tabh[i].key_h->name_k) == 0)
                    break;
            }
            if (!kw) {
                report_error("internal error: could not find `%s'",
                             tabh[i].key_h->name_k);
                break;
            }
            fprintf(out, "#line %u \"%s\"\n", kw->line, filename);
            fprintf(out, "    {\"%s\"%s}", kw->name, kw->args);
        } else
            fprintf(out, "    { NULL }");

        if (i < nkeys-1)
            fprintf(out, ",");
        fprintf(out, "\n");
    }
    fprintf(out, "  };\n");

    /* output the hash tab[] array */
    make_c_tab(out, tab, smax, blen, scramble);

    /* The hash function body */
    fprintf(out, "  const struct %s *ret;\n", struct_name);
    for (i=0; i<final.used; ++i)
        fprintf(out, "%s", final.line[i]);
    fprintf(out, "  if (rsl >= %lu) return NULL;\n", nkeys);
    fprintf(out, "  ret = &pd[rsl];\n");
    fprintf(out, "  if (strcmp(key, ret->name) != 0) return NULL;\n");
    fprintf(out, "  return ret;\n");
    fprintf(out, "}\n");
    fprintf(out, "\n");

    free(tab);
    free(tabh);
}

int
main(int argc, char *argv[])
{
    FILE *in, *out;
    size_t i;
    char *ch;
    static char line[1024], tmp[1024];
    static char struct_name[128] = "";
    static char lookup_function_name[128] = "in_word_set";
    static char language[16] = "";
    static char delimiters[16] = ",\r\n";
    static char name[128];
    static char filename[768];
    int need_struct = 0;
    int have_struct = 0;
    int go_keywords = 0;
    int ignore_case = 0;
    int compare_strncmp = 0;
    int readonly_tables = 0;
    slist usercode, usercode2;
    keyword_list keywords;
    sval *sv;
    keyword *kw;

    if (argc != 3) {
        fprintf(stderr, "Usage: genperf <in> <out>\n");
        return EXIT_FAILURE;
    }

    in = fopen(argv[1], "rt");
    if (!in) {
        fprintf(stderr, "Could not open `%s' for reading\n", argv[1]);
        return EXIT_FAILURE;
    }

    ch = argv[1];
    i = 0;
    while (*ch && i < 767) {
        if (*ch == '\\') {
            filename[i++] = '/';
            ch++;
        } else
            filename[i++] = *ch++;
    }
    filename[i] = '\0';

    STAILQ_INIT(&usercode);
    STAILQ_INIT(&usercode2);
    STAILQ_INIT(&keywords);

    /* Parse declarations section */
    while (fgets(line, 1024, in)) {
        /* Comments start with # as the first thing on a line */
        if (line[0] == '#') {
            cur_line++;
            continue;
        }

        /* Handle structure declaration */
        if (strncmp(line, "struct", 6) == 0) {
            int braces;

            if (!need_struct) {
                report_error("struct without %%struct-type declaration");
                return EXIT_FAILURE;
            }
            if (have_struct) {
                report_error("more than one struct declaration");
                return EXIT_FAILURE;
            }
            have_struct = 1;

            /* copy struct name */
            ch = &line[6];
            while (isspace(*ch))
                ch++;
            i = 0;
            while ((isalnum(*ch) || *ch == '_') && i < 127)
                struct_name[i++] = *ch++;
            if (i == 127) {
                report_error("struct name too long");
                return EXIT_FAILURE;
            }
            struct_name[i] = '\0';

            sv = yasm_xmalloc(sizeof(sval));
            sprintf(tmp, "#line %u \"%s\"\n", cur_line, filename);
            sv->str = yasm__xstrdup(tmp);
            STAILQ_INSERT_TAIL(&usercode, sv, link);

            braces = 0;
            do {
                /* count braces to determine when we're done */
                ch = line;
                while (*ch != '\0') {
                    if (*ch == '{')
                        braces++;
                    if (*ch == '}')
                        braces--;
                    ch++;
                }
                sv = yasm_xmalloc(sizeof(sval));
                sv->str = yasm__xstrdup(line);
                STAILQ_INSERT_TAIL(&usercode, sv, link);
                cur_line++;
                if (braces <= 0)
                    break;
            } while (fgets(line, 1024, in));
            cur_line++;
            continue;
        }

        /* Ignore non-declaration lines */
        if (line[0] != '%') {
            cur_line++;
            continue;
        }

        /* %% terminates declarations section */
        if (line[1] == '%') {
            if (need_struct && !have_struct) {
                report_error("%%struct-type declaration, but no struct found");
                return EXIT_FAILURE;
            }
            go_keywords = 1;
            break;      /* move on to keywords section */
        }

        /* %{ begins a verbatim code section that ends with %} */
        if (line[1] == '{') {
            sv = yasm_xmalloc(sizeof(sval));
            sprintf(tmp, "#line %u \"%s\"\n\n", cur_line, filename);
            sv->str = yasm__xstrdup(tmp);
            STAILQ_INSERT_TAIL(&usercode, sv, link);

            while (fgets(line, 1024, in)) {
                cur_line++;
                if (line[0] == '%' && line[1] == '}')
                    break;
                sv = yasm_xmalloc(sizeof(sval));
                sv->str = yasm__xstrdup(line);
                STAILQ_INSERT_TAIL(&usercode, sv, link);
            }
            cur_line++;
            continue;
        }

        if (strncmp(&line[1], "ignore-case", 11) == 0) {
            ignore_case = 1;
        } else if (strncmp(&line[1], "compare-strncmp", 15) == 0) {
            compare_strncmp = 1;
        } else if (strncmp(&line[1], "readonly-tables", 15) == 0) {
            readonly_tables = 1;
        } else if (strncmp(&line[1], "language=", 9) == 0) {
            ch = &line[10];
            i = 0;
            while (*ch != '\n' && i<15)
                language[i++] = *ch++;
            language[i] = '\0';
        } else if (strncmp(&line[1], "delimiters=", 11) == 0) {
            ch = &line[12];
            i = 0;
            while (i<15)
                delimiters[i++] = *ch++;
            delimiters[i] = '\0';
        } else if (strncmp(&line[1], "enum", 4) == 0) {
            /* unused */
        } else if (strncmp(&line[1], "struct-type", 11) == 0) {
            need_struct = 1;
        } else if (strncmp(&line[1], "define", 6) == 0) {
            /* Several different defines we need to handle */
            ch = &line[7];
            while (isspace(*ch))
                ch++;

            if (strncmp(ch, "hash-function-name", 18) == 0) {
                /* unused */
            } else if (strncmp(ch, "lookup-function-name", 20) == 0) {
                ch = &line[7+20+1];
                while (isspace(*ch))
                    ch++;
                i = 0;
                while ((isalnum(*ch) || *ch == '_') && i < 127)
                    lookup_function_name[i++] = *ch++;
                if (i == 127) {
                    report_error("struct name too long");
                    return EXIT_FAILURE;
                }
                lookup_function_name[i] = '\0';
            } else {
                fprintf(stderr, "%u: unrecognized define `%s'\n", cur_line,
                        line);
            }
        } else {
            fprintf(stderr, "%u: unrecognized declaration `%s'\n", cur_line,
                    line);
        }

        cur_line++;
    }

    if (!go_keywords) {
        report_error("no keywords section found");
        return EXIT_FAILURE;
    }

    /* Parse keywords section */
    while (fgets(line, 1024, in)) {
        char *d;

        /* Comments start with # as the first thing on a line */
        if (line[0] == '#') {
            cur_line++;
            continue;
        }

        /* Keywords section terminated with %% */
        if (line[0] == '%' && line[1] == '%')
            break;

        /* Look for name */
        ch = &line[0];
        i = 0;
        while (strchr(delimiters, *ch) == NULL && i < 127)
            name[i++] = *ch++;
        if (i == 127) {
            report_error("keyword name too long");
            return EXIT_FAILURE;
        }
        name[i] = '\0';

        /* Strip EOL */
        d = strrchr(ch, '\n');
        if (d)
            *d = '\0';
        d = strrchr(ch, '\r');
        if (d)
            *d = '\0';
        kw = yasm_xmalloc(sizeof(keyword));
        kw->name = yasm__xstrdup(name);
        kw->args = yasm__xstrdup(ch);
        kw->line = cur_line;
        STAILQ_INSERT_TAIL(&keywords, kw, link);
        cur_line++;
    }

    if (errors > 0)
        return EXIT_FAILURE;

    /* Pull in any end code */
    if (!feof(in)) {
        sv = yasm_xmalloc(sizeof(sval));
        sprintf(tmp, "#line %u \"%s\"\n\n", cur_line, filename);
        sv->str = yasm__xstrdup(tmp);
        STAILQ_INSERT_TAIL(&usercode2, sv, link);

        while (fgets(line, 1024, in)) {
            sv = yasm_xmalloc(sizeof(sval));
            sv->str = yasm__xstrdup(line);
            STAILQ_INSERT_TAIL(&usercode2, sv, link);
        }
    }

    /* output code */
    out = fopen(argv[2], "wt");
    if (!out) {
        fprintf(stderr, "Could not open `%s' for writing\n", argv[2]);
        return EXIT_FAILURE;
    }

    fprintf(out, "/* %s code produced by genperf */\n", language);
    fprintf(out, "/* Command-line: genperf %s %s */\n", argv[1], argv[2]);

    STAILQ_FOREACH(sv, &usercode, link)
        fprintf(out, "%s", sv->str);

    /* Get perfect hash */
    perfect_gen(out, lookup_function_name, struct_name, &keywords, filename);

    STAILQ_FOREACH(sv, &usercode2, link)
        fprintf(out, "%s", sv->str);

    fclose(out);

    if (errors > 0) {
        remove(argv[2]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

