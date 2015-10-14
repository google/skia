/*
 * NASM-compatible parser header file
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
#ifndef YASM_NASM_PARSER_H
#define YASM_NASM_PARSER_H

#include "nasm-parser-struct.h"

#define YYCTYPE         unsigned char

#define MAX_SAVED_LINE_LEN  80

enum tokentype {
    INTNUM = 258,
    FLTNUM,
    DIRECTIVE_NAME,
    FILENAME,
    STRING,
    SIZE_OVERRIDE,
    OFFSET,
    DECLARE_DATA,
    RESERVE_SPACE,
    LABEL,
    INCBIN,
    EQU,
    TIMES,
    DUP,
    SEG,
    WRT,
    ABS,
    REL,
    NOSPLIT,
    STRICT,
    INSN,
    PREFIX,
    REG,
    REGGROUP,
    SEGREG,
    TARGETMOD,
    LEFT_OP,
    RIGHT_OP,
    LOW,
    HIGH,
    SIGNDIV,
    SIGNMOD,
    START_SECTION_ID,
    ID,
    LOCAL_ID,
    SPECIAL_ID,
    NONLOCAL_ID,
    LINE,
    NONE                /* special token for lookahead */
};

enum nasm_parser_state {
    INITIAL,
    DIRECTIVE,
    SECTION_DIRECTIVE,
    DIRECTIVE2,
    LINECHG,
    LINECHG2,
    INSTRUCTION
};

#define YYSTYPE nasm_yystype

/* shorter access names to commonly used parser_nasm fields */
#define p_object        (parser_nasm->object)
#define p_symtab        (parser_nasm->object->symtab)
#define cursect         (parser_nasm->object->cur_section)
#define curtok          (parser_nasm->token)
#define curval          (parser_nasm->tokval)

#define INTNUM_val              (curval.intn)
#define FLTNUM_val              (curval.flt)
#define DIRECTIVE_NAME_val      (curval.str_val)
#define FILENAME_val            (curval.str_val)
#define STRING_val              (curval.str)
#define SIZE_OVERRIDE_val       (curval.int_info)
#define DECLARE_DATA_val        (curval.int_info)
#define RESERVE_SPACE_val       (curval.int_info)
#define INSN_val                (curval.bc)
#define PREFIX_val              (curval.arch_data)
#define REG_val                 (curval.arch_data)
#define REGGROUP_val            (curval.arch_data)
#define SEGREG_val              (curval.arch_data)
#define TARGETMOD_val           (curval.arch_data)
#define ID_val                  (curval.str_val)

#define cur_line        (yasm_linemap_get_current(parser_nasm->linemap))

#define p_expr_new_tree(l,o,r)  yasm_expr_create_tree(l,o,r,cur_line)
#define p_expr_new_branch(o,r)  yasm_expr_create_branch(o,r,cur_line)
#define p_expr_new_ident(r)     yasm_expr_create_ident(r,cur_line)

void nasm_parser_parse(yasm_parser_nasm *parser_nasm);
void nasm_parser_cleanup(yasm_parser_nasm *parser_nasm);
int nasm_parser_lex(YYSTYPE *lvalp, yasm_parser_nasm *parser_nasm);

#endif
