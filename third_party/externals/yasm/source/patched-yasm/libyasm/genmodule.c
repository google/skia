/*
 *
 * Generate module.c from module.in and Makefile.am or Makefile.
 *
 *  Copyright (C) 2004-2007  Peter Johnson
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "compat-queue.h"

#define MAXNAME 128
#define MAXLINE 1024
#define MAXMODULES 128
#define MAXINCLUDES 256

typedef struct include {
    STAILQ_ENTRY(include) link;
    char *filename;
} include;

int
main(int argc, char *argv[])
{
    FILE *in, *out;
    char *str;
    int i;
    size_t len;
    char *strp;
    char *modules[MAXMODULES];
    int num_modules = 0;
    STAILQ_HEAD(includehead, include) includes =
        STAILQ_HEAD_INITIALIZER(includes);
    include *inc;
    int isam = 0;
    int linecont = 0;
    char *outfile;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <module.in> <Makefile[.am]> <outfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    outfile = argv[3];
    str = malloc(MAXLINE);

    /* Starting with initial input Makefile, look for include <file> or
     * YASM_MODULES += <module>.  Note this currently doesn't handle
     * a relative starting path.
     */
    len = strlen(argv[2]);
    inc = malloc(sizeof(include));
    inc->filename = malloc(len+1);
    strcpy(inc->filename, argv[2]);
    STAILQ_INSERT_TAIL(&includes, inc, link);

    isam = argv[2][len-2] == 'a' && argv[2][len-1] == 'm';

    while (!STAILQ_EMPTY(&includes)) {
        inc = STAILQ_FIRST(&includes);
        STAILQ_REMOVE_HEAD(&includes, link);
        in = fopen(inc->filename, "rt");
        if (!in) {
            fprintf(stderr, "Could not open `%s'.\n", inc->filename);
            return EXIT_FAILURE;
        }
        free(inc->filename);
        free(inc);

        while (fgets(str, MAXLINE, in)) {
            /* Strip off any trailing whitespace */
            len = strlen(str);
            if (len > 0) {
                strp = &str[len-1];
                while (len > 0 && isspace(*strp)) {
                    *strp-- = '\0';
                    len--;
                }
            }

            strp = str;

            /* Skip whitespace */
            while (isspace(*strp))
                strp++;

            /* Skip comments */
            if (*strp == '#')
                continue;

            /* If line continuation, skip to continue copy */
            if (linecont)
                goto keepgoing;

            /* Check for include if original input is .am file */
            if (isam && strncmp(strp, "include", 7) == 0 && isspace(strp[7])) {
                strp += 7;
                while (isspace(*strp))
                    strp++;
                /* Build new include and add to end of list */
                inc = malloc(sizeof(include));
                inc->filename = malloc(strlen(strp)+1);
                strcpy(inc->filename, strp);
                STAILQ_INSERT_TAIL(&includes, inc, link);
                continue;
            }

            /* Check for YASM_MODULES = or += */
            if (strncmp(strp, "YASM_MODULES", 12) != 0)
                continue;
            strp += 12;
            while (isspace(*strp))
                strp++;
            if (strncmp(strp, "+=", 2) != 0 && *strp != '=')
                continue;
            if (*strp == '+')
                strp++;
            strp++;
            while (isspace(*strp))
                strp++;

keepgoing:
            /* Check for continuation */
            if (len > 0 && str[len-1] == '\\') {
                str[len-1] = '\0';
                while (isspace(*strp))
                    *strp-- = '\0';
                linecont = 1;
            } else
                linecont = 0;

            while (*strp != '\0') {
                /* Copy module name */
                modules[num_modules] = malloc(MAXNAME);
                len = 0;
                while (*strp != '\0' && !isspace(*strp))
                    modules[num_modules][len++] = *strp++;
                modules[num_modules][len] = '\0';
                num_modules++;

                while (isspace(*strp))
                    strp++;
            }
        }
        fclose(in);
    }

    out = fopen(outfile, "wt");

    if (!out) {
        fprintf(stderr, "Could not open `%s'.\n", outfile);
        return EXIT_FAILURE;
    }

    fprintf(out, "/* This file auto-generated by genmodule.c"
                 " - don't edit it */\n\n");

    in = fopen(argv[1], "rt");
    if (!in) {
        fprintf(stderr, "Could not open `%s'.\n", argv[1]);
        fclose(out);
        remove(outfile);
        return EXIT_FAILURE;
    }

    len = 0;
    while (fgets(str, MAXLINE, in)) {
        if (strncmp(str, "MODULES_", 8) == 0) {
            len = 0;
            strp = str+8;
            while (*strp != '\0' && *strp != '_') {
                len++;
                strp++;
            }
            *strp = '\0';

            for (i=0; i<num_modules; i++) {
                if (strncmp(modules[i], str+8, len) == 0) {
                    fprintf(out, "    {\"%s\", &yasm_%s_LTX_%s},\n",
                            modules[i]+len+1, modules[i]+len+1, str+8);
                }
            }
        } else if (strncmp(str, "EXTERN_LIST", 11) == 0) {
            for (i=0; i<num_modules; i++) {
                strcpy(str, modules[i]);
                strp = str;
                while (*strp != '\0' && *strp != '_')
                    strp++;
                *strp++ = '\0';

                fprintf(out, "extern yasm_%s_module yasm_%s_LTX_%s;\n",
                        str, strp, str);
            }
        } else
            fputs(str, out);
    }

    fclose(in);
    fclose(out);

    for (i=0; i<num_modules; i++)
        free(modules[i]);
    free(str);

    return EXIT_SUCCESS;
}
