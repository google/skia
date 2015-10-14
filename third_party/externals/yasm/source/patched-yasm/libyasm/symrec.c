/*
 * Symbol table handling
 *
 *  Copyright (C) 2001-2007  Michael Urman, Peter Johnson
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
#include "util.h"

#include <limits.h>
#include <ctype.h>

#include "libyasm-stdint.h"
#include "coretype.h"
#include "valparam.h"
#include "hamt.h"
#include "assocdat.h"

#include "errwarn.h"
#include "intnum.h"
#include "floatnum.h"
#include "expr.h"
#include "symrec.h"

#include "bytecode.h"
#include "section.h"
#include "objfmt.h"


typedef enum {
    SYM_UNKNOWN,                /* for unknown type (COMMON/EXTERN) */
    SYM_EQU,                    /* for EQU defined symbols (expressions) */
    SYM_LABEL,                  /* for labels */
    SYM_CURPOS,                 /* for labels representing the current
                                   assembly position */
    SYM_SPECIAL                 /* for special symbols that need to be in
                                   the symbol table but otherwise have no
                                   purpose */
} sym_type;

struct yasm_symrec {
    char *name;
    sym_type type;
    yasm_sym_status status;
    yasm_sym_vis visibility;
    unsigned long def_line;     /* line where symbol was first defined */
    unsigned long decl_line;    /* line where symbol was first declared */
    unsigned long use_line;     /* line where symbol was first used */
    union {
        yasm_expr *expn;        /* equ value */

        /* bytecode immediately preceding a label */
        /*@dependent@*/ yasm_bytecode *precbc;
    } value;
    unsigned int size;          /* 0 if not user-defined */
    const char *segment;        /* for segmented systems like DOS */

    /* associated data; NULL if none */
    /*@null@*/ /*@only@*/ yasm__assoc_data *assoc_data;
};

/* Linked list of symbols not in the symbol table. */
typedef struct non_table_symrec_s {
     /*@reldef@*/ SLIST_ENTRY(non_table_symrec_s) link;
     /*@owned@*/ yasm_symrec *rec;
} non_table_symrec;

struct yasm_symtab {
    /* The symbol table: a hash array mapped trie (HAMT). */
    /*@only@*/ HAMT *sym_table;
    /* Symbols not in the table */
    SLIST_HEAD(nontablesymhead_s, non_table_symrec_s) non_table_syms;

    int case_sensitive;
};

static void
objext_valparams_destroy(void *data)
{
    yasm_vps_destroy((yasm_valparamhead *)data);
}

static void
objext_valparams_print(void *data, FILE *f, int indent_level)
{
    yasm_vps_print((yasm_valparamhead *)data, f);
}

static yasm_assoc_data_callback objext_valparams_cb = {
    objext_valparams_destroy,
    objext_valparams_print
};

static void
common_size_destroy(void *data)
{
    yasm_expr **e = (yasm_expr **)data;
    yasm_expr_destroy(*e);
    yasm_xfree(data);
}

static void
common_size_print(void *data, FILE *f, int indent_level)
{
    yasm_expr **e = (yasm_expr **)data;
    yasm_expr_print(*e, f);
}

static yasm_assoc_data_callback common_size_cb = {
    common_size_destroy,
    common_size_print
};

yasm_symtab *
yasm_symtab_create(void)
{
    yasm_symtab *symtab = yasm_xmalloc(sizeof(yasm_symtab));
    symtab->sym_table = HAMT_create(0, yasm_internal_error_);
    SLIST_INIT(&symtab->non_table_syms);
    symtab->case_sensitive = 1;
    return symtab;
}

void
yasm_symtab_set_case_sensitive(yasm_symtab *symtab, int sensitive)
{
    symtab->case_sensitive = sensitive;
}

static void
symrec_destroy_one(/*@only@*/ void *d)
{
    yasm_symrec *sym = d;
    yasm_xfree(sym->name);
    if (sym->type == SYM_EQU && (sym->status & YASM_SYM_VALUED))
        yasm_expr_destroy(sym->value.expn);
    yasm__assoc_data_destroy(sym->assoc_data);
    yasm_xfree(sym);
}

static /*@partial@*/ yasm_symrec *
symrec_new_common(/*@keep@*/ char *name, int case_sensitive)
{
    yasm_symrec *rec = yasm_xmalloc(sizeof(yasm_symrec));

    if (!case_sensitive) {
        char *c;
        for (c=name; *c; c++)
            *c = tolower(*c);
    }

    rec->name = name;
    rec->type = SYM_UNKNOWN;
    rec->def_line = 0;
    rec->decl_line = 0;
    rec->use_line = 0;
    rec->visibility = YASM_SYM_LOCAL;
    rec->size = 0;
    rec->segment = NULL;
    rec->assoc_data = NULL;
    return rec;
}

static /*@partial@*/ /*@dependent@*/ yasm_symrec *
symtab_get_or_new_in_table(yasm_symtab *symtab, /*@only@*/ char *name)
{
    yasm_symrec *rec = symrec_new_common(name, symtab->case_sensitive);
    int replace = 0;

    rec->status = YASM_SYM_NOSTATUS;

    if (!symtab->case_sensitive) {
        char *c;
        for (c=name; *c; c++)
            *c = tolower(*c);
    }

    return HAMT_insert(symtab->sym_table, name, rec, &replace,
                       symrec_destroy_one);
}

static /*@partial@*/ /*@dependent@*/ yasm_symrec *
symtab_get_or_new_not_in_table(yasm_symtab *symtab, /*@only@*/ char *name)
{
    non_table_symrec *sym = yasm_xmalloc(sizeof(non_table_symrec));
    sym->rec = symrec_new_common(name, symtab->case_sensitive);

    sym->rec->status = YASM_SYM_NOTINTABLE;

    SLIST_INSERT_HEAD(&symtab->non_table_syms, sym, link);

    return sym->rec;
}

/* create a new symrec */
/*@-freshtrans -mustfree@*/
static /*@partial@*/ /*@dependent@*/ yasm_symrec *
symtab_get_or_new(yasm_symtab *symtab, const char *name, int in_table)
{
    char *symname = yasm__xstrdup(name);

    if (in_table)
        return symtab_get_or_new_in_table(symtab, symname);
    else
        return symtab_get_or_new_not_in_table(symtab, symname);
}
/*@=freshtrans =mustfree@*/

int
yasm_symtab_traverse(yasm_symtab *symtab, void *d,
                     int (*func) (yasm_symrec *sym, void *d))
{
    return HAMT_traverse(symtab->sym_table, d, (int (*) (void *, void *))func);
}

const yasm_symtab_iter *
yasm_symtab_first(const yasm_symtab *symtab)
{
    return (const yasm_symtab_iter *)HAMT_first(symtab->sym_table);
}

/*@null@*/ const yasm_symtab_iter *
yasm_symtab_next(const yasm_symtab_iter *prev)
{
    return (const yasm_symtab_iter *)HAMT_next((const HAMTEntry *)prev);
}

yasm_symrec *
yasm_symtab_iter_value(const yasm_symtab_iter *cur)
{
    return (yasm_symrec *)HAMTEntry_get_data((const HAMTEntry *)cur);
}

yasm_symrec *
yasm_symtab_abs_sym(yasm_symtab *symtab)
{
    yasm_symrec *rec = symtab_get_or_new(symtab, "", 1);
    rec->def_line = 0;
    rec->decl_line = 0;
    rec->use_line = 0;
    rec->type = SYM_EQU;
    rec->value.expn =
        yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(0)), 0);
    rec->status |= YASM_SYM_DEFINED|YASM_SYM_VALUED|YASM_SYM_USED;
    return rec;
}

yasm_symrec *
yasm_symtab_use(yasm_symtab *symtab, const char *name, unsigned long line)
{
    yasm_symrec *rec = symtab_get_or_new(symtab, name, 1);
    if (rec->use_line == 0)
        rec->use_line = line;   /* set line number of first use */
    rec->status |= YASM_SYM_USED;
    return rec;
}

yasm_symrec *
yasm_symtab_get(yasm_symtab *symtab, const char *name)
{
    if (!symtab->case_sensitive) {
        char *_name = yasm__xstrdup(name);
        char *c;
        yasm_symrec *ret;
        for (c=_name; *c; c++)
            *c = tolower(*c);
        ret = HAMT_search(symtab->sym_table, _name);
        yasm_xfree(_name);
        return ret;
    } else
      return HAMT_search(symtab->sym_table, name);
}

static /*@dependent@*/ yasm_symrec *
symtab_define(yasm_symtab *symtab, const char *name, sym_type type,
              int in_table, unsigned long line)
{
    yasm_symrec *rec = symtab_get_or_new(symtab, name, in_table);

    /* Has it been defined before (either by DEFINED or COMMON/EXTERN)? */
    if (rec->status & YASM_SYM_DEFINED) {
        yasm_error_set_xref(rec->def_line!=0 ? rec->def_line : rec->decl_line,
                            N_("`%s' previously defined here"), name);
        yasm_error_set(YASM_ERROR_GENERAL, N_("redefinition of `%s'"),
                       name);
    } else {
        if (rec->visibility & YASM_SYM_EXTERN)
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("`%s' both defined and declared extern"), name);
        rec->def_line = line;   /* set line number of definition */
        rec->type = type;
        rec->status |= YASM_SYM_DEFINED;
        rec->size = 0;
        rec->segment = NULL;
    }
    return rec;
}

yasm_symrec *
yasm_symtab_define_equ(yasm_symtab *symtab, const char *name, yasm_expr *e,
                       unsigned long line)
{
    yasm_symrec *rec = symtab_define(symtab, name, SYM_EQU, 1, line);
    if (yasm_error_occurred())
        return rec;
    rec->value.expn = e;
    rec->status |= YASM_SYM_VALUED;
    return rec;
}

yasm_symrec *
yasm_symtab_define_label(yasm_symtab *symtab, const char *name,
                         yasm_bytecode *precbc, int in_table,
                         unsigned long line)
{
    yasm_symrec *rec = symtab_define(symtab, name, SYM_LABEL, in_table, line);
    if (yasm_error_occurred())
        return rec;
    rec->value.precbc = precbc;
    if (in_table && precbc)
        yasm_bc__add_symrec(precbc, rec);
    return rec;
}

yasm_symrec *
yasm_symtab_define_curpos(yasm_symtab *symtab, const char *name,
                          yasm_bytecode *precbc, unsigned long line)
{
    yasm_symrec *rec = symtab_define(symtab, name, SYM_CURPOS, 0, line);
    if (yasm_error_occurred())
        return rec;
    rec->value.precbc = precbc;
    return rec;
}

yasm_symrec *
yasm_symtab_define_special(yasm_symtab *symtab, const char *name,
                           yasm_sym_vis vis)
{
    yasm_symrec *rec = symtab_define(symtab, name, SYM_SPECIAL, 1, 0);
    if (yasm_error_occurred())
        return rec;
    rec->status |= YASM_SYM_VALUED;
    rec->visibility = vis;
    return rec;
}

yasm_symrec *
yasm_symtab_declare(yasm_symtab *symtab, const char *name, yasm_sym_vis vis,
                    unsigned long line)
{
    yasm_symrec *rec = symtab_get_or_new(symtab, name, 1);
    yasm_symrec_declare(rec, vis, line);
    return rec;
}

void
yasm_symrec_declare(yasm_symrec *rec, yasm_sym_vis vis, unsigned long line)
{
    /* Allowable combinations:
     *  Existing State--------------  vis  New State-------------------
     *  DEFINED GLOBAL COMMON EXTERN  GCE  DEFINED GLOBAL COMMON EXTERN
     *     0      -      0      0     GCE     0      G      C      E
     *     0      -      0      1     GE      0      G      0      E
     *     0      -      1      0     GC      0      G      C      0
     * X   0      -      1      1
     *     1      -      0      0      G      1      G      0      0
     * X   1      -      -      1
     * X   1      -      1      -
     */
    if ((vis == YASM_SYM_GLOBAL) ||
        (!(rec->status & YASM_SYM_DEFINED) &&
         (!(rec->visibility & (YASM_SYM_COMMON | YASM_SYM_EXTERN)) ||
          ((rec->visibility & YASM_SYM_COMMON) && (vis == YASM_SYM_COMMON)) ||
          ((rec->visibility & YASM_SYM_EXTERN) && (vis == YASM_SYM_EXTERN))))) {
        rec->decl_line = line;
        rec->visibility |= vis;
    } else
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("duplicate definition of `%s'; first defined on line %lu"),
            rec->name, rec->def_line!=0 ? rec->def_line : rec->decl_line);
}

typedef struct symtab_finalize_info {
    unsigned long firstundef_line;
    int undef_extern;
    yasm_errwarns *errwarns;
} symtab_finalize_info;

static int
symtab_parser_finalize_checksym(yasm_symrec *sym, /*@null@*/ void *d)
{
    symtab_finalize_info *info = (symtab_finalize_info *)d;

    /* error if a symbol is used but never defined or extern/common declared */
    if ((sym->status & YASM_SYM_USED) && !(sym->status & YASM_SYM_DEFINED) &&
        !(sym->visibility & (YASM_SYM_EXTERN | YASM_SYM_COMMON))) {
        if (info->undef_extern)
            sym->visibility |= YASM_SYM_EXTERN;
        else {
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("undefined symbol `%s' (first use)"), sym->name);
            yasm_errwarn_propagate(info->errwarns, sym->use_line);
            if (sym->use_line < info->firstundef_line)
                info->firstundef_line = sym->use_line;
        }
    }

    return 0;
}

void
yasm_symtab_parser_finalize(yasm_symtab *symtab, int undef_extern,
                            yasm_errwarns *errwarns)
{
    symtab_finalize_info info;
    info.firstundef_line = ULONG_MAX;
    info.undef_extern = undef_extern;
    info.errwarns = errwarns;
    yasm_symtab_traverse(symtab, &info, symtab_parser_finalize_checksym);
    if (info.firstundef_line < ULONG_MAX) {
        yasm_error_set(YASM_ERROR_GENERAL,
                       N_(" (Each undefined symbol is reported only once.)"));
        yasm_errwarn_propagate(errwarns, info.firstundef_line);
    }
}

void
yasm_symtab_destroy(yasm_symtab *symtab)
{
    HAMT_destroy(symtab->sym_table, symrec_destroy_one);

    while (!SLIST_EMPTY(&symtab->non_table_syms)) {
        non_table_symrec *sym = SLIST_FIRST(&symtab->non_table_syms);
        SLIST_REMOVE_HEAD(&symtab->non_table_syms, link);
        symrec_destroy_one(sym->rec);
        yasm_xfree(sym);
    }

    yasm_xfree(symtab);
}

typedef struct symrec_print_data {
    FILE *f;
    int indent_level;
} symrec_print_data;

/*@+voidabstract@*/
static int
symrec_print_wrapper(yasm_symrec *sym, /*@null@*/ void *d)
{
    symrec_print_data *data = (symrec_print_data *)d;
    assert(data != NULL);
    fprintf(data->f, "%*sSymbol `%s'\n", data->indent_level, "", sym->name);
    yasm_symrec_print(sym, data->f, data->indent_level+1);
    return 0;
}

void
yasm_symtab_print(yasm_symtab *symtab, FILE *f, int indent_level)
{
    symrec_print_data data;
    data.f = f;
    data.indent_level = indent_level;
    yasm_symtab_traverse(symtab, &data, symrec_print_wrapper);
}
/*@=voidabstract@*/

const char *
yasm_symrec_get_name(const yasm_symrec *sym)
{
    return sym->name;
}

char *
yasm_symrec_get_global_name(const yasm_symrec *sym, const yasm_object *object)
{
    if (sym->visibility & (YASM_SYM_GLOBAL|YASM_SYM_COMMON|YASM_SYM_EXTERN)) {
        char *name = yasm_xmalloc(strlen(object->global_prefix) +
                                  strlen(sym->name) +
                                  strlen(object->global_suffix) + 1);
        strcpy(name, object->global_prefix);
        strcat(name, sym->name);
        strcat(name, object->global_suffix);
        return name;
    }
    return yasm__xstrdup(sym->name);
}

yasm_sym_vis
yasm_symrec_get_visibility(const yasm_symrec *sym)
{
    return sym->visibility;
}

yasm_sym_status
yasm_symrec_get_status(const yasm_symrec *sym)
{
    return sym->status;
}

unsigned long
yasm_symrec_get_def_line(const yasm_symrec *sym)
{
    return sym->def_line;
}

unsigned long
yasm_symrec_get_decl_line(const yasm_symrec *sym)
{
    return sym->decl_line;
}

unsigned long
yasm_symrec_get_use_line(const yasm_symrec *sym)
{
    return sym->use_line;
}

const yasm_expr *
yasm_symrec_get_equ(const yasm_symrec *sym)
{
    if (sym->type == SYM_EQU && (sym->status & YASM_SYM_VALUED))
        return sym->value.expn;
    return (const yasm_expr *)NULL;
}

int
yasm_symrec_get_label(const yasm_symrec *sym,
                      yasm_symrec_get_label_bytecodep *precbc)
{
    if (!(sym->type == SYM_LABEL || sym->type == SYM_CURPOS)
        || !sym->value.precbc) {
        *precbc = (yasm_symrec_get_label_bytecodep)0xDEADBEEF;
        return 0;
    }
    *precbc = sym->value.precbc;
    return 1;
}

void
yasm_symrec_set_size(yasm_symrec *sym, int size)
{
    sym->size = size;
}

int
yasm_symrec_get_size(const yasm_symrec *sym)
{
    return sym->size;
}

void
yasm_symrec_set_segment(yasm_symrec *sym, const char *segment)
{
    sym->segment = segment;
}

const char *
yasm_symrec_get_segment(const yasm_symrec *sym)
{
    return sym->segment;
}

int
yasm_symrec_is_abs(const yasm_symrec *sym)
{
    return (sym->def_line == 0 && sym->type == SYM_EQU &&
            sym->name[0] == '\0');
}

int
yasm_symrec_is_special(const yasm_symrec *sym)
{
    return (sym->type == SYM_SPECIAL);
}

int
yasm_symrec_is_curpos(const yasm_symrec *sym)
{
    return (sym->type == SYM_CURPOS);
}

void
yasm_symrec_set_objext_valparams(yasm_symrec *sym,
                                 /*@only@*/ yasm_valparamhead *objext_valparams)
{
    yasm_symrec_add_data(sym, &objext_valparams_cb, objext_valparams);
}

yasm_valparamhead *
yasm_symrec_get_objext_valparams(yasm_symrec *sym)
{
    return yasm_symrec_get_data(sym, &objext_valparams_cb);
}

void
yasm_symrec_set_common_size(yasm_symrec *sym,
                            /*@only@*/ yasm_expr *common_size)
{
    yasm_expr **ep = yasm_xmalloc(sizeof(yasm_expr *));
    *ep = common_size;
    yasm_symrec_add_data(sym, &common_size_cb, ep);
}

yasm_expr **
yasm_symrec_get_common_size(yasm_symrec *sym)
{
    return (yasm_expr **)yasm_symrec_get_data(sym, &common_size_cb);
}

void *
yasm_symrec_get_data(yasm_symrec *sym,
                     const yasm_assoc_data_callback *callback)
{
    return yasm__assoc_data_get(sym->assoc_data, callback);
}

void
yasm_symrec_add_data(yasm_symrec *sym,
                     const yasm_assoc_data_callback *callback, void *data)
{
    sym->assoc_data = yasm__assoc_data_add(sym->assoc_data, callback, data);
}

void
yasm_symrec_print(const yasm_symrec *sym, FILE *f, int indent_level)
{
    switch (sym->type) {
        case SYM_UNKNOWN:
            fprintf(f, "%*s-Unknown (Common/Extern)-\n", indent_level, "");
            break;
        case SYM_EQU:
            fprintf(f, "%*s_EQU_\n", indent_level, "");
            fprintf(f, "%*sExpn=", indent_level, "");
            if (sym->status & YASM_SYM_VALUED)
                yasm_expr_print(sym->value.expn, f);
            else
                fprintf(f, "***UNVALUED***");
            fprintf(f, "\n");
            break;
        case SYM_LABEL:
        case SYM_CURPOS:
            fprintf(f, "%*s_%s_\n%*sSection:\n", indent_level, "",
                    sym->type == SYM_LABEL ? "Label" : "CurPos",
                    indent_level, "");
            yasm_section_print(yasm_bc_get_section(sym->value.precbc), f,
                               indent_level+1, 0);
            fprintf(f, "%*sPreceding bytecode:\n", indent_level, "");
            yasm_bc_print(sym->value.precbc, f, indent_level+1);
            break;
        case SYM_SPECIAL:
            fprintf(f, "%*s-Special-\n", indent_level, "");
            break;
    }

    fprintf(f, "%*sStatus=", indent_level, "");
    if (sym->status == YASM_SYM_NOSTATUS)
        fprintf(f, "None\n");
    else {
        if (sym->status & YASM_SYM_USED)
            fprintf(f, "Used,");
        if (sym->status & YASM_SYM_DEFINED)
            fprintf(f, "Defined,");
        if (sym->status & YASM_SYM_VALUED)
            fprintf(f, "Valued,");
        if (sym->status & YASM_SYM_NOTINTABLE)
            fprintf(f, "Not in Table,");
        fprintf(f, "\n");
    }

    fprintf(f, "%*sVisibility=", indent_level, "");
    if (sym->visibility == YASM_SYM_LOCAL)
        fprintf(f, "Local\n");
    else {
        if (sym->visibility & YASM_SYM_GLOBAL)
            fprintf(f, "Global,");
        if (sym->visibility & YASM_SYM_COMMON)
            fprintf(f, "Common,");
        if (sym->visibility & YASM_SYM_EXTERN)
            fprintf(f, "Extern,");
        fprintf(f, "\n");
    }

    if (sym->assoc_data) {
        fprintf(f, "%*sAssociated data:\n", indent_level, "");
        yasm__assoc_data_print(sym->assoc_data, f, indent_level+1);
    }

    fprintf(f, "%*sLine Index (Defined)=%lu\n", indent_level, "",
            sym->def_line);
    fprintf(f, "%*sLine Index (Declared)=%lu\n", indent_level, "",
            sym->decl_line);
    fprintf(f, "%*sLine Index (Used)=%lu\n", indent_level, "", sym->use_line);
}
