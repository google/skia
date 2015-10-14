/*
 * NASM-compatible parser struct header file
 *
 *  Copyright (C) 2002-2007  Peter Johnson
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
#ifndef YASM_NASM_PARSER_STRUCT_H
#define YASM_NASM_PARSER_STRUCT_H

typedef union {
    unsigned int int_info;
    char *str_val;
    yasm_intnum *intn;
    yasm_floatnum *flt;
    yasm_bytecode *bc;
    uintptr_t arch_data;
    struct {
        char *contents;
        size_t len;
    } str;
} nasm_yystype;

typedef struct yasm_parser_nasm {
    int tasm;
    int masm;

    /*@only@*/ yasm_object *object;

    /* last "base" label for local (.) labels */
    /*@null@*/ char *locallabel_base;
    size_t locallabel_base_len;

    /*@dependent@*/ yasm_preproc *preproc;
    /*@dependent@*/ yasm_errwarns *errwarns;

    /*@dependent@*/ yasm_linemap *linemap;

    /*@null@*/ yasm_bytecode *prev_bc;

    int save_input;

    yasm_scanner s;
    int state;

    int token;          /* enum tokentype or any character */
    nasm_yystype tokval;
    char tokch;         /* first character of token */

    /* one token of lookahead; used sparingly */
    int peek_token;     /* NONE if none */
    nasm_yystype peek_tokval;
    char peek_tokch;

    /* Starting point of the absolute section.  NULL if not in an absolute
     * section.
     */
    /*@null@*/ yasm_expr *absstart;

    /* Current location inside an absolute section (including the start).
     * NULL if not in an absolute section.
     */
    /*@null@*/ yasm_expr *abspos;
} yasm_parser_nasm;

#endif
