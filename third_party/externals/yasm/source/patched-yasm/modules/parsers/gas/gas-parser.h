/*
 * GAS-compatible parser header file
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
#ifndef YASM_GAS_PARSER_H
#define YASM_GAS_PARSER_H

#define YYCTYPE         unsigned char

#define MAX_SAVED_LINE_LEN  80

enum tokentype {
    INTNUM = 258,
    FLTNUM,
    STRING,
    REG,
    REGGROUP,
    SEGREG,
    TARGETMOD,
    LEFT_OP,
    RIGHT_OP,
    ID,
    LABEL,
    CPP_LINE_MARKER,
    NASM_LINE_MARKER,
    NONE
};

typedef union {
    unsigned int int_info;
    yasm_intnum *intn;
    yasm_floatnum *flt;
    yasm_bytecode *bc;
    uintptr_t arch_data;
    struct {
        char *contents;
        size_t len;
    } str;
} yystype;
#define YYSTYPE yystype

enum gas_parser_state {
    INITIAL,
    COMMENT,
    SECTION_DIRECTIVE,
    NASM_FILENAME
};

typedef struct yasm_parser_gas {
    /*@only@*/ yasm_object *object;

    /* last "base" label for local (.) labels */
    /*@null@*/ char *locallabel_base;
    size_t locallabel_base_len;

    /* .line/.file: we have to see both to start setting linemap versions */
    int dir_fileline;
    /*@null@*/ char *dir_file;
    unsigned long dir_line;

    /* Have we seen a line marker? */
    int seen_line_marker;

    /*@dependent@*/ yasm_preproc *preproc;
    /*@dependent@*/ yasm_errwarns *errwarns;

    /*@dependent@*/ yasm_linemap *linemap;

    /*@null@*/ yasm_bytecode *prev_bc;
    yasm_bytecode *temp_bc;

    int save_input;
    YYCTYPE save_line[2][MAX_SAVED_LINE_LEN];
    int save_last;

    /* Line data storage used in preproc_input(). */
    char *line, *linepos;
    size_t lineleft;

    yasm_scanner s;
    enum gas_parser_state state;

    int token;          /* enum tokentype or any character */
    yystype tokval;
    char tokch;         /* first character of token */

    /* one token of lookahead; used sparingly */
    int peek_token;     /* NONE if none */
    yystype peek_tokval;
    char peek_tokch;

    /* Index of local labels; what's stored here is the /next/ index,
     * so these are all 0 at start.
     */
    unsigned long local[10];

    /* Parser-handled directives HAMT lookup */
    HAMT *dirs;

    int intel_syntax;

    int is_nasm_preproc;
    int is_cpp_preproc;
} yasm_parser_gas;

/* shorter access names to commonly used parser_gas fields */
#define p_object        (parser_gas->object)
#define p_symtab        (parser_gas->object->symtab)
#define cursect         (parser_gas->object->cur_section)
#define curtok          (parser_gas->token)
#define curval          (parser_gas->tokval)

#define INTNUM_val              (curval.intn)
#define FLTNUM_val              (curval.flt)
#define STRING_val              (curval.str)
#define REG_val                 (curval.arch_data)
#define REGGROUP_val            (curval.arch_data)
#define SEGREG_val              (curval.arch_data)
#define TARGETMOD_val           (curval.arch_data)
#define ID_val                  (curval.str.contents)
#define ID_len                  (curval.str.len)
#define LABEL_val               (curval.str.contents)
#define LABEL_len               (curval.str.len)

#define cur_line        (yasm_linemap_get_current(parser_gas->linemap))

#define p_expr_new(l,o,r)       yasm_expr_create(o,l,r,cur_line)
#define p_expr_new_tree(l,o,r)  yasm_expr_create_tree(l,o,r,cur_line)
#define p_expr_new_branch(o,r)  yasm_expr_create_branch(o,r,cur_line)
#define p_expr_new_ident(r)     yasm_expr_create_ident(r,cur_line)

yasm_bytecode *parse_instr_intel(yasm_parser_gas *parser_gas);

void gas_parser_parse(yasm_parser_gas *parser_gas);
void gas_parser_cleanup(yasm_parser_gas *parser_gas);
int gas_parser_lex(YYSTYPE *lvalp, yasm_parser_gas *parser_gas);

#endif
