/*
 *
 * Generate array-of-const-string from text file.
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
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1024

int
main(int argc, char *argv[])
{
    FILE *in, *out;
    int i;
    char *str;
    char *strp;
    size_t len;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <string> <outfile> <file> [<file> ...]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    out = fopen(argv[2], "wt");

    if (!out) {
        fprintf(stderr, "Could not open `%s'.\n", argv[2]);
        return EXIT_FAILURE;
    }

    str = malloc(MAXLINE);

    fprintf(out, "/* This file auto-generated from %s by genstring.c"
                 " - don't edit it */\n\n"
                 "static const char *%s[] = {\n", argv[3], argv[1]);

    for (i=3; i<argc; i++) {
        in = fopen(argv[i], "rt");
        if (!in) {
            fprintf(stderr, "Could not open `%s'.\n", argv[i]);
            fclose(out);
            remove(argv[2]);
            return EXIT_FAILURE;
        }

        while (fgets(str, MAXLINE, in)) {
            strp = str;

            /* strip off trailing whitespace */
            len = strlen(strp);
            while (len > 0 && (strp[len-1] == ' ' || strp[len-1] == '\t' ||
                               strp[len-1] == '\n')) {
                strp[len-1] = '\0';
                len--;
            }

            /* output as string to output file */
            fprintf(out, "    \"");
            while (*strp != '\0') {
                if (*strp == '\\' || *strp == '"')
                    fputc('\\', out);
                fputc(*strp, out);
                strp++;
            }
            fprintf(out, "\",\n");
        }

        fclose(in);
    }

    fprintf(out, "};\n");
    fclose(out);

    free(str);

    return EXIT_SUCCESS;
}
