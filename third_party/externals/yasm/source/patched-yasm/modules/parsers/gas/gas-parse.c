/*
 * GAS-compatible parser
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

#include <ctype.h>
#include <limits.h>
#include <math.h>

#include "modules/parsers/gas/gas-parser.h"

typedef struct dir_lookup {
    const char *name;
    yasm_bytecode * (*handler) (yasm_parser_gas *, unsigned int);
    unsigned int param;
    enum gas_parser_state newstate;
} dir_lookup;

static void cpp_line_marker(yasm_parser_gas *parser_gas);
static void nasm_line_marker(yasm_parser_gas *parser_gas);
static yasm_bytecode *parse_instr(yasm_parser_gas *parser_gas);
static int parse_dirvals(yasm_parser_gas *parser_gas, yasm_valparamhead *vps);
static int parse_datavals(yasm_parser_gas *parser_gas, yasm_datavalhead *dvs);
static int parse_strvals(yasm_parser_gas *parser_gas, yasm_datavalhead *dvs);
static yasm_effaddr *parse_memaddr(yasm_parser_gas *parser_gas);
static yasm_insn_operand *parse_operand(yasm_parser_gas *parser_gas);
static yasm_expr *parse_expr(yasm_parser_gas *parser_gas);
static yasm_expr *parse_expr0(yasm_parser_gas *parser_gas);
static yasm_expr *parse_expr1(yasm_parser_gas *parser_gas);
static yasm_expr *parse_expr2(yasm_parser_gas *parser_gas);

static void define_label(yasm_parser_gas *parser_gas, char *name, int local);
static void define_lcomm(yasm_parser_gas *parser_gas, /*@only@*/ char *name,
                         yasm_expr *size, /*@null@*/ yasm_expr *align);
static yasm_section *gas_get_section
    (yasm_parser_gas *parser_gas, /*@only@*/ char *name, /*@null@*/ char *flags,
     /*@null@*/ char *type, /*@null@*/ yasm_valparamhead *objext_valparams,
     int builtin);
static void gas_switch_section
    (yasm_parser_gas *parser_gas, /*@only@*/ const char *name,
     /*@null@*/ char *flags, /*@null@*/ char *type,
     /*@null@*/ yasm_valparamhead *objext_valparams, int builtin);
static yasm_bytecode *gas_parser_align
    (yasm_parser_gas *parser_gas, yasm_section *sect, yasm_expr *boundval,
     /*@null@*/ yasm_expr *fillval, /*@null@*/ yasm_expr *maxskipval,
     int power2);
static yasm_bytecode *gas_parser_dir_fill
    (yasm_parser_gas *parser_gas, /*@only@*/ yasm_expr *repeat,
     /*@only@*/ /*@null@*/ yasm_expr *size,
     /*@only@*/ /*@null@*/ yasm_expr *value);

#define is_eol_tok(tok) ((tok) == '\n' || (tok) == ';' || (tok) == 0)
#define is_eol()        is_eol_tok(curtok)

#define get_next_token()    (curtok = gas_parser_lex(&curval, parser_gas))

static void
get_peek_token(yasm_parser_gas *parser_gas)
{
    char savech = parser_gas->tokch;
    if (parser_gas->peek_token != NONE)
        yasm_internal_error(N_("can only have one token of lookahead"));
    parser_gas->peek_token =
        gas_parser_lex(&parser_gas->peek_tokval, parser_gas);
    parser_gas->peek_tokch = parser_gas->tokch;
    parser_gas->tokch = savech;
}

static void
destroy_curtok_(yasm_parser_gas *parser_gas)
{
    if (curtok < 256)
        ;
    else switch ((enum tokentype)curtok) {
        case INTNUM:
            yasm_intnum_destroy(curval.intn);
            break;
        case FLTNUM:
            yasm_floatnum_destroy(curval.flt);
            break;
        case ID:
        case LABEL:
        case STRING:
            yasm_xfree(curval.str.contents);
            break;
        default:
            break;
    }
    curtok = NONE;          /* sanity */
}
#define destroy_curtok()    destroy_curtok_(parser_gas)

/* Eat all remaining tokens to EOL, discarding all of them.  If there's any
 * intervening tokens, generates an error (junk at end of line).
 */
static void
demand_eol_(yasm_parser_gas *parser_gas)
{
    if (is_eol())
        return;

    yasm_error_set(YASM_ERROR_SYNTAX,
        N_("junk at end of line, first unrecognized character is `%c'"),
        parser_gas->tokch);

    do {
        destroy_curtok();
        get_next_token();
    } while (!is_eol());
}
#define demand_eol() demand_eol_(parser_gas)

static int
expect_(yasm_parser_gas *parser_gas, int token)
{
    static char strch[] = "` '";
    const char *str;

    if (curtok == token)
        return 1;

    switch (token) {
        case INTNUM:            str = "integer"; break;
        case FLTNUM:            str = "floating point value"; break;
        case STRING:            str = "string"; break;
        case REG:               str = "register"; break;
        case REGGROUP:          str = "register group"; break;
        case SEGREG:            str = "segment register"; break;
        case TARGETMOD:         str = "target modifier"; break;
        case LEFT_OP:           str = "<<"; break;
        case RIGHT_OP:          str = ">>"; break;
        case ID:                str = "identifier"; break;
        case LABEL:             str = "label"; break;
        default:
            strch[1] = token;
            str = strch;
            break;
    }
    yasm_error_set(YASM_ERROR_PARSE, "expected %s", str);
    destroy_curtok();
    return 0;
}
#define expect(token) expect_(parser_gas, token)

static yasm_bytecode *
parse_line(yasm_parser_gas *parser_gas)
{
    yasm_bytecode *bc;
    yasm_expr *e;
    yasm_valparamhead vps;
    char *id;
    const dir_lookup *dir;

    if (is_eol())
        return NULL;

    bc = parse_instr(parser_gas);
    if (bc)
        return bc;

    switch (curtok) {
        case ID:
            id = ID_val;

            /* See if it's a gas-specific directive */
            dir = (const dir_lookup *)HAMT_search(parser_gas->dirs, id);
            if (dir) {
                parser_gas->state = dir->newstate;
                get_next_token(); /* ID */
                return dir->handler(parser_gas, dir->param);
            }

            get_next_token(); /* ID */
            if (curtok == ':') {
                /* Label */
                parser_gas->state = INITIAL;
                get_next_token(); /* : */
                define_label(parser_gas, id, 0);
                return parse_line(parser_gas);
            } else if (curtok == '=') {
                /* EQU */
                /* TODO: allow redefinition, assigning to . (same as .org) */
                parser_gas->state = INITIAL;
                get_next_token(); /* = */
                e = parse_expr(parser_gas);
                if (e)
                    yasm_symtab_define_equ(p_symtab, id, e, cur_line);
                else
                    yasm_error_set(YASM_ERROR_SYNTAX,
                                   N_("expression expected after `%s'"), "=");
                yasm_xfree(id);
                return NULL;
            }

            /* possibly a directive; try to parse it */
            parse_dirvals(parser_gas, &vps);
            if (!yasm_object_directive(p_object, id, "gas", &vps, NULL,
                                       cur_line)) {
                yasm_vps_delete(&vps);
                yasm_xfree(id);
                return NULL;
            }
            yasm_vps_delete(&vps);
            if (id[0] == '.')
                yasm_warn_set(YASM_WARN_GENERAL,
                              N_("directive `%s' not recognized"), id);
            else
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("instruction not recognized: `%s'"), id);
            yasm_xfree(id);
            return NULL;
        case LABEL:
            define_label(parser_gas, LABEL_val, 0);
            get_next_token(); /* LABEL */
            return parse_line(parser_gas);
        case CPP_LINE_MARKER:
            get_next_token();
            cpp_line_marker(parser_gas);
            return NULL;
        case NASM_LINE_MARKER:
            get_next_token();
            nasm_line_marker(parser_gas);
            return NULL;
        default:
            yasm_error_set(YASM_ERROR_SYNTAX,
                N_("label or instruction expected at start of line"));
            return NULL;
    }
}

/*
    Handle line markers generated by cpp.

    We expect a positive integer (line) followed by a string (filename). If we
    fail to find either of these, we treat the line as a comment. There is a
    possibility of false positives (mistaking a comment for a line marker, when
    the comment is not intended as a line marker) but this cannot be avoided
    without adding a filter to the input before passing it to cpp.

    This function is only called if the preprocessor was 'cpp', since the
    CPP_LINE_MARKER token isn't generated for any other preprocessor. With any
    other preprocessor, anything after a '#' is always treated as a comment.
*/
static void
cpp_line_marker(yasm_parser_gas *parser_gas)
{
    yasm_valparamhead vps;
    yasm_valparam *vp;
    unsigned long line;
    char *filename;

    /* Line number. */
    if (curtok != INTNUM) {
        /* Skip over a comment. */
        while (curtok != '\n')
            get_next_token();

        return;
    }

    if (yasm_intnum_sign(INTNUM_val) < 0) {
        get_next_token(); /* INTNUM */
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("line number is negative"));
        return;
    }

    line = yasm_intnum_get_uint(INTNUM_val);

    /*
        Set to (line - 1) since the directive indicates that the *next* line
        will have the number given.

        cpp should never produce line=0, but the if keeps us safe just incase.
    */
    if (line != 0)
        line--;

    yasm_intnum_destroy(INTNUM_val);
    get_next_token(); /* INTNUM */

    /* File name, in quotes. */
    if (curtok != STRING) {
        /* Skip over a comment. */
        while (curtok != '\n')
            get_next_token();

        return;
    }

    filename = STRING_val.contents;
    get_next_token();

    /* Set linemap. */
    yasm_linemap_set(parser_gas->linemap, filename, 0, line, 1);

    /*
        The first line marker in the file (which should be on the first line
        of the file) will give us the name of the source file. This information
        needs to be passed on to the debug format module.
    */
    if (parser_gas->seen_line_marker == 0) {
        parser_gas->seen_line_marker = 1;

        yasm_vps_initialize(&vps);
        vp = yasm_vp_create_string(NULL, filename);
        yasm_vps_append(&vps, vp);

        yasm_object_directive(p_object, ".file", "gas", &vps, NULL, cur_line);

        yasm_vps_delete(&vps);
    } else
        yasm_xfree(filename);

    /* Skip flags. */
    while (1) {
        switch (curtok) {
            case INTNUM:
                break;

            case '\n':
                return;

            default:
                yasm_error_set(YASM_ERROR_SYNTAX,
                    N_("junk at end of cpp line marker"));
                return;
        }
        get_next_token();
    }
}

/*
    Handle line markers generated by the nasm preproc.

    We expect a positive integer (line) followed by a plus sign, followed by
    another positive integer, followed by a string (filename).

    This function is only called if the preprocessor was 'nasm', since the
    NASM_LINE_MARKER token isn't generated for any other preprocessor.
*/
static void
nasm_line_marker(yasm_parser_gas *parser_gas)
{
    yasm_valparamhead vps;
    yasm_valparam *vp;
    unsigned long line, incr;
    char *filename;

    /* Line number. */
    if (!expect(INTNUM)) return;

    if (yasm_intnum_sign(INTNUM_val) < 0) {
        get_next_token(); /* INTNUM */
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("line number is negative"));
        return;
    }

    line = yasm_intnum_get_uint(INTNUM_val);

    /*
        Set to (line - 1) since the directive indicates that the *next* line
        will have the number given.

        cpp should never produce line=0, but the if keeps us safe just incase.
    */
    if (line != 0)
        line--;

    yasm_intnum_destroy(INTNUM_val);
    get_next_token(); /* INTNUM */

    if (!expect('+')) return;
    get_next_token(); /* + */

    /* Line number increment. */
    if (!expect(INTNUM)) return;

    if (yasm_intnum_sign(INTNUM_val) < 0) {
        get_next_token(); /* INTNUM */
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("line increment is negative"));
        return;
    }

    incr = yasm_intnum_get_uint(INTNUM_val);
    yasm_intnum_destroy(INTNUM_val);

    /* File name is not in quotes, so need to switch to a different tokenizer
     * state.
     */
    parser_gas->state = NASM_FILENAME;
    get_next_token(); /* INTNUM */
    if (!expect(STRING)) {
        parser_gas->state = INITIAL;
        return;
    }

    filename = STRING_val.contents;

    /* Set linemap. */
    yasm_linemap_set(parser_gas->linemap, filename, 0, line, incr);

    /*
        The first line marker in the file (which should be on the first line
        of the file) will give us the name of the source file. This information
        needs to be passed on to the debug format module.
    */
    if (parser_gas->seen_line_marker == 0) {
        parser_gas->seen_line_marker = 1;

        yasm_vps_initialize(&vps);
        vp = yasm_vp_create_string(NULL, filename);
        yasm_vps_append(&vps, vp);

        yasm_object_directive(p_object, ".file", "gas", &vps, NULL, cur_line);

        yasm_vps_delete(&vps);
    } else
        yasm_xfree(filename);

    /* We need to poke back on the \n that was consumed by the tokenizer */
    parser_gas->peek_token = '\n';
    get_next_token();
}

/* Line directive */
static yasm_bytecode *
dir_line(yasm_parser_gas *parser_gas, unsigned int param)
{
    if (!expect(INTNUM)) return NULL;
    if (yasm_intnum_sign(INTNUM_val) < 0) {
        get_next_token(); /* INTNUM */
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("line number is negative"));
        return NULL;
    }

    parser_gas->dir_line = yasm_intnum_get_uint(INTNUM_val);
    yasm_intnum_destroy(INTNUM_val);
    get_next_token(); /* INTNUM */

    if (parser_gas->dir_fileline == 3) {
        /* Have both file and line */
        yasm_linemap_set(parser_gas->linemap, NULL, 0,
                         parser_gas->dir_line, 1);
    } else if (parser_gas->dir_fileline == 1) {
        /* Had previous file directive only */
        parser_gas->dir_fileline = 3;
        yasm_linemap_set(parser_gas->linemap, parser_gas->dir_file, 0,
                         parser_gas->dir_line, 1);
    } else {
        /* Didn't see file yet */
        parser_gas->dir_fileline = 2;
    }
    return NULL;
}

/* Alignment directives */

static yasm_bytecode *
dir_align(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_expr *bound, *fill=NULL, *maxskip=NULL;

    bound = parse_expr(parser_gas);
    if (!bound) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_(".align directive must specify alignment"));
        return NULL;
    }

    if (curtok == ',') {
        get_next_token(); /* ',' */
        fill = parse_expr(parser_gas);
        if (curtok == ',') {
            get_next_token(); /* ',' */
            maxskip = parse_expr(parser_gas);
        }
    }

    return gas_parser_align(parser_gas, cursect, bound, fill, maxskip,
                            (int)param);
}

static yasm_bytecode *
dir_org(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_intnum *start, *value=NULL;
    yasm_bytecode *bc;

    /* TODO: support expr instead of intnum */
    if (!expect(INTNUM)) return NULL;
    start = INTNUM_val;
    get_next_token(); /* INTNUM */

    if (curtok == ',') {
        get_next_token(); /* ',' */
        /* TODO: support expr instead of intnum */
        if (!expect(INTNUM)) return NULL;
        value = INTNUM_val;
        get_next_token(); /* INTNUM */
    }
    if (value) {
        bc = yasm_bc_create_org(yasm_intnum_get_uint(start),
                                yasm_intnum_get_uint(value), cur_line);
        yasm_intnum_destroy(value);
    } else
        bc = yasm_bc_create_org(yasm_intnum_get_uint(start), 0,
                                cur_line);
    yasm_intnum_destroy(start);
    return bc;
}

/* Data visibility directives */

static yasm_bytecode *
dir_local(yasm_parser_gas *parser_gas, unsigned int param)
{
    if (!expect(ID)) return NULL;
    yasm_symtab_declare(p_symtab, ID_val, YASM_SYM_DLOCAL, cur_line);
    yasm_xfree(ID_val);
    get_next_token(); /* ID */
    return NULL;
}

static yasm_bytecode *
dir_comm(yasm_parser_gas *parser_gas, unsigned int is_lcomm)
{
    yasm_expr *align = NULL;
    /*@null@*/ /*@dependent@*/ yasm_symrec *sym;
    char *id;
    yasm_expr *e;

    if (!expect(ID)) return NULL;
    id = ID_val;
    get_next_token(); /* ID */
    if (!expect(',')) {
        yasm_xfree(id);
        return NULL;
    }
    get_next_token(); /* , */
    e = parse_expr(parser_gas);
    if (!e) {
        yasm_error_set(YASM_ERROR_SYNTAX, N_("size expected for `%s'"),
                       ".COMM");
        return NULL;
    }
    if (curtok == ',') {
        /* Optional alignment expression */
        get_next_token(); /* ',' */
        align = parse_expr(parser_gas);
    }
    /* If already explicitly declared local, treat like LCOMM */
    if (is_lcomm
        || ((sym = yasm_symtab_get(p_symtab, id))
            && yasm_symrec_get_visibility(sym) == YASM_SYM_DLOCAL)) {
        define_lcomm(parser_gas, id, e, align);
    } else if (align) {
        /* Give third parameter as objext valparam */
        yasm_valparamhead *extvps = yasm_vps_create();
        yasm_valparam *vp = yasm_vp_create_expr(NULL, align);
        yasm_vps_append(extvps, vp);

        sym = yasm_symtab_declare(p_symtab, id, YASM_SYM_COMMON,
                                  cur_line);
        yasm_symrec_set_common_size(sym, e);
        yasm_symrec_set_objext_valparams(sym, extvps);

        yasm_xfree(id);
    } else {
        sym = yasm_symtab_declare(p_symtab, id, YASM_SYM_COMMON,
                                  cur_line);
        yasm_symrec_set_common_size(sym, e);
        yasm_xfree(id);
    }
    return NULL;
}

/* Integer data definition directives */

static yasm_bytecode *
dir_ascii(yasm_parser_gas *parser_gas, unsigned int withzero)
{
    yasm_datavalhead dvs;
    if (!parse_strvals(parser_gas, &dvs))
        return NULL;
    return yasm_bc_create_data(&dvs, 1, (int)withzero, p_object->arch,
                               cur_line);
}

static yasm_bytecode *
dir_data(yasm_parser_gas *parser_gas, unsigned int size)
{
    yasm_datavalhead dvs;
    if (!parse_datavals(parser_gas, &dvs))
        return NULL;
    return yasm_bc_create_data(&dvs, size, 0, p_object->arch, cur_line);
}

static yasm_bytecode *
dir_leb128(yasm_parser_gas *parser_gas, unsigned int sign)
{
    yasm_datavalhead dvs;
    if (!parse_datavals(parser_gas, &dvs))
        return NULL;
    return yasm_bc_create_leb128(&dvs, (int)sign, cur_line);
}

/* Empty space / fill data definition directives */

static yasm_bytecode *
dir_zero(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_bytecode *bc;
    yasm_datavalhead dvs;
    yasm_expr *e = parse_expr(parser_gas);
    if (!e) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("expression expected after `%s'"), ".ZERO");
        return NULL;
    }

    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_expr(
        p_expr_new_ident(yasm_expr_int(yasm_intnum_create_uint(0)))));
    bc = yasm_bc_create_data(&dvs, 1, 0, p_object->arch, cur_line);
    yasm_bc_set_multiple(bc, e);
    return bc;
}

static yasm_bytecode *
dir_skip(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_expr *e, *e_val;
    yasm_bytecode *bc;
    yasm_datavalhead dvs;

    e = parse_expr(parser_gas);
    if (!e) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("expression expected after `%s'"), ".SKIP");
        return NULL;
    }
    if (curtok != ',')
        return yasm_bc_create_reserve(e, 1, cur_line);
    get_next_token(); /* ',' */
    e_val = parse_expr(parser_gas);
    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_expr(e_val));
    bc = yasm_bc_create_data(&dvs, 1, 0, p_object->arch, cur_line);

    yasm_bc_set_multiple(bc, e);
    return bc;
}

/* fill data definition directive */
static yasm_bytecode *
dir_fill(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_expr *sz=NULL, *val=NULL;
    yasm_expr *e = parse_expr(parser_gas);
    if (!e) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("expression expected after `%s'"), ".FILL");
        return NULL;
    }
    if (curtok == ',') {
        get_next_token(); /* ',' */
        sz = parse_expr(parser_gas);
        if (curtok == ',') {
            get_next_token(); /* ',' */
            val = parse_expr(parser_gas);
        }
    }
    return gas_parser_dir_fill(parser_gas, e, sz, val);
}

/* Section directives */

static yasm_bytecode *
dir_bss_section(yasm_parser_gas *parser_gas, unsigned int param)
{
    gas_switch_section(parser_gas, ".bss", NULL, NULL, NULL, 1);
    return NULL;
}

static yasm_bytecode *
dir_data_section(yasm_parser_gas *parser_gas, unsigned int param)
{
    gas_switch_section(parser_gas, ".data", NULL, NULL, NULL, 1);
    return NULL;
}

static yasm_bytecode *
dir_text_section(yasm_parser_gas *parser_gas, unsigned int param)
{
    gas_switch_section(parser_gas, ".text", NULL, NULL, NULL, 1);
    return NULL;
}

static yasm_bytecode *
dir_section(yasm_parser_gas *parser_gas, unsigned int param)
{
    /* DIR_SECTION ID ',' STRING ',' '@' ID ',' dirvals */
    char *sectname, *flags = NULL, *type = NULL;
    yasm_valparamhead vps;
    int have_vps = 0;

    if (!expect(ID)) return NULL;
    sectname = ID_val;
    get_next_token(); /* ID */

    if (curtok == ',') {
        get_next_token(); /* ',' */
        if (!expect(STRING)) {
            yasm_error_set(YASM_ERROR_SYNTAX,
                           N_("flag string expected"));
            yasm_xfree(sectname);
            return NULL;
        }
        flags = STRING_val.contents;
        get_next_token(); /* STRING */
    }

    if (curtok == ',') {
        get_next_token(); /* ',' */
        if (!expect('@')) {
            yasm_xfree(sectname);
            yasm_xfree(flags);
            return NULL;
        }
        get_next_token(); /* '@' */
        if (!expect(ID)) {
            yasm_xfree(sectname);
            yasm_xfree(flags);
            return NULL;
        }
        type = ID_val;
        get_next_token(); /* ID */
    }

    if (curtok == ',') {
        get_next_token(); /* ',' */
        if (parse_dirvals(parser_gas, &vps))
            have_vps = 1;
    }

    gas_switch_section(parser_gas, sectname, flags, type,
                       have_vps ? &vps : NULL, 0);
    yasm_xfree(sectname);
    yasm_xfree(flags);
    return NULL;
}

/* Other directives */

static yasm_bytecode *
dir_equ(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_expr *e;
    char *id;

    /* ID ',' expr */
    if (!expect(ID)) return NULL;
    id = ID_val;
    get_next_token(); /* ID */
    if (!expect(',')) {
        yasm_xfree(id);
        return NULL;
    }
    get_next_token(); /* ',' */
    e = parse_expr(parser_gas);
    if (e)
        yasm_symtab_define_equ(p_symtab, id, e, cur_line);
    else
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("expression expected after `%s'"), ",");
    yasm_xfree(id);
    return NULL;
}

static yasm_bytecode *
dir_file(yasm_parser_gas *parser_gas, unsigned int param)
{
    yasm_valparamhead vps;
    yasm_valparam *vp;

    if (curtok == STRING) {
        /* No file number; this form also sets the assembler's
         * internal line number.
         */
        char *filename = STRING_val.contents;

        get_next_token(); /* STRING */
        if (parser_gas->dir_fileline == 3) {
            /* Have both file and line */
            const char *old_fn;
            unsigned long old_line;

            yasm_linemap_lookup(parser_gas->linemap, cur_line, &old_fn,
                                &old_line);
            yasm_linemap_set(parser_gas->linemap, filename, 0, old_line,
                             1);
        } else if (parser_gas->dir_fileline == 2) {
            /* Had previous line directive only */
            parser_gas->dir_fileline = 3;
            yasm_linemap_set(parser_gas->linemap, filename, 0,
                             parser_gas->dir_line, 1);
        } else {
            /* Didn't see line yet, save file */
            parser_gas->dir_fileline = 1;
            if (parser_gas->dir_file)
                yasm_xfree(parser_gas->dir_file);
            parser_gas->dir_file = yasm__xstrdup(filename);
        }

        /* Pass change along to debug format */
        yasm_vps_initialize(&vps);
        vp = yasm_vp_create_string(NULL, filename);
        yasm_vps_append(&vps, vp);

        yasm_object_directive(p_object, ".file", "gas", &vps, NULL,
                              cur_line);

        yasm_vps_delete(&vps);
        return NULL;
    }

    /* fileno filename form */
    yasm_vps_initialize(&vps);

    if (!expect(INTNUM)) return NULL;
    vp = yasm_vp_create_expr(NULL,
        p_expr_new_ident(yasm_expr_int(INTNUM_val)));
    yasm_vps_append(&vps, vp);
    get_next_token(); /* INTNUM */

    if (!expect(STRING)) {
        yasm_vps_delete(&vps);
        return NULL;
    }
    vp = yasm_vp_create_string(NULL, STRING_val.contents);
    yasm_vps_append(&vps, vp);
    get_next_token(); /* STRING */

    yasm_object_directive(p_object, ".file", "gas", &vps, NULL,
                          cur_line);

    yasm_vps_delete(&vps);
    return NULL;
}


static yasm_bytecode *
dir_intel_syntax(yasm_parser_gas *parser_gas, unsigned int param)
{
    parser_gas->intel_syntax = 1;

    do {
        destroy_curtok();
        get_next_token();
    } while (!is_eol());
    return NULL;
}

static yasm_bytecode *
dir_att_syntax(yasm_parser_gas *parser_gas, unsigned int param)
{
    parser_gas->intel_syntax = 0;
    return NULL;
}

static yasm_bytecode *
parse_instr(yasm_parser_gas *parser_gas)
{
    yasm_bytecode *bc;
    char *id;
    size_t id_len;
    uintptr_t prefix;

    if (parser_gas->intel_syntax) {
        bc = parse_instr_intel(parser_gas);
        if (bc) {
            yasm_warn_disable(YASM_WARN_UNREC_CHAR);
             do {
                destroy_curtok();
                get_next_token();
            } while (!is_eol());
            yasm_warn_enable(YASM_WARN_UNREC_CHAR);
        }
        return bc;
    }

    if (curtok != ID)
        return NULL;

    id = ID_val;
    id_len = ID_len;

    /* instructions/prefixes must start with a letter */
    if (!isalpha(id[0]))
        return NULL;

    /* check to be sure it's not a label or equ */
    get_peek_token(parser_gas);
    if (parser_gas->peek_token == ':' || parser_gas->peek_token == '=')
        return NULL;

    switch (yasm_arch_parse_check_insnprefix
            (p_object->arch, ID_val, ID_len, cur_line, &bc, &prefix)) {
        case YASM_ARCH_INSN:
        {
            yasm_insn *insn;

            /* Propagate errors in case we got a warning from the arch */
            yasm_errwarn_propagate(parser_gas->errwarns, cur_line);

            insn = yasm_bc_get_insn(bc);

            yasm_xfree(id);
            get_next_token();   /* ID */
            if (is_eol())
                return bc;      /* no operands */

            /* parse operands */
            for (;;) {
                yasm_insn_operand *op = parse_operand(parser_gas);
                if (!op) {
                    yasm_error_set(YASM_ERROR_SYNTAX,
                                   N_("expression syntax error"));
                    yasm_bc_destroy(bc);
                    return NULL;
                }
                yasm_insn_ops_append(insn, op);

                if (is_eol())
                    break;
                if (!expect(',')) {
                    yasm_bc_destroy(bc);
                    return NULL;
                }
                get_next_token();
            }
            return bc;
        }
        case YASM_ARCH_PREFIX:
            /* Propagate errors in case we got a warning from the arch */
            yasm_errwarn_propagate(parser_gas->errwarns, cur_line);

            yasm_xfree(id);
            get_next_token();   /* ID */
            bc = parse_instr(parser_gas);
            if (!bc)
                bc = yasm_arch_create_empty_insn(p_object->arch, cur_line);
            yasm_insn_add_prefix(yasm_bc_get_insn(bc), prefix);
            return bc;
        default:
            break;
    }

    /* Check for segment register used as prefix */
    switch (yasm_arch_parse_check_regtmod(p_object->arch, ID_val, ID_len,
                                          &prefix)) {
        case YASM_ARCH_SEGREG:
            yasm_xfree(id);
            get_next_token();   /* ID */
            bc = parse_instr(parser_gas);
            if (!bc)
                bc = yasm_arch_create_empty_insn(p_object->arch, cur_line);
            yasm_insn_add_seg_prefix(yasm_bc_get_insn(bc), prefix);
            return bc;
        default:
            return NULL;
    }
}

static int
parse_dirvals(yasm_parser_gas *parser_gas, yasm_valparamhead *vps)
{
    yasm_valparam *vp;
    yasm_expr *e;
    int num = 0;

    yasm_vps_initialize(vps);

    for (;;) {
        switch (curtok) {
            case ID:
                get_peek_token(parser_gas);
                switch (parser_gas->peek_token) {
                    case '+': case '-':
                    case '|': case '^': case '&': case '!':
                    case '*': case '/': case '%': case LEFT_OP: case RIGHT_OP:
                        e = parse_expr(parser_gas);
                        vp = yasm_vp_create_expr(NULL, e);
                        break;
                    default:
                        /* Just an ID */
                        vp = yasm_vp_create_id(NULL, ID_val, '\0');
                        get_next_token(); /* ID */
                        break;
                }
                break;
            case STRING:
                vp = yasm_vp_create_string(NULL, STRING_val.contents);
                get_next_token(); /* STRING */
                break;
            case REG:
                e = p_expr_new_ident(yasm_expr_reg(REG_val));
                vp = yasm_vp_create_expr(NULL, e);
                get_next_token(); /* REG */
                break;
            case '@':
                /* XXX: is throwing it away *really* the right thing? */
                get_next_token(); /* @ */
                continue;
            default:
                e = parse_expr(parser_gas);
                if (!e)
                    return num;
                vp = yasm_vp_create_expr(NULL, e);
                break;
        }
        yasm_vps_append(vps, vp);
        num++;
        if (curtok == ',')
            get_next_token(); /* ',' */
    }
    return num;
}

static int
parse_datavals(yasm_parser_gas *parser_gas, yasm_datavalhead *dvs)
{
    yasm_expr *e;
    yasm_dataval *dv;
    int num = 0;

    yasm_dvs_initialize(dvs);

    for (;;) {
        e = parse_expr(parser_gas);
        if (!e) {
            yasm_dvs_delete(dvs);
            yasm_dvs_initialize(dvs);
            return 0;
        }
        dv = yasm_dv_create_expr(e);
        yasm_dvs_append(dvs, dv);
        num++;
        if (curtok != ',')
            break;
        get_next_token(); /* ',' */
    }
    return num;
}

static int
parse_strvals(yasm_parser_gas *parser_gas, yasm_datavalhead *dvs)
{
    yasm_dataval *dv;
    int num = 0;

    yasm_dvs_initialize(dvs);

    for (;;) {
        if (!expect(STRING)) {
            yasm_dvs_delete(dvs);
            yasm_dvs_initialize(dvs);
            return 0;
        }
        dv = yasm_dv_create_string(STRING_val.contents, STRING_val.len);
        yasm_dvs_append(dvs, dv);
        get_next_token(); /* STRING */
        num++;
        if (curtok != ',')
            break;
        get_next_token(); /* ',' */
    }
    return num;
}

/* instruction operands */
/* memory addresses */
static yasm_effaddr *
parse_memaddr(yasm_parser_gas *parser_gas)
{
    yasm_effaddr *ea = NULL;
    yasm_expr *e1, *e2;
    int strong = 0;

    if (curtok == SEGREG) {
        uintptr_t segreg = SEGREG_val;
        get_next_token(); /* SEGREG */
        if (!expect(':')) return NULL;
        get_next_token(); /* ':' */
        ea = parse_memaddr(parser_gas);
        if (!ea)
            return NULL;
        yasm_ea_set_segreg(ea, segreg);
        return ea;
    }

    /* We want to parse a leading expression, except when it's actually
     * just a memory address (with no preceding expression) such as
     * (REG...) or (,...).
     */
    get_peek_token(parser_gas);
    if (curtok != '(' || (parser_gas->peek_token != REG
                          && parser_gas->peek_token != ','))
        e1 = parse_expr(parser_gas);
    else
        e1 = NULL;

    if (curtok == '(') {
        int havereg = 0;
        uintptr_t reg = 0;
        yasm_intnum *scale = NULL;

        get_next_token(); /* '(' */

        /* base register */
        if (curtok == REG) {
            e2 = p_expr_new_ident(yasm_expr_reg(REG_val));
            get_next_token(); /* REG */
        } else
            e2 = p_expr_new_ident(yasm_expr_int(yasm_intnum_create_uint(0)));

        if (curtok == ')')
            goto done;

        if (!expect(',')) {
            yasm_error_set(YASM_ERROR_SYNTAX, N_("invalid memory expression"));
            if (e1) yasm_expr_destroy(e1);
            yasm_expr_destroy(e2);
            return NULL;
        }
        get_next_token(); /* ',' */

        if (curtok == ')')
            goto done;

        /* index register */
        if (curtok == REG) {
            reg = REG_val;
            havereg = 1;
            get_next_token(); /* REG */
            if (curtok != ',') {
                scale = yasm_intnum_create_uint(1);
                goto done;
            }
            get_next_token(); /* ',' */
        }

        /* scale */
        if (!expect(INTNUM)) {
            yasm_error_set(YASM_ERROR_SYNTAX, N_("non-integer scale"));
            if (e1) yasm_expr_destroy(e1);
            yasm_expr_destroy(e2);
            return NULL;
        }
        scale = INTNUM_val;
        get_next_token(); /* INTNUM */

done:
        if (!expect(')')) {
            yasm_error_set(YASM_ERROR_SYNTAX, N_("invalid memory expression"));
            if (scale) yasm_intnum_destroy(scale);
            if (e1) yasm_expr_destroy(e1);
            yasm_expr_destroy(e2);
            return NULL;
        }
        get_next_token(); /* ')' */

        if (scale) {
            if (!havereg) {
                if (yasm_intnum_get_uint(scale) != 1)
                    yasm_warn_set(YASM_WARN_GENERAL,
                        N_("scale factor of %u without an index register"),
                        yasm_intnum_get_uint(scale));
                yasm_intnum_destroy(scale);
            } else
                e2 = p_expr_new(yasm_expr_expr(e2), YASM_EXPR_ADD,
                    yasm_expr_expr(p_expr_new(yasm_expr_reg(reg), YASM_EXPR_MUL,
                                              yasm_expr_int(scale))));
        }

        if (e1) {
            /* Ordering is critical here to correctly detecting presence of
             * RIP in RIP-relative expressions.
             */
            e1 = p_expr_new_tree(e2, YASM_EXPR_ADD, e1);
        } else
            e1 = e2;
        strong = 1;
    }

    if (!e1)
        return NULL;
    ea = yasm_arch_ea_create(p_object->arch, e1);
    if (strong)
        ea->strong = 1;
    return ea;
}

static yasm_insn_operand *
parse_operand(yasm_parser_gas *parser_gas)
{
    yasm_effaddr *ea;
    yasm_insn_operand *op;
    uintptr_t reg;

    switch (curtok) {
        case REG:
            reg = REG_val;
            get_next_token(); /* REG */
            return yasm_operand_create_reg(reg);
        case SEGREG:
            /* need to see if it's really a memory address */
            get_peek_token(parser_gas);
            if (parser_gas->peek_token == ':') {
                ea = parse_memaddr(parser_gas);
                if (!ea)
                    return NULL;
                return yasm_operand_create_mem(ea);
            }
            reg = SEGREG_val;
            get_next_token(); /* SEGREG */
            return yasm_operand_create_segreg(reg);
        case REGGROUP:
        {
            unsigned long regindex;
            reg = REGGROUP_val;
            get_next_token(); /* REGGROUP */
            if (curtok != '(')
                return yasm_operand_create_reg(reg);
            get_next_token(); /* '(' */
            if (!expect(INTNUM)) {
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("integer register index expected"));
                return NULL;
            }
            regindex = yasm_intnum_get_uint(INTNUM_val);
            get_next_token(); /* INTNUM */
            if (!expect(')')) {
                yasm_error_set(YASM_ERROR_SYNTAX,
                    N_("missing closing parenthesis for register index"));
                return NULL;
            }
            get_next_token(); /* ')' */
            reg = yasm_arch_reggroup_get_reg(p_object->arch, reg, regindex);
            if (reg == 0) {
                yasm_error_set(YASM_ERROR_SYNTAX, N_("bad register index `%u'"),
                               regindex);
                return NULL;
            }
            return yasm_operand_create_reg(reg);
        }
        case '$':
        {
            yasm_expr *e;
            get_next_token(); /* '$' */
            e = parse_expr(parser_gas);
            if (!e) {
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("expression missing after `%s'"), "$");
                return NULL;
            }
            return yasm_operand_create_imm(e);
        }
        case '*':
            get_next_token(); /* '*' */
            if (curtok == REG) {
                op = yasm_operand_create_reg(REG_val);
                get_next_token(); /* REG */
            } else {
                ea = parse_memaddr(parser_gas);
                if (!ea) {
                    yasm_error_set(YASM_ERROR_SYNTAX,
                                   N_("expression missing after `%s'"), "*");
                    return NULL;
                }
                op = yasm_operand_create_mem(ea);
            }
            op->deref = 1;
            return op;
        default:
            ea = parse_memaddr(parser_gas);
            if (!ea)
                return NULL;
            return yasm_operand_create_mem(ea);
    }
}

/* Expression grammar parsed is:
 *
 * expr  : expr0 [ {+,-} expr0...]
 * expr0 : expr1 [ {|,^,&,!} expr1...]
 * expr1 : expr2 [ {*,/,%,<<,>>} expr2...]
 * expr2 : { ~,+,- } expr2
 *       | (expr)
 *       | symbol
 *       | number
 */

static yasm_expr *
parse_expr(yasm_parser_gas *parser_gas)
{
    yasm_expr *e, *f;
    e = parse_expr0(parser_gas);
    if (!e)
        return NULL;

    while (curtok == '+' || curtok == '-') {
        int op = curtok;
        get_next_token();
        f = parse_expr0(parser_gas);
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        switch (op) {
            case '+': e = p_expr_new_tree(e, YASM_EXPR_ADD, f); break;
            case '-': e = p_expr_new_tree(e, YASM_EXPR_SUB, f); break;
        }
    }
    return e;
}

static yasm_expr *
parse_expr0(yasm_parser_gas *parser_gas)
{
    yasm_expr *e, *f;
    e = parse_expr1(parser_gas);
    if (!e)
        return NULL;

    while (curtok == '|' || curtok == '^' || curtok == '&' || curtok == '!') {
        int op = curtok;
        get_next_token();
        f = parse_expr1(parser_gas);
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        switch (op) {
            case '|': e = p_expr_new_tree(e, YASM_EXPR_OR, f); break;
            case '^': e = p_expr_new_tree(e, YASM_EXPR_XOR, f); break;
            case '&': e = p_expr_new_tree(e, YASM_EXPR_AND, f); break;
            case '!': e = p_expr_new_tree(e, YASM_EXPR_NOR, f); break;
        }
    }
    return e;
}

static yasm_expr *
parse_expr1(yasm_parser_gas *parser_gas)
{
    yasm_expr *e, *f;
    e = parse_expr2(parser_gas);
    if (!e)
        return NULL;

    while (curtok == '*' || curtok == '/' || curtok == '%' || curtok == LEFT_OP
           || curtok == RIGHT_OP) {
        int op = curtok;
        get_next_token();
        f = parse_expr2(parser_gas);
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        switch (op) {
            case '*': e = p_expr_new_tree(e, YASM_EXPR_MUL, f); break;
            case '/': e = p_expr_new_tree(e, YASM_EXPR_DIV, f); break;
            case '%': e = p_expr_new_tree(e, YASM_EXPR_MOD, f); break;
            case LEFT_OP: e = p_expr_new_tree(e, YASM_EXPR_SHL, f); break;
            case RIGHT_OP: e = p_expr_new_tree(e, YASM_EXPR_SHR, f); break;
        }
    }
    return e;
}

static yasm_expr *
parse_expr2(yasm_parser_gas *parser_gas)
{
    yasm_expr *e;
    yasm_symrec *sym;

    switch (curtok) {
        case '+':
            get_next_token();
            return parse_expr2(parser_gas);
        case '-':
            get_next_token();
            e = parse_expr2(parser_gas);
            if (!e)
                return NULL;
            return p_expr_new_branch(YASM_EXPR_NEG, e);
        case '~':
            get_next_token();
            e = parse_expr2(parser_gas);
            if (!e)
                return NULL;
            return p_expr_new_branch(YASM_EXPR_NOT, e);
        case '(':
            get_next_token();
            e = parse_expr(parser_gas);
            if (!e)
                return NULL;
            if (!expect(')')) {
                yasm_error_set(YASM_ERROR_SYNTAX, N_("missing parenthesis"));
                return NULL;
            }
            get_next_token();
            return e;
        case INTNUM:
            e = p_expr_new_ident(yasm_expr_int(INTNUM_val));
            get_next_token();
            return e;
        case FLTNUM:
            e = p_expr_new_ident(yasm_expr_float(FLTNUM_val));
            get_next_token();
            return e;
        case ID:
        {
            char *name = ID_val;
            get_next_token(); /* ID */

            /* "." references the current assembly position */
            if (name[1] == '\0' && name[0] == '.')
                sym = yasm_symtab_define_curpos(p_symtab, ".",
                                                parser_gas->prev_bc, cur_line);
            else
                sym = yasm_symtab_use(p_symtab, name, cur_line);
            yasm_xfree(name);

            if (curtok == '@') {
                yasm_symrec *wrt;
                /* TODO: this is needed for shared objects, e.g. sym@PLT */
                get_next_token(); /* '@' */
                if (!expect(ID)) {
                    yasm_error_set(YASM_ERROR_SYNTAX,
                                   N_("expected identifier after `@'"));
                    return NULL;
                }
                wrt = yasm_objfmt_get_special_sym(p_object, ID_val, "gas");
                yasm_xfree(ID_val);
                get_next_token(); /* ID */
                if (!wrt) {
                    yasm_warn_set(YASM_WARN_GENERAL,
                                  N_("unrecognized identifier after `@'"));
                    return p_expr_new_ident(yasm_expr_sym(sym));
                }
                return p_expr_new(yasm_expr_sym(sym), YASM_EXPR_WRT,
                                  yasm_expr_sym(wrt));
            }

            return p_expr_new_ident(yasm_expr_sym(sym));
        }
        default:
            return NULL;
    }
}

static void
define_label(yasm_parser_gas *parser_gas, char *name, int local)
{
    if (!local) {
        if (parser_gas->locallabel_base)
            yasm_xfree(parser_gas->locallabel_base);
        parser_gas->locallabel_base_len = strlen(name);
        parser_gas->locallabel_base =
            yasm_xmalloc(parser_gas->locallabel_base_len+1);
        strcpy(parser_gas->locallabel_base, name);
    }

    yasm_symtab_define_label(p_symtab, name, parser_gas->prev_bc, 1,
                             cur_line);
    yasm_xfree(name);
}

static void
define_lcomm(yasm_parser_gas *parser_gas, /*@only@*/ char *name,
             yasm_expr *size, /*@null@*/ yasm_expr *align)
{
    /* Put into .bss section. */
    /*@dependent@*/ yasm_section *bss =
        gas_get_section(parser_gas, yasm__xstrdup(".bss"), NULL, NULL, NULL, 1);

    if (align) {
        /* XXX: assume alignment is in bytes, not power-of-two */
        yasm_section_bcs_append(bss, gas_parser_align(parser_gas, bss, align,
                                NULL, NULL, 0));
    }

    yasm_symtab_define_label(p_symtab, name, yasm_section_bcs_last(bss), 1,
                             cur_line);
    yasm_section_bcs_append(bss, yasm_bc_create_reserve(size, 1, cur_line));
    yasm_xfree(name);
}

static yasm_section *
gas_get_section(yasm_parser_gas *parser_gas, char *name,
                /*@null@*/ char *flags, /*@null@*/ char *type,
                /*@null@*/ yasm_valparamhead *objext_valparams,
                int builtin)
{
    yasm_valparamhead vps;
    yasm_valparam *vp;
    char *gasflags;
    yasm_section *new_section;

    yasm_vps_initialize(&vps);
    vp = yasm_vp_create_id(NULL, name, '\0');
    yasm_vps_append(&vps, vp);

    if (!builtin) {
        if (flags)
            gasflags = yasm__xstrdup(flags);
        else
            gasflags = yasm__xstrdup("");
        vp = yasm_vp_create_string(yasm__xstrdup("gasflags"), gasflags);
        yasm_vps_append(&vps, vp);
        if (type) {
            vp = yasm_vp_create_id(NULL, type, '\0');
            yasm_vps_append(&vps, vp);
        }
    }

    new_section = yasm_objfmt_section_switch(p_object, &vps, objext_valparams,
                                             cur_line);

    yasm_vps_delete(&vps);
    return new_section;
}

static void
gas_switch_section(yasm_parser_gas *parser_gas, const char *name,
                   /*@null@*/ char *flags, /*@null@*/ char *type,
                   /*@null@*/ yasm_valparamhead *objext_valparams,
                   int builtin)
{
    yasm_section *new_section;

    new_section = gas_get_section(parser_gas, yasm__xstrdup(name), flags, type,
                                  objext_valparams, builtin);
    if (new_section) {
        cursect = new_section;
        parser_gas->prev_bc = yasm_section_bcs_last(new_section);
    } else
        yasm_error_set(YASM_ERROR_GENERAL, N_("invalid section name `%s'"),
                       name);

    if (objext_valparams)
        yasm_vps_delete(objext_valparams);
}

static yasm_bytecode *
gas_parser_align(yasm_parser_gas *parser_gas, yasm_section *sect,
                 yasm_expr *boundval, /*@null@*/ yasm_expr *fillval,
                 /*@null@*/ yasm_expr *maxskipval, int power2)
{
    yasm_intnum *boundintn;

    /* Convert power of two to number of bytes if necessary */
    if (power2)
        boundval = yasm_expr_create(YASM_EXPR_SHL,
                                    yasm_expr_int(yasm_intnum_create_uint(1)),
                                    yasm_expr_expr(boundval), cur_line);

    /* Largest .align in the section specifies section alignment. */
    boundintn = yasm_expr_get_intnum(&boundval, 0);
    if (boundintn) {
        unsigned long boundint = yasm_intnum_get_uint(boundintn);

        /* Alignments must be a power of two. */
        if (is_exp2(boundint)) {
            if (boundint > yasm_section_get_align(sect))
                yasm_section_set_align(sect, boundint, cur_line);
        }
    }

    return yasm_bc_create_align(boundval, fillval, maxskipval,
                                yasm_section_is_code(sect) ?
                                    yasm_arch_get_fill(p_object->arch) : NULL,
                                cur_line);
}

static yasm_bytecode *
gas_parser_dir_fill(yasm_parser_gas *parser_gas, /*@only@*/ yasm_expr *repeat,
                    /*@only@*/ /*@null@*/ yasm_expr *size,
                    /*@only@*/ /*@null@*/ yasm_expr *value)
{
    yasm_datavalhead dvs;
    yasm_bytecode *bc;
    unsigned int ssize;

    if (size) {
        /*@dependent@*/ /*@null@*/ yasm_intnum *intn;
        intn = yasm_expr_get_intnum(&size, 0);
        if (!intn) {
            yasm_error_set(YASM_ERROR_NOT_ABSOLUTE,
                           N_("size must be an absolute expression"));
            yasm_expr_destroy(repeat);
            yasm_expr_destroy(size);
            if (value)
                yasm_expr_destroy(value);
            return NULL;
        }
        ssize = yasm_intnum_get_uint(intn);
    } else
        ssize = 1;

    if (!value)
        value = yasm_expr_create_ident(
            yasm_expr_int(yasm_intnum_create_uint(0)), cur_line);

    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_expr(value));
    bc = yasm_bc_create_data(&dvs, ssize, 0, p_object->arch, cur_line);

    yasm_bc_set_multiple(bc, repeat);

    return bc;
}

static dir_lookup dirs_static[] = {
    /* FIXME: Whether this is power-of-two or not depends on arch and objfmt. */
    {".align",      dir_align,  0,  INITIAL},
    {".p2align",    dir_align,  1,  INITIAL},
    {".balign",     dir_align,  0,  INITIAL},
    {".org",        dir_org,    0,  INITIAL},
    /* data visibility directives */
    {".local",      dir_local,  0,  INITIAL},
    {".comm",       dir_comm,   0,  INITIAL},
    {".lcomm",      dir_comm,   1,  INITIAL},
    /* integer data declaration directives */
    {".byte",       dir_data,   1,  INITIAL},
    {".2byte",      dir_data,   2,  INITIAL},
    {".4byte",      dir_data,   4,  INITIAL},
    {".8byte",      dir_data,   8,  INITIAL},
    {".16byte",     dir_data,   16, INITIAL},
    /* TODO: These should depend on arch */
    {".short",      dir_data,   2,  INITIAL},
    {".int",        dir_data,   4,  INITIAL},
    {".long",       dir_data,   4,  INITIAL},
    {".hword",      dir_data,   2,  INITIAL},
    {".quad",       dir_data,   8,  INITIAL},
    {".octa",       dir_data,   16, INITIAL},
    /* XXX: At least on x86, this is 2 bytes */
    {".value",      dir_data,   2,  INITIAL},
    /* ASCII data declaration directives */
    {".ascii",      dir_ascii,  0,  INITIAL},   /* no terminating zero */
    {".asciz",      dir_ascii,  1,  INITIAL},   /* add terminating zero */
    {".string",     dir_ascii,  1,  INITIAL},   /* add terminating zero */
    /* LEB128 integer data declaration directives */
    {".sleb128",    dir_leb128, 1,  INITIAL},   /* signed */
    {".uleb128",    dir_leb128, 0,  INITIAL},   /* unsigned */
    /* floating point data declaration directives */
    {".float",      dir_data,   4,  INITIAL},
    {".single",     dir_data,   4,  INITIAL},
    {".double",     dir_data,   8,  INITIAL},
    {".tfloat",     dir_data,   10, INITIAL},
    /* section directives */
    {".bss",        dir_bss_section,    0,  INITIAL},
    {".data",       dir_data_section,   0,  INITIAL},
    {".text",       dir_text_section,   0,  INITIAL},
    {".section",    dir_section,        0, SECTION_DIRECTIVE},
    /* empty space/fill directives */
    {".skip",       dir_skip,   0,  INITIAL},
    {".space",      dir_skip,   0,  INITIAL},
    {".fill",       dir_fill,   0,  INITIAL},
    {".zero",       dir_zero,   0,  INITIAL},
    /* syntax directives */
    {".intel_syntax", dir_intel_syntax, 0, INITIAL},
    {".att_syntax",   dir_att_syntax,   0, INITIAL},    
    /* other directives */
    {".equ",        dir_equ,    0,  INITIAL},
    {".file",       dir_file,   0,  INITIAL},
    {".line",       dir_line,   0,  INITIAL},
    {".set",        dir_equ,    0,  INITIAL}
};

static void
no_delete(void *data)
{
}

void
gas_parser_parse(yasm_parser_gas *parser_gas)
{
    dir_lookup word;
    unsigned int i;
    int replace = 1;

    word.name = ".word";
    word.handler = dir_data;
    word.param = yasm_arch_wordsize(p_object->arch)/8;
    word.newstate = INITIAL;

    /* Create directive lookup */
    parser_gas->dirs = HAMT_create(1, yasm_internal_error_);
    HAMT_insert(parser_gas->dirs, word.name, &word, &replace, no_delete);
    for (i=0; i<NELEMS(dirs_static); i++) {
        replace = 1;
        HAMT_insert(parser_gas->dirs, dirs_static[i].name,
                    &dirs_static[i], &replace, no_delete);
    }

    while (get_next_token() != 0) {
        yasm_bytecode *bc = NULL, *temp_bc;

        if (!is_eol()) {
            bc = parse_line(parser_gas);
            demand_eol();
        }

        yasm_errwarn_propagate(parser_gas->errwarns, cur_line);

        temp_bc = yasm_section_bcs_append(cursect, bc);
        if (temp_bc)
            parser_gas->prev_bc = temp_bc;
        if (curtok == ';')
            continue;       /* don't advance line number until \n */
        if (parser_gas->save_input)
            yasm_linemap_add_source(parser_gas->linemap,
                temp_bc,
                (char *)parser_gas->save_line[parser_gas->save_last ^ 1]);
        yasm_linemap_goto_next(parser_gas->linemap);
        parser_gas->dir_line++; /* keep track for .line followed by .file */
    }

    HAMT_destroy(parser_gas->dirs, no_delete);
}
