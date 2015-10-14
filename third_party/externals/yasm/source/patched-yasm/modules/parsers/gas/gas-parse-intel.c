/*
 * GAS-compatible parser Intel syntax support
 *
 *  Copyright (C) 2010  Alexei Svitkine
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
#include "modules/parsers/nasm/nasm-parser-struct.h"

extern yasm_bytecode *gas_intel_syntax_parse_instr(yasm_parser_nasm *parser_nasm, unsigned char *instr);

#define SET_FIELDS(to, from) \
    (to)->object = (from)->object; \
    (to)->locallabel_base = (from)->locallabel_base; \
    (to)->locallabel_base_len = (from)->locallabel_base_len; \
    (to)->preproc = (from)->preproc; \
    (to)->errwarns = (from)->errwarns; \
    (to)->linemap = (from)->linemap; \
    (to)->prev_bc = (from)->prev_bc;

yasm_bytecode *parse_instr_intel(yasm_parser_gas *parser_gas)
{
    char *stok, *slim;
    unsigned char *line;
    size_t length;

    yasm_parser_nasm parser_nasm;
    yasm_bytecode *bc;

    memset(&parser_nasm, 0, sizeof(parser_nasm));

    yasm_arch_set_var(parser_gas->object->arch, "gas_intel_mode", 1);
    SET_FIELDS(&parser_nasm, parser_gas);
    parser_nasm.masm = 1;

    stok = (char *) parser_gas->s.tok;
    slim = (char *) parser_gas->s.lim;
    length = 0;
    while (&stok[length] < slim && stok[length] != '\n') {
        length++;
    }

    if (&stok[length] == slim && parser_gas->line) {
        line = yasm_xmalloc(length + parser_gas->lineleft + 1);
        memcpy(line, parser_gas->s.tok, length);
        memcpy(line + length, parser_gas->linepos, parser_gas->lineleft);
        length += parser_gas->lineleft;
        if (line[length - 1] == '\n') length--;
    } else {
        line = yasm_xmalloc(length + 1);
        memcpy(line, parser_gas->s.tok, length);
    }
    line[length] = '\0';

    bc = gas_intel_syntax_parse_instr(&parser_nasm, line);

    SET_FIELDS(parser_gas, &parser_nasm);
    yasm_arch_set_var(parser_gas->object->arch, "gas_intel_mode", 0);

    yasm_xfree(line);

    return bc;
}
