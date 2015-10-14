/*
 * Section utility functions
 *
 *  Copyright (C) 2001-2007  Peter Johnson
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

#include "libyasm-stdint.h"
#include "coretype.h"
#include "hamt.h"
#include "valparam.h"
#include "assocdat.h"

#include "linemap.h"
#include "errwarn.h"
#include "intnum.h"
#include "expr.h"
#include "value.h"
#include "symrec.h"

#include "bytecode.h"
#include "arch.h"
#include "section.h"

#include "dbgfmt.h"
#include "objfmt.h"

#include "inttree.h"


struct yasm_section {
    /*@reldef@*/ STAILQ_ENTRY(yasm_section) link;

    /*@dependent@*/ yasm_object *object;    /* Pointer to parent object */

    /*@owned@*/ char *name;     /* strdup()'ed name (given by user) */

    /* associated data; NULL if none */
    /*@null@*/ /*@only@*/ yasm__assoc_data *assoc_data;

    unsigned long align;        /* Section alignment */

    unsigned long opt_flags;    /* storage for optimizer flags */

    int code;                   /* section contains code (instructions) */
    int res_only;               /* allow only resb family of bytecodes? */
    int def;                    /* "default" section, e.g. not specified by
                                   using section directive */

    /* the bytecodes for the section's contents */
    /*@reldef@*/ STAILQ_HEAD(yasm_bytecodehead, yasm_bytecode) bcs;

    /* the relocations for the section */
    /*@reldef@*/ STAILQ_HEAD(yasm_relochead, yasm_reloc) relocs;

    void (*destroy_reloc) (/*@only@*/ void *reloc);
};

static void yasm_section_destroy(/*@only@*/ yasm_section *sect);

/* Wrapper around directive for HAMT insertion */
typedef struct yasm_directive_wrap {
    const yasm_directive *directive;
} yasm_directive_wrap;

/*
 * Standard "builtin" object directives.
 */

static void
dir_extern(yasm_object *object, yasm_valparamhead *valparams,
           yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    yasm_symrec *sym;
    sym = yasm_symtab_declare(object->symtab, yasm_vp_id(vp), YASM_SYM_EXTERN,
                              line);
    if (objext_valparams) {
        yasm_valparamhead *vps = yasm_vps_create();
        *vps = *objext_valparams;   /* structure copy */
        yasm_vps_initialize(objext_valparams);  /* don't double-free */
        yasm_symrec_set_objext_valparams(sym, vps);
    }
}

static void
dir_global(yasm_object *object, yasm_valparamhead *valparams,
           yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    yasm_symrec *sym;
    sym = yasm_symtab_declare(object->symtab, yasm_vp_id(vp), YASM_SYM_GLOBAL,
                              line);
    if (objext_valparams) {
        yasm_valparamhead *vps = yasm_vps_create();
        *vps = *objext_valparams;   /* structure copy */
        yasm_vps_initialize(objext_valparams);  /* don't double-free */
        yasm_symrec_set_objext_valparams(sym, vps);
    }
}

static void
dir_common(yasm_object *object, yasm_valparamhead *valparams,
           yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    yasm_valparam *vp2 = yasm_vps_next(vp);
    yasm_expr *size = yasm_vp_expr(vp2, object->symtab, line);
    yasm_symrec *sym;

    if (!size) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("no size specified in %s declaration"), "COMMON");
        return;
    }
    sym = yasm_symtab_declare(object->symtab, yasm_vp_id(vp), YASM_SYM_COMMON,
                              line);
    yasm_symrec_set_common_size(sym, size);
    if (objext_valparams) {
        yasm_valparamhead *vps = yasm_vps_create();
        *vps = *objext_valparams;   /* structure copy */
        yasm_vps_initialize(objext_valparams);  /* don't double-free */
        yasm_symrec_set_objext_valparams(sym, vps);
    }
}

static void
dir_section(yasm_object *object, yasm_valparamhead *valparams,
            yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_section *new_section =
        yasm_objfmt_section_switch(object, valparams, objext_valparams, line);
    if (new_section)
        object->cur_section = new_section;
    else
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("invalid argument to directive `%s'"), "SECTION");
}

static const yasm_directive object_directives[] = {
    { ".extern",        "gas",  dir_extern,     YASM_DIR_ID_REQUIRED },
    { ".global",        "gas",  dir_global,     YASM_DIR_ID_REQUIRED },
    { ".globl",         "gas",  dir_global,     YASM_DIR_ID_REQUIRED },
    { "extern",         "nasm", dir_extern,     YASM_DIR_ID_REQUIRED },
    { "global",         "nasm", dir_global,     YASM_DIR_ID_REQUIRED },
    { "common",         "nasm", dir_common,     YASM_DIR_ID_REQUIRED },
    { "section",        "nasm", dir_section,    YASM_DIR_ARG_REQUIRED },
    { "segment",        "nasm", dir_section,    YASM_DIR_ARG_REQUIRED },
    { NULL, NULL, NULL, 0 }
};

static void
directive_level2_delete(/*@only@*/ void *data)
{
    yasm_xfree(data);
}

static void
directive_level1_delete(/*@only@*/ void *data)
{
    HAMT_destroy(data, directive_level2_delete);
}

static void
directives_add(yasm_object *object, /*@null@*/ const yasm_directive *dir)
{
    if (!dir)
        return;

    while (dir->name) {
        HAMT *level2 = HAMT_search(object->directives, dir->parser);
        int replace;
        yasm_directive_wrap *wrap = yasm_xmalloc(sizeof(yasm_directive_wrap));

        if (!level2) {
            replace = 0;
            level2 = HAMT_insert(object->directives, dir->parser,
                                 HAMT_create(1, yasm_internal_error_),
                                 &replace, directive_level1_delete);
        }
        replace = 0;
        wrap->directive = dir;
        HAMT_insert(level2, dir->name, wrap, &replace,
                    directive_level2_delete);
        dir++;
    }
}

/*@-compdestroy@*/
yasm_object *
yasm_object_create(const char *src_filename, const char *obj_filename,
                   /*@kept@*/ yasm_arch *arch,
                   const yasm_objfmt_module *objfmt_module,
                   const yasm_dbgfmt_module *dbgfmt_module)
{
    yasm_object *object = yasm_xmalloc(sizeof(yasm_object));
    int matched, i;

    object->src_filename = yasm__xstrdup(src_filename);
    object->obj_filename = yasm__xstrdup(obj_filename);

    /* No prefix/suffix */
    object->global_prefix = yasm__xstrdup("");
    object->global_suffix = yasm__xstrdup("");

    /* Create empty symbol table */
    object->symtab = yasm_symtab_create();

    /* Initialize sections linked list */
    STAILQ_INIT(&object->sections);

    /* Create directives HAMT */
    object->directives = HAMT_create(1, yasm_internal_error_);

    /* Initialize the target architecture */
    object->arch = arch;

    /* Initialize things to NULL in case of error */
    object->dbgfmt = NULL;

    /* Initialize the object format */
    object->objfmt = yasm_objfmt_create(objfmt_module, object);
    if (!object->objfmt) {
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("object format `%s' does not support architecture `%s' machine `%s'"),
            objfmt_module->keyword, ((yasm_arch_base *)arch)->module->keyword,
            yasm_arch_get_machine(arch));
        goto error;
    }

    /* Get a fresh copy of objfmt_module as it may have changed. */
    objfmt_module = ((yasm_objfmt_base *)object->objfmt)->module;

    /* Add an initial "default" section to object */
    object->cur_section = yasm_objfmt_add_default_section(object);

    /* Check to see if the requested debug format is in the allowed list
     * for the active object format.
     */
    matched = 0;
    for (i=0; objfmt_module->dbgfmt_keywords[i]; i++)
        if (yasm__strcasecmp(objfmt_module->dbgfmt_keywords[i],
                             dbgfmt_module->keyword) == 0)
            matched = 1;
    if (!matched) {
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("`%s' is not a valid debug format for object format `%s'"),
            dbgfmt_module->keyword, objfmt_module->keyword);
        goto error;
    }

    /* Initialize the debug format */
    object->dbgfmt = yasm_dbgfmt_create(dbgfmt_module, object);
    if (!object->dbgfmt) {
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("debug format `%s' does not work with object format `%s'"),
            dbgfmt_module->keyword, objfmt_module->keyword);
        goto error;
    }

    /* Add directives to HAMT.  Note ordering here determines priority. */
    directives_add(object,
                   ((yasm_objfmt_base *)object->objfmt)->module->directives);
    directives_add(object,
                   ((yasm_dbgfmt_base *)object->dbgfmt)->module->directives);
    directives_add(object,
                   ((yasm_arch_base *)object->arch)->module->directives);
    directives_add(object, object_directives);

    return object;

error:
    yasm_object_destroy(object);
    return NULL;
}
/*@=compdestroy@*/

/*@-onlytrans@*/
yasm_section *
yasm_object_get_general(yasm_object *object, const char *name,
                        unsigned long align, int code, int res_only,
                        int *isnew, unsigned long line)
{
    yasm_section *s;
    yasm_bytecode *bc;

    /* Search through current sections to see if we already have one with
     * that name.
     */
    STAILQ_FOREACH(s, &object->sections, link) {
        if (strcmp(s->name, name) == 0) {
            *isnew = 0;
            return s;
        }
    }

    /* No: we have to allocate and create a new one. */

    /* Okay, the name is valid; now allocate and initialize */
    s = yasm_xcalloc(1, sizeof(yasm_section));
    STAILQ_INSERT_TAIL(&object->sections, s, link);

    s->object = object;
    s->name = yasm__xstrdup(name);
    s->assoc_data = NULL;
    s->align = align;

    /* Initialize bytecodes with one empty bytecode (acts as "prior" for first
     * real bytecode in section.
     */
    STAILQ_INIT(&s->bcs);
    bc = yasm_bc_create_common(NULL, NULL, 0);
    bc->section = s;
    bc->offset = 0;
    STAILQ_INSERT_TAIL(&s->bcs, bc, link);

    /* Initialize relocs */
    STAILQ_INIT(&s->relocs);
    s->destroy_reloc = NULL;

    s->code = code;
    s->res_only = res_only;
    s->def = 0;

    /* Initialize object format specific data */
    yasm_objfmt_init_new_section(s, line);

    *isnew = 1;
    return s;
}
/*@=onlytrans@*/

int
yasm_object_directive(yasm_object *object, const char *name,
                      const char *parser, yasm_valparamhead *valparams,
                      yasm_valparamhead *objext_valparams,
                      unsigned long line)
{
    HAMT *level2;
    yasm_directive_wrap *wrap;

    level2 = HAMT_search(object->directives, parser);
    if (!level2)
        return 1;

    wrap = HAMT_search(level2, name);
    if (!wrap)
        return 1;

    yasm_call_directive(wrap->directive, object, valparams, objext_valparams,
                        line);
    return 0;
}

void
yasm_object_set_source_fn(yasm_object *object, const char *src_filename)
{
    yasm_xfree(object->src_filename);
    object->src_filename = yasm__xstrdup(src_filename);
}

void
yasm_object_set_global_prefix(yasm_object *object, const char *prefix)
{
    yasm_xfree(object->global_prefix);
    object->global_prefix = yasm__xstrdup(prefix);
}

void
yasm_object_set_global_suffix(yasm_object *object, const char *suffix)
{
    yasm_xfree(object->global_suffix);
    object->global_suffix = yasm__xstrdup(suffix);
}

int
yasm_section_is_code(yasm_section *sect)
{
    return sect->code;
}

unsigned long
yasm_section_get_opt_flags(const yasm_section *sect)
{
    return sect->opt_flags;
}

void
yasm_section_set_opt_flags(yasm_section *sect, unsigned long opt_flags)
{
    sect->opt_flags = opt_flags;
}

int
yasm_section_is_default(const yasm_section *sect)
{
    return sect->def;
}

void
yasm_section_set_default(yasm_section *sect, int def)
{
    sect->def = def;
}

yasm_object *
yasm_section_get_object(const yasm_section *sect)
{
    return sect->object;
}

void *
yasm_section_get_data(yasm_section *sect,
                      const yasm_assoc_data_callback *callback)
{
    return yasm__assoc_data_get(sect->assoc_data, callback);
}

void
yasm_section_add_data(yasm_section *sect,
                      const yasm_assoc_data_callback *callback, void *data)
{
    sect->assoc_data = yasm__assoc_data_add(sect->assoc_data, callback, data);
}

void
yasm_object_destroy(yasm_object *object)
{
    yasm_section *cur, *next;

    /* Delete object format, debug format, and arch.  This can be called
     * due to an error in yasm_object_create(), so look out for NULLs.
     */
    if (object->objfmt)
        yasm_objfmt_destroy(object->objfmt);
    if (object->dbgfmt)
        yasm_dbgfmt_destroy(object->dbgfmt);

    /* Delete sections */
    cur = STAILQ_FIRST(&object->sections);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        yasm_section_destroy(cur);
        cur = next;
    }

    /* Delete directives HAMT */
    HAMT_destroy(object->directives, directive_level1_delete);

    /* Delete prefix/suffix */
    yasm_xfree(object->global_prefix);
    yasm_xfree(object->global_suffix);

    /* Delete associated filenames */
    yasm_xfree(object->src_filename);
    yasm_xfree(object->obj_filename);

    /* Delete symbol table */
    yasm_symtab_destroy(object->symtab);

    /* Delete architecture */
    if (object->arch)
        yasm_arch_destroy(object->arch);

    yasm_xfree(object);
}

void
yasm_object_print(const yasm_object *object, FILE *f, int indent_level)
{
    yasm_section *cur;

    /* Print symbol table */
    fprintf(f, "%*sSymbol Table:\n", indent_level, "");
    yasm_symtab_print(object->symtab, f, indent_level+1);

    /* Print sections and bytecodes */
    STAILQ_FOREACH(cur, &object->sections, link) {
        fprintf(f, "%*sSection:\n", indent_level, "");
        yasm_section_print(cur, f, indent_level+1, 1);
    }
}

void
yasm_object_finalize(yasm_object *object, yasm_errwarns *errwarns)
{
    yasm_section *sect;

    /* Iterate through sections */
    STAILQ_FOREACH(sect, &object->sections, link) {
        yasm_bytecode *cur = STAILQ_FIRST(&sect->bcs);
        yasm_bytecode *prev;

        /* Skip our locally created empty bytecode first. */
        prev = cur;
        cur = STAILQ_NEXT(cur, link);

        /* Iterate through the remainder, if any. */
        while (cur) {
            /* Finalize */
            yasm_bc_finalize(cur, prev);
            yasm_errwarn_propagate(errwarns, cur->line);
            prev = cur;
            cur = STAILQ_NEXT(cur, link);
        }
    }
}

int
yasm_object_sections_traverse(yasm_object *object, /*@null@*/ void *d,
                              int (*func) (yasm_section *sect,
                                           /*@null@*/ void *d))
{
    yasm_section *cur;

    STAILQ_FOREACH(cur, &object->sections, link) {
        int retval = func(cur, d);
        if (retval != 0)
            return retval;
    }
    return 0;
}

/*@-onlytrans@*/
yasm_section *
yasm_object_find_general(yasm_object *object, const char *name)
{
    yasm_section *cur;

    STAILQ_FOREACH(cur, &object->sections, link) {
        if (strcmp(cur->name, name) == 0)
            return cur;
    }
    return NULL;
}
/*@=onlytrans@*/

void
yasm_section_add_reloc(yasm_section *sect, yasm_reloc *reloc,
                       void (*destroy_func) (/*@only@*/ void *reloc))
{
    STAILQ_INSERT_TAIL(&sect->relocs, reloc, link);
    if (!destroy_func)
        yasm_internal_error(N_("NULL destroy function given to add_reloc"));
    else if (sect->destroy_reloc && destroy_func != sect->destroy_reloc)
        yasm_internal_error(N_("different destroy function given to add_reloc"));
    sect->destroy_reloc = destroy_func;
}

/*@null@*/ yasm_reloc *
yasm_section_relocs_first(yasm_section *sect)
{
    return STAILQ_FIRST(&sect->relocs);
}

#undef yasm_section_reloc_next
/*@null@*/ yasm_reloc *
yasm_section_reloc_next(yasm_reloc *reloc)
{
    return STAILQ_NEXT(reloc, link);
}

void
yasm_reloc_get(yasm_reloc *reloc, yasm_intnum **addrp, yasm_symrec **symp)
{
    *addrp = reloc->addr;
    *symp = reloc->sym;
}


yasm_bytecode *
yasm_section_bcs_first(yasm_section *sect)
{
    return STAILQ_FIRST(&sect->bcs);
}

yasm_bytecode *
yasm_section_bcs_last(yasm_section *sect)
{
    return STAILQ_LAST(&sect->bcs, yasm_bytecode, link);
}

yasm_bytecode *
yasm_section_bcs_append(yasm_section *sect, yasm_bytecode *bc)
{
    if (bc) {
        if (bc->callback) {
            bc->section = sect;     /* record parent section */
            STAILQ_INSERT_TAIL(&sect->bcs, bc, link);
            return bc;
        } else
            yasm_xfree(bc);
    }
    return (yasm_bytecode *)NULL;
}

int
yasm_section_bcs_traverse(yasm_section *sect,
                          /*@null@*/ yasm_errwarns *errwarns,
                          /*@null@*/ void *d,
                          int (*func) (yasm_bytecode *bc, /*@null@*/ void *d))
{
    yasm_bytecode *cur = STAILQ_FIRST(&sect->bcs);

    /* Skip our locally created empty bytecode first. */
    cur = STAILQ_NEXT(cur, link);

    /* Iterate through the remainder, if any. */
    while (cur) {
        int retval = func(cur, d);
        if (errwarns)
            yasm_errwarn_propagate(errwarns, cur->line);
        if (retval != 0)
            return retval;
        cur = STAILQ_NEXT(cur, link);
    }
    return 0;
}

const char *
yasm_section_get_name(const yasm_section *sect)
{
    return sect->name;
}

void
yasm_section_set_align(yasm_section *sect, unsigned long align,
                       unsigned long line)
{
    sect->align = align;
}

unsigned long
yasm_section_get_align(const yasm_section *sect)
{
    return sect->align;
}

static void
yasm_section_destroy(yasm_section *sect)
{
    yasm_bytecode *cur, *next;
    yasm_reloc *r_cur, *r_next;

    if (!sect)
        return;

    yasm_xfree(sect->name);
    yasm__assoc_data_destroy(sect->assoc_data);

    /* Delete bytecodes */
    cur = STAILQ_FIRST(&sect->bcs);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        yasm_bc_destroy(cur);
        cur = next;
    }

    /* Delete relocations */
    r_cur = STAILQ_FIRST(&sect->relocs);
    while (r_cur) {
        r_next = STAILQ_NEXT(r_cur, link);
        yasm_intnum_destroy(r_cur->addr);
        sect->destroy_reloc(r_cur);
        r_cur = r_next;
    }

    yasm_xfree(sect);
}

void
yasm_section_print(const yasm_section *sect, FILE *f, int indent_level,
                   int print_bcs)
{
    if (!sect) {
        fprintf(f, "%*s(none)\n", indent_level, "");
        return;
    }

    fprintf(f, "%*sname=%s\n", indent_level, "", sect->name);

    if (sect->assoc_data) {
        fprintf(f, "%*sAssociated data:\n", indent_level, "");
        yasm__assoc_data_print(sect->assoc_data, f, indent_level+1);
    }

    if (print_bcs) {
        yasm_bytecode *cur;

        fprintf(f, "%*sBytecodes:\n", indent_level, "");

        STAILQ_FOREACH(cur, &sect->bcs, link) {
            fprintf(f, "%*sNext Bytecode:\n", indent_level+1, "");
            yasm_bc_print(cur, f, indent_level+2);
        }
    }
}

/*
 * Robertson (1977) optimizer
 * Based (somewhat loosely) on the algorithm given in:
 *   MRC Technical Summary Report # 1779
 *   CODE GENERATION FOR SHORT/LONG ADDRESS MACHINES
 *   Edward L. Robertson
 *   Mathematics Research Center
 *   University of Wisconsin-Madison
 *   610 Walnut Street
 *   Madison, Wisconsin 53706
 *   August 1977
 *
 * Key components of algorithm:
 *  - start assuming all short forms
 *  - build spans for short->long transition dependencies
 *  - if a long form is needed, walk the dependencies and update
 * Major differences from Robertson's algorithm:
 *  - detection of cycles
 *  - any difference of two locations is allowed
 *  - handling of alignment/org gaps (offset setting)
 *  - handling of multiples
 *
 * Data structures:
 *  - Interval tree to store spans and associated data
 *  - Queues QA and QB
 *
 * Each span keeps track of:
 *  - Associated bytecode (bytecode that depends on the span length)
 *  - Active/inactive state (starts out active)
 *  - Sign (negative/positive; negative being "backwards" in address)
 *  - Current length in bytes
 *  - New length in bytes
 *  - Negative/Positive thresholds
 *  - Span ID (unique within each bytecode)
 *
 * How org and align and any other offset-based bytecodes are handled:
 *
 * Some portions are critical values that must not depend on any bytecode
 * offset (either relative or absolute).
 *
 * All offset-setters (ORG and ALIGN) are put into a linked list in section
 * order (e.g. increasing offset order).  Each span keeps track of the next
 * offset-setter following the span's associated bytecode.
 *
 * When a bytecode is expanded, the next offset-setter is examined.  The
 * offset-setter may be able to absorb the expansion (e.g. any offset
 * following it would not change), or it may have to move forward (in the
 * case of align) or error (in the case of org).  If it has to move forward,
 * following offset-setters must also be examined for absorption or moving
 * forward.  In either case, the ongoing offset is updated as well as the
 * lengths of any spans dependent on the offset-setter.
 *
 * Alignment/ORG value is critical value.
 * Cannot be combined with TIMES.
 *
 * How times is handled:
 *
 * TIMES: Handled separately from bytecode "raw" size.  If not span-dependent,
 *      trivial (just multiplied in at any bytecode size increase).  Span
 *      dependent times update on any change (span ID 0).  If the resultant
 *      next bytecode offset would be less than the old next bytecode offset,
 *      error.  Otherwise increase offset and update dependent spans.
 *
 * To reduce interval tree size, a first expansion pass is performed
 * before the spans are added to the tree.
 *
 * Basic algorithm outline:
 *
 * 1. Initialization:
 *  a. Number bytecodes sequentially (via bc_index) and calculate offsets
 *     of all bytecodes assuming minimum length, building a list of all
 *     dependent spans as we go.
 *     "minimum" here means absolute minimum:
 *      - align/org (offset-based) bumps offset as normal
 *      - times values (with span-dependent values) assumed to be 0
 *  b. Iterate over spans.  Set span length based on bytecode offsets
 *     determined in 1a.  If span is "certainly" long because the span
 *     is an absolute reference to another section (or external) or the
 *     distance calculated based on the minimum length is greater than the
 *     span's threshold, expand the span's bytecode, and if no further
 *     expansion can result, mark span as inactive.
 *  c. Iterate over bytecodes to update all bytecode offsets based on new
 *     (expanded) lengths calculated in 1b.
 *  d. Iterate over active spans.  Add span to interval tree.  Update span's
 *     length based on new bytecode offsets determined in 1c.  If span's
 *     length exceeds long threshold, add that span to Q.
 * 2. Main loop:
 *   While Q not empty:
 *     Expand BC dependent on span at head of Q (and remove span from Q).
 *     Update span:
 *       If BC no longer dependent on span, mark span as inactive.
 *       If BC has new thresholds for span, update span.
 *     If BC increased in size, for each active span that contains BC:
 *       Increase span length by difference between short and long BC length.
 *       If span exceeds long threshold (or is flagged to recalculate on any
 *       change), add it to tail of Q.
 * 3. Final pass over bytecodes to generate final offsets.
 */

typedef struct yasm_span yasm_span;

typedef struct yasm_offset_setter {
    /* Linked list in section order (e.g. offset order) */
    /*@reldef@*/ STAILQ_ENTRY(yasm_offset_setter) link;

    /*@dependent@*/ yasm_bytecode *bc;

    unsigned long cur_val, new_val;
    unsigned long thres;
} yasm_offset_setter;

typedef struct yasm_span_term {
    yasm_bytecode *precbc, *precbc2;
    yasm_span *span;        /* span this term is a member of */
    long cur_val, new_val;
    unsigned int subst;
} yasm_span_term;

struct yasm_span {
    /*@reldef@*/ TAILQ_ENTRY(yasm_span) link;   /* for allocation tracking */
    /*@reldef@*/ STAILQ_ENTRY(yasm_span) linkq; /* for Q */

    /*@dependent@*/ yasm_bytecode *bc;

    yasm_value depval;

    /* span term for relative portion of value */
    yasm_span_term *rel_term;
    /* span terms in absolute portion of value */
    yasm_span_term *terms;
    yasm_expr__item *items;
    unsigned int num_terms;

    long cur_val;
    long new_val;

    long neg_thres;
    long pos_thres;

    int id;

    int active;

    /* NULL-terminated array of spans that led to this span.  Used only for
     * checking for circular references (cycles) with id=0 spans.
     */
    yasm_span **backtrace;
    int backtrace_size;

    /* First offset setter following this span's bytecode */
    yasm_offset_setter *os;
};

typedef struct optimize_data {
    /*@reldef@*/ TAILQ_HEAD(yasm_span_head, yasm_span) spans;
    /*@reldef@*/ STAILQ_HEAD(yasm_span_shead, yasm_span) QA, QB;
    /*@only@*/ IntervalTree *itree;
    /*@reldef@*/ STAILQ_HEAD(offset_setters_head, yasm_offset_setter)
        offset_setters;
    long len_diff;      /* used only for optimize_term_expand */
    yasm_span *span;    /* used only for check_cycle */
    yasm_offset_setter *os;
} optimize_data;

static yasm_span *
create_span(yasm_bytecode *bc, int id, /*@null@*/ const yasm_value *value, 
            long neg_thres, long pos_thres, yasm_offset_setter *os)
{
    yasm_span *span = yasm_xmalloc(sizeof(yasm_span));

    span->bc = bc;
    if (value)
        yasm_value_init_copy(&span->depval, value);
    else
        yasm_value_initialize(&span->depval, NULL, 0);
    span->rel_term = NULL;
    span->terms = NULL;
    span->items = NULL;
    span->num_terms = 0;
    span->cur_val = 0;
    span->new_val = 0;
    span->neg_thres = neg_thres;
    span->pos_thres = pos_thres;
    span->id = id;
    span->active = 1;
    span->backtrace = NULL;
    span->backtrace_size = 0;
    span->os = os;

    return span;
}

static void
optimize_add_span(void *add_span_data, yasm_bytecode *bc, int id,
                  const yasm_value *value, long neg_thres, long pos_thres)
{
    optimize_data *optd = (optimize_data *)add_span_data;
    yasm_span *span;
    span = create_span(bc, id, value, neg_thres, pos_thres, optd->os);
    TAILQ_INSERT_TAIL(&optd->spans, span, link);
}

static void
add_span_term(unsigned int subst, yasm_bytecode *precbc,
              yasm_bytecode *precbc2, void *d)
{
    yasm_span *span = d;
    yasm_intnum *intn;

    if (subst >= span->num_terms) {
        /* Linear expansion since total number is essentially always small */
        span->num_terms = subst+1;
        span->terms = yasm_xrealloc(span->terms,
                                    span->num_terms*sizeof(yasm_span_term));
    }
    span->terms[subst].precbc = precbc;
    span->terms[subst].precbc2 = precbc2;
    span->terms[subst].span = span;
    span->terms[subst].subst = subst;

    intn = yasm_calc_bc_dist(precbc, precbc2);
    if (!intn)
        yasm_internal_error(N_("could not calculate bc distance"));
    span->terms[subst].cur_val = 0;
    span->terms[subst].new_val = yasm_intnum_get_int(intn);
    yasm_intnum_destroy(intn);
}

static void
span_create_terms(yasm_span *span)
{
    unsigned int i;

    /* Split out sym-sym terms in absolute portion of dependent value */
    if (span->depval.abs) {
        span->num_terms = yasm_expr__bc_dist_subst(&span->depval.abs, span,
                                                   add_span_term);
        if (span->num_terms > 0) {
            span->items = yasm_xmalloc(span->num_terms*sizeof(yasm_expr__item));
            for (i=0; i<span->num_terms; i++) {
                /* Create items with dummy value */
                span->items[i].type = YASM_EXPR_INT;
                span->items[i].data.intn = yasm_intnum_create_int(0);

                /* Check for circular references */
                if (span->id <= 0 &&
                    ((span->bc->bc_index > span->terms[i].precbc->bc_index &&
                      span->bc->bc_index <= span->terms[i].precbc2->bc_index) ||
                     (span->bc->bc_index > span->terms[i].precbc2->bc_index &&
                      span->bc->bc_index <= span->terms[i].precbc->bc_index)))
                    yasm_error_set(YASM_ERROR_VALUE,
                                   N_("circular reference detected"));
            }
        }
    }

    /* Create term for relative portion of dependent value */
    if (span->depval.rel) {
        yasm_bytecode *rel_precbc;
        int sym_local;

        sym_local = yasm_symrec_get_label(span->depval.rel, &rel_precbc);
        if (span->depval.wrt || span->depval.seg_of || span->depval.section_rel
            || !sym_local)
            return;     /* we can't handle SEG, WRT, or external symbols */
        if (rel_precbc->section != span->bc->section)
            return;     /* not in this section */
        if (!span->depval.curpos_rel)
            return;     /* not PC-relative */

        span->rel_term = yasm_xmalloc(sizeof(yasm_span_term));
        span->rel_term->precbc = NULL;
        span->rel_term->precbc2 = rel_precbc;
        span->rel_term->span = span;
        span->rel_term->subst = ~0U;

        span->rel_term->cur_val = 0;
        span->rel_term->new_val = yasm_bc_next_offset(rel_precbc) -
            span->bc->offset;
    }
}

/* Recalculate span value based on current span replacement values.
 * Returns 1 if span needs expansion (e.g. exceeded thresholds).
 */
static int
recalc_normal_span(yasm_span *span)
{
    span->new_val = 0;

    if (span->depval.abs) {
        yasm_expr *abs_copy = yasm_expr_copy(span->depval.abs);
        /*@null@*/ /*@dependent@*/ yasm_intnum *num;

        /* Update sym-sym terms and substitute back into expr */
        unsigned int i;
        for (i=0; i<span->num_terms; i++)
            yasm_intnum_set_int(span->items[i].data.intn,
                                span->terms[i].new_val);
        yasm_expr__subst(abs_copy, span->num_terms, span->items);
        num = yasm_expr_get_intnum(&abs_copy, 0);
        if (num)
            span->new_val = yasm_intnum_get_int(num);
        else
            span->new_val = LONG_MAX; /* too complex; force to longest form */
        yasm_expr_destroy(abs_copy);
    }

    if (span->rel_term) {
        if (span->new_val != LONG_MAX && span->rel_term->new_val != LONG_MAX)
            span->new_val += span->rel_term->new_val >> span->depval.rshift;
        else
            span->new_val = LONG_MAX;   /* too complex; force to longest form */
    } else if (span->depval.rel)
        span->new_val = LONG_MAX;   /* too complex; force to longest form */

    if (span->new_val == LONG_MAX)
        span->active = 0;

    /* If id<=0, flag update on any change */
    if (span->id <= 0)
        return (span->new_val != span->cur_val);

    return (span->new_val < span->neg_thres
            || span->new_val > span->pos_thres);
}

/* Updates all bytecode offsets.  For offset-based bytecodes, calls expand
 * to determine new length.
 */
static int
update_all_bc_offsets(yasm_object *object, yasm_errwarns *errwarns)
{
    yasm_section *sect;
    int saw_error = 0;

    STAILQ_FOREACH(sect, &object->sections, link) {
        unsigned long offset = 0;

        yasm_bytecode *bc = STAILQ_FIRST(&sect->bcs);
        yasm_bytecode *prevbc;

        /* Skip our locally created empty bytecode first. */
        prevbc = bc;
        bc = STAILQ_NEXT(bc, link);

        /* Iterate through the remainder, if any. */
        while (bc) {
            if (bc->callback->special == YASM_BC_SPECIAL_OFFSET) {
                /* Recalculate/adjust len of offset-based bytecodes here */
                long neg_thres = 0;
                long pos_thres = (long)yasm_bc_next_offset(bc);
                int retval = yasm_bc_expand(bc, 1, 0,
                                            (long)yasm_bc_next_offset(prevbc),
                                            &neg_thres, &pos_thres);
                yasm_errwarn_propagate(errwarns, bc->line);
                if (retval < 0)
                    saw_error = 1;
            }
            bc->offset = offset;
            offset += bc->len*bc->mult_int;
            prevbc = bc;
            bc = STAILQ_NEXT(bc, link);
        }
    }
    return saw_error;
}

static void
span_destroy(/*@only@*/ yasm_span *span)
{
    unsigned int i;

    yasm_value_delete(&span->depval);
    if (span->rel_term)
        yasm_xfree(span->rel_term);
    if (span->terms)
        yasm_xfree(span->terms);
    if (span->items) {
        for (i=0; i<span->num_terms; i++)
            yasm_intnum_destroy(span->items[i].data.intn);
        yasm_xfree(span->items);
    }
    if (span->backtrace)
        yasm_xfree(span->backtrace);
    yasm_xfree(span);
}

static void
optimize_cleanup(optimize_data *optd)
{
    yasm_span *s1, *s2;
    yasm_offset_setter *os1, *os2;

    IT_destroy(optd->itree);

    s1 = TAILQ_FIRST(&optd->spans);
    while (s1) {
        s2 = TAILQ_NEXT(s1, link);
        span_destroy(s1);
        s1 = s2;
    }

    os1 = STAILQ_FIRST(&optd->offset_setters);
    while (os1) {
        os2 = STAILQ_NEXT(os1, link);
        yasm_xfree(os1);
        os1 = os2;
    }
}

static void
optimize_itree_add(IntervalTree *itree, yasm_span *span, yasm_span_term *term)
{
    long precbc_index, precbc2_index;
    unsigned long low, high;

    /* Update term length */
    if (term->precbc)
        precbc_index = term->precbc->bc_index;
    else
        precbc_index = span->bc->bc_index-1;

    if (term->precbc2)
        precbc2_index = term->precbc2->bc_index;
    else
        precbc2_index = span->bc->bc_index-1;

    if (precbc_index < precbc2_index) {
        low = precbc_index+1;
        high = precbc2_index;
    } else if (precbc_index > precbc2_index) {
        low = precbc2_index+1;
        high = precbc_index;
    } else
        return;     /* difference is same bc - always 0! */

    IT_insert(itree, (long)low, (long)high, term);
}

static void
check_cycle(IntervalTreeNode *node, void *d)
{
    optimize_data *optd = d;
    yasm_span_term *term = node->data;
    yasm_span *depspan = term->span;
    int i;
    int depspan_bt_alloc;

    /* Only check for cycles in id=0 spans */
    if (depspan->id > 0)
        return;

    /* Check for a circular reference by looking to see if this dependent
     * span is in our backtrace.
     */
    if (optd->span->backtrace) {
        for (i=0; i<optd->span->backtrace_size; i++) {
            if (optd->span->backtrace[i] == depspan)
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("circular reference detected"));
        }
    }

    /* Add our complete backtrace and ourselves to backtrace of dependent
     * span.
     */
    if (!depspan->backtrace) {
        depspan->backtrace = yasm_xmalloc((optd->span->backtrace_size+1)*
                                          sizeof(yasm_span *));
        if (optd->span->backtrace_size > 0)
            memcpy(depspan->backtrace, optd->span->backtrace,
                   optd->span->backtrace_size*sizeof(yasm_span *));
        depspan->backtrace[optd->span->backtrace_size] = optd->span;
        depspan->backtrace_size = optd->span->backtrace_size+1;
        return;
    }

    /* Add our complete backtrace, checking for duplicates */
    depspan_bt_alloc = depspan->backtrace_size;
    for (i=0; i<optd->span->backtrace_size; i++) {
        int present = 0;
        int j;
        for (j=0; j<depspan->backtrace_size; j++) {
            if (optd->span->backtrace[i] == optd->span->backtrace[j]) {
                present = 1;
                break;
            }
        }
        if (present)
            continue;
        /* Not already in array; add it. */
        if (depspan->backtrace_size >= depspan_bt_alloc)
        {
            depspan_bt_alloc *= 2;
            depspan->backtrace =
                yasm_xrealloc(depspan->backtrace,
                              depspan_bt_alloc*sizeof(yasm_span *));
        }
        depspan->backtrace[depspan->backtrace_size] = optd->span->backtrace[i];
        depspan->backtrace_size++;
    }

    /* Add ourselves. */
    if (depspan->backtrace_size >= depspan_bt_alloc)
    {
        depspan_bt_alloc++;
        depspan->backtrace =
            yasm_xrealloc(depspan->backtrace,
                          depspan_bt_alloc*sizeof(yasm_span *));
    }
    depspan->backtrace[depspan->backtrace_size] = optd->span;
    depspan->backtrace_size++;
}

static void
optimize_term_expand(IntervalTreeNode *node, void *d)
{
    optimize_data *optd = d;
    yasm_span_term *term = node->data;
    yasm_span *span = term->span;
    long len_diff = optd->len_diff;
    long precbc_index, precbc2_index;

    /* Don't expand inactive spans */
    if (!span->active)
        return;

    /* Update term length */
    if (term->precbc)
        precbc_index = term->precbc->bc_index;
    else
        precbc_index = span->bc->bc_index-1;

    if (term->precbc2)
        precbc2_index = term->precbc2->bc_index;
    else
        precbc2_index = span->bc->bc_index-1;

    if (precbc_index < precbc2_index)
        term->new_val += len_diff;
    else
        term->new_val -= len_diff;

    /* If already on Q, don't re-add */
    if (span->active == 2)
        return;

    /* Update term and check against thresholds */
    if (!recalc_normal_span(span))
        return; /* didn't exceed thresholds, we're done */

    /* Exceeded thresholds, need to add to Q for expansion */
    if (span->id <= 0)
        STAILQ_INSERT_TAIL(&optd->QA, span, linkq);
    else
        STAILQ_INSERT_TAIL(&optd->QB, span, linkq);
    span->active = 2;       /* Mark as being in Q */
}

void
yasm_object_optimize(yasm_object *object, yasm_errwarns *errwarns)
{
    yasm_section *sect;
    unsigned long bc_index = 0;
    int saw_error = 0;
    optimize_data optd;
    yasm_span *span, *span_temp;
    yasm_offset_setter *os;
    int retval;
    unsigned int i;

    TAILQ_INIT(&optd.spans);
    STAILQ_INIT(&optd.offset_setters);
    optd.itree = IT_create();

    /* Create an placeholder offset setter for spans to point to; this will
     * get updated if/when we actually run into one.
     */
    os = yasm_xmalloc(sizeof(yasm_offset_setter));
    os->bc = NULL;
    os->cur_val = 0;
    os->new_val = 0;
    os->thres = 0;
    STAILQ_INSERT_TAIL(&optd.offset_setters, os, link);
    optd.os = os;

    /* Step 1a */
    STAILQ_FOREACH(sect, &object->sections, link) {
        unsigned long offset = 0;

        yasm_bytecode *bc = STAILQ_FIRST(&sect->bcs);
        yasm_bytecode *prevbc;

        bc->bc_index = bc_index++;

        /* Skip our locally created empty bytecode first. */
        prevbc = bc;
        bc = STAILQ_NEXT(bc, link);

        /* Iterate through the remainder, if any. */
        while (bc) {
            bc->bc_index = bc_index++;
            bc->offset = offset;

            retval = yasm_bc_calc_len(bc, optimize_add_span, &optd);
            yasm_errwarn_propagate(errwarns, bc->line);
            if (retval)
                saw_error = 1;
            else {
                if (bc->callback->special == YASM_BC_SPECIAL_OFFSET) {
                    /* Remember it as offset setter */
                    os->bc = bc;
                    os->thres = yasm_bc_next_offset(bc);

                    /* Create new placeholder */
                    os = yasm_xmalloc(sizeof(yasm_offset_setter));
                    os->bc = NULL;
                    os->cur_val = 0;
                    os->new_val = 0;
                    os->thres = 0;
                    STAILQ_INSERT_TAIL(&optd.offset_setters, os, link);
                    optd.os = os;

                    if (bc->multiple) {
                        yasm_error_set(YASM_ERROR_VALUE,
                            N_("cannot combine multiples and setting assembly position"));
                        yasm_errwarn_propagate(errwarns, bc->line);
                        saw_error = 1;
                    }
                }

                offset += bc->len*bc->mult_int;
            }

            prevbc = bc;
            bc = STAILQ_NEXT(bc, link);
        }
    }

    if (saw_error) {
        optimize_cleanup(&optd);
        return;
    }

    /* Step 1b */
    TAILQ_FOREACH_SAFE(span, &optd.spans, link, span_temp) {
        span_create_terms(span);
        if (yasm_error_occurred()) {
            yasm_errwarn_propagate(errwarns, span->bc->line);
            saw_error = 1;
        } else if (recalc_normal_span(span)) {
            retval = yasm_bc_expand(span->bc, span->id, span->cur_val,
                                    span->new_val, &span->neg_thres,
                                    &span->pos_thres);
            yasm_errwarn_propagate(errwarns, span->bc->line);
            if (retval < 0)
                saw_error = 1;
            else if (retval > 0) {
                if (!span->active) {
                    yasm_error_set(YASM_ERROR_VALUE,
                        N_("secondary expansion of an external/complex value"));
                    yasm_errwarn_propagate(errwarns, span->bc->line);
                    saw_error = 1;
                }
            } else {
                TAILQ_REMOVE(&optd.spans, span, link);
                span_destroy(span);
                continue;
            }
        }
        span->cur_val = span->new_val;
    }

    if (saw_error) {
        optimize_cleanup(&optd);
        return;
    }

    /* Step 1c */
    if (update_all_bc_offsets(object, errwarns)) {
        optimize_cleanup(&optd);
        return;
    }

    /* Step 1d */
    STAILQ_INIT(&optd.QB);
    TAILQ_FOREACH(span, &optd.spans, link) {
        yasm_intnum *intn;

        /* Update span terms based on new bc offsets */
        for (i=0; i<span->num_terms; i++) {
            intn = yasm_calc_bc_dist(span->terms[i].precbc,
                                     span->terms[i].precbc2);
            if (!intn)
                yasm_internal_error(N_("could not calculate bc distance"));
            span->terms[i].cur_val = span->terms[i].new_val;
            span->terms[i].new_val = yasm_intnum_get_int(intn);
            yasm_intnum_destroy(intn);
        }
        if (span->rel_term) {
            span->rel_term->cur_val = span->rel_term->new_val;
            if (span->rel_term->precbc2)
                span->rel_term->new_val =
                    yasm_bc_next_offset(span->rel_term->precbc2) -
                    span->bc->offset;
            else
                span->rel_term->new_val = span->bc->offset -
                    yasm_bc_next_offset(span->rel_term->precbc);
        }

        if (recalc_normal_span(span)) {
            /* Exceeded threshold, add span to QB */
            STAILQ_INSERT_TAIL(&optd.QB, span, linkq);
            span->active = 2;
        }
    }

    /* Do we need step 2?  If not, go ahead and exit. */
    if (STAILQ_EMPTY(&optd.QB)) {
        optimize_cleanup(&optd);
        return;
    }

    /* Update offset-setters values */
    STAILQ_FOREACH(os, &optd.offset_setters, link) {
        if (!os->bc)
            continue;
        os->thres = yasm_bc_next_offset(os->bc);
        os->new_val = os->bc->offset;
        os->cur_val = os->new_val;
    }

    /* Build up interval tree */
    TAILQ_FOREACH(span, &optd.spans, link) {
        for (i=0; i<span->num_terms; i++)
            optimize_itree_add(optd.itree, span, &span->terms[i]);
        if (span->rel_term)
            optimize_itree_add(optd.itree, span, span->rel_term);
    }

    /* Look for cycles in times expansion (span.id==0) */
    TAILQ_FOREACH(span, &optd.spans, link) {
        if (span->id > 0)
            continue;
        optd.span = span;
        IT_enumerate(optd.itree, (long)span->bc->bc_index,
                     (long)span->bc->bc_index, &optd, check_cycle);
        if (yasm_error_occurred()) {
            yasm_errwarn_propagate(errwarns, span->bc->line);
            saw_error = 1;
        }
    }

    if (saw_error) {
        optimize_cleanup(&optd);
        return;
    }

    /* Step 2 */
    STAILQ_INIT(&optd.QA);
    while (!STAILQ_EMPTY(&optd.QA) || !(STAILQ_EMPTY(&optd.QB))) {
        unsigned long orig_len;
        long offset_diff;

        /* QA is for TIMES, update those first, then update non-TIMES.
         * This is so that TIMES can absorb increases before we look at
         * expanding non-TIMES BCs.
         */
        if (!STAILQ_EMPTY(&optd.QA)) {
            span = STAILQ_FIRST(&optd.QA);
            STAILQ_REMOVE_HEAD(&optd.QA, linkq);
        } else {
            span = STAILQ_FIRST(&optd.QB);
            STAILQ_REMOVE_HEAD(&optd.QB, linkq);
        }

        if (!span->active)
            continue;
        span->active = 1;   /* no longer in Q */

        /* Make sure we ended up ultimately exceeding thresholds; due to
         * offset BCs we may have been placed on Q and then reduced in size
         * again.
         */
        if (!recalc_normal_span(span))
            continue;

        orig_len = span->bc->len * span->bc->mult_int;

        retval = yasm_bc_expand(span->bc, span->id, span->cur_val,
                                span->new_val, &span->neg_thres,
                                &span->pos_thres);
        yasm_errwarn_propagate(errwarns, span->bc->line);

        if (retval < 0) {
            /* error */
            saw_error = 1;
            continue;
        } else if (retval > 0) {
            /* another threshold, keep active */
            for (i=0; i<span->num_terms; i++)
                span->terms[i].cur_val = span->terms[i].new_val;
            if (span->rel_term)
                span->rel_term->cur_val = span->rel_term->new_val;
            span->cur_val = span->new_val;
        } else
            span->active = 0;       /* we're done with this span */

        optd.len_diff = span->bc->len * span->bc->mult_int - orig_len;
        if (optd.len_diff == 0)
            continue;   /* didn't increase in size */

        /* Iterate over all spans dependent across the bc just expanded */
        IT_enumerate(optd.itree, (long)span->bc->bc_index,
                     (long)span->bc->bc_index, &optd, optimize_term_expand);

        /* Iterate over offset-setters that follow the bc just expanded.
         * Stop iteration if:
         *  - no more offset-setters in this section
         *  - offset-setter didn't move its following offset
         */
        os = span->os;
        offset_diff = optd.len_diff;
        while (os->bc && os->bc->section == span->bc->section
               && offset_diff != 0) {
            unsigned long old_next_offset = os->cur_val + os->bc->len;
            long neg_thres_temp;

            if (offset_diff < 0 && (unsigned long)(-offset_diff) > os->new_val)
                yasm_internal_error(N_("org/align went to negative offset"));
            os->new_val += offset_diff;

            orig_len = os->bc->len;
            retval = yasm_bc_expand(os->bc, 1, (long)os->cur_val,
                                    (long)os->new_val, &neg_thres_temp,
                                    (long *)&os->thres);
            yasm_errwarn_propagate(errwarns, os->bc->line);

            offset_diff = os->new_val + os->bc->len - old_next_offset;
            optd.len_diff = os->bc->len - orig_len;
            if (optd.len_diff != 0)
                IT_enumerate(optd.itree, (long)os->bc->bc_index,
                     (long)os->bc->bc_index, &optd, optimize_term_expand);

            os->cur_val = os->new_val;
            os = STAILQ_NEXT(os, link);
        }
    }

    if (saw_error) {
        optimize_cleanup(&optd);
        return;
    }

    /* Step 3 */
    update_all_bc_offsets(object, errwarns);
    optimize_cleanup(&optd);
}
