/*
 * Flat-format binary object format
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
#include <util.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <libyasm.h>


#define REGULAR_OUTBUF_SIZE     1024

typedef struct bin_section_data {
    int bss;                    /* aka nobits */

    /* User-provided alignment */
    yasm_intnum *align, *valign;

    /* User-provided starts */
    /*@null@*/ /*@owned@*/ yasm_expr *start, *vstart;

    /* User-provided follows */
    /*@null@*/ /*@owned@*/ char *follows, *vfollows;

    /* Calculated (final) starts, used only during output() */
    /*@null@*/ /*@owned@*/ yasm_intnum *istart, *ivstart;

    /* Calculated (final) length, used only during output() */
    /*@null@*/ /*@owned@*/ yasm_intnum *length;
} bin_section_data;

typedef struct yasm_objfmt_bin {
    yasm_objfmt_base objfmt;            /* base structure */

    enum {
        NO_MAP = 0,
        MAP_NONE = 0x01,
        MAP_BRIEF = 0x02,
        MAP_SECTIONS = 0x04,
        MAP_SYMBOLS = 0x08
    } map_flags;
    /*@null@*/ /*@only@*/ char *map_filename;

    /*@null@*/ /*@only@*/ yasm_expr *org;
} yasm_objfmt_bin;

/* symrec data is used only for the special symbols section<sectname>.start,
 * section<sectname>.vstart, and section<sectname>.length
 */
typedef struct bin_symrec_data {
    yasm_section *section;          /* referenced section */
    enum bin_ssym {
        SSYM_START,
        SSYM_VSTART,
        SSYM_LENGTH
    } which;
} bin_symrec_data;

static void bin_section_data_destroy(/*@only@*/ void *d);
static void bin_section_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback bin_section_data_cb = {
    bin_section_data_destroy,
    bin_section_data_print
};

static void bin_symrec_data_destroy(/*@only@*/ void *d);
static void bin_symrec_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback bin_symrec_data_cb = {
    bin_symrec_data_destroy,
    bin_symrec_data_print
};

yasm_objfmt_module yasm_bin_LTX_objfmt;


static yasm_objfmt *
bin_objfmt_create(yasm_object *object)
{
    yasm_objfmt_bin *objfmt_bin = yasm_xmalloc(sizeof(yasm_objfmt_bin));
    objfmt_bin->objfmt.module = &yasm_bin_LTX_objfmt;

    objfmt_bin->map_flags = NO_MAP;
    objfmt_bin->map_filename = NULL;
    objfmt_bin->org = NULL;

    return (yasm_objfmt *)objfmt_bin;
}

typedef TAILQ_HEAD(bin_group_head, bin_group) bin_groups;

typedef struct bin_group {
    TAILQ_ENTRY(bin_group) link;
    yasm_section *section;
    bin_section_data *bsd;

    /* Groups that (in parallel) logically come immediately after this
     * group's section.
     */
    bin_groups follow_groups;
} bin_group;

/* Recursive function to find group containing named section. */
static bin_group *
find_group_by_name(bin_groups *groups, const char *name)
{
    bin_group *group, *found;
    TAILQ_FOREACH(group, groups, link) {
        if (strcmp(yasm_section_get_name(group->section), name) == 0)
            return group;
        /* Recurse to loop through follow groups */
        found = find_group_by_name(&group->follow_groups, name);
        if (found)
            return found;
    }
    return NULL;
}

/* Recursive function to find group.  Returns NULL if not found. */
static bin_group *
find_group_by_section(bin_groups *groups, yasm_section *section)
{
    bin_group *group, *found;
    TAILQ_FOREACH(group, groups, link) {
        if (group->section == section)
            return group;
        /* Recurse to loop through follow groups */
        found = find_group_by_section(&group->follow_groups, section);
        if (found)
            return found;
    }
    return NULL;
}

#if 0
/* Debugging function */
static void
print_groups(const bin_groups *groups, int indent_level)
{
    bin_group *group;
    TAILQ_FOREACH(group, groups, link) {
        printf("%*sSection `%s':\n", indent_level, "",
               yasm_section_get_name(group->section));
        bin_section_data_print(group->bsd, stdout, indent_level+1);
        if (!TAILQ_EMPTY(&group->follow_groups)) {
            printf("%*sFollowing groups:\n", indent_level, "");
            print_groups(&group->follow_groups, indent_level+1);
        }
    }
}
#endif

static void
bin_group_destroy(/*@only@*/ bin_group *group)
{
    bin_group *follow, *group_temp;
    TAILQ_FOREACH_SAFE(follow, &group->follow_groups, link, group_temp)
        bin_group_destroy(follow);
    yasm_xfree(group);
}

typedef struct bin_objfmt_output_info {
    yasm_object *object;
    yasm_errwarns *errwarns;
    /*@dependent@*/ FILE *f;
    /*@only@*/ unsigned char *buf;
    /*@observer@*/ const yasm_section *sect;
    unsigned long start;        /* what normal variables go against */

    yasm_intnum *origin;
    yasm_intnum *tmp_intn;      /* temporary working intnum */

    bin_groups lma_groups, vma_groups;
} bin_objfmt_output_info;

static int
bin_objfmt_check_sym(yasm_symrec *sym, /*@null@*/ void *d)
{
    /*@null@*/ bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);
    assert(info != NULL);

    /* Don't check internally-generated symbols.  Only internally generated
     * symbols have symrec data, so simply check for its presence.
     */
    if (yasm_symrec_get_data(sym, &bin_symrec_data_cb))
        return 0;

    if (vis & YASM_SYM_EXTERN) {
        yasm_warn_set(YASM_WARN_GENERAL,
            N_("binary object format does not support extern variables"));
        yasm_errwarn_propagate(info->errwarns, yasm_symrec_get_decl_line(sym));
    } else if (vis & YASM_SYM_GLOBAL) {
        yasm_warn_set(YASM_WARN_GENERAL,
            N_("binary object format does not support global variables"));
        yasm_errwarn_propagate(info->errwarns, yasm_symrec_get_decl_line(sym));
    } else if (vis & YASM_SYM_COMMON) {
        yasm_error_set(YASM_ERROR_TYPE,
            N_("binary object format does not support common variables"));
        yasm_errwarn_propagate(info->errwarns, yasm_symrec_get_decl_line(sym));
    }
    return 0;
}

static int
bin_lma_create_group(yasm_section *sect, /*@null@*/ void *d)
{
    bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    bin_section_data *bsd = yasm_section_get_data(sect, &bin_section_data_cb);
    unsigned long align = yasm_section_get_align(sect);
    bin_group *group;

    assert(info != NULL);
    assert(bsd != NULL);

    group = yasm_xmalloc(sizeof(bin_group));
    group->section = sect;
    group->bsd = bsd;
    TAILQ_INIT(&group->follow_groups);

    /* Determine section alignment as necessary. */
    if (!bsd->align)
        bsd->align = yasm_intnum_create_uint(align > 4 ? align : 4);
    else {
        yasm_intnum *align_intn = yasm_intnum_create_uint(align);
        if (yasm_intnum_compare(align_intn, bsd->align) > 0) {
            yasm_warn_set(YASM_WARN_GENERAL,
                N_("section `%s' internal align of %lu is greater than `%s' of %lu; using `%s'"),
                yasm_section_get_name(sect),
                yasm_intnum_get_uint(align_intn),
                N_("align"),
                yasm_intnum_get_uint(bsd->align),
                N_("align"));
            yasm_errwarn_propagate(info->errwarns, 0);
        }
        yasm_intnum_destroy(align_intn);
    }

    /* Calculate section integer start. */
    if (bsd->start) {
        bsd->istart = yasm_expr_get_intnum(&bsd->start, 0);
        if (!bsd->istart) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("start expression is too complex"));
            yasm_errwarn_propagate(info->errwarns, bsd->start->line);
            return 1;
        } else
            bsd->istart = yasm_intnum_copy(bsd->istart);
    } else
        bsd->istart = NULL;

    /* Calculate section integer vstart. */
    if (bsd->vstart) {
        bsd->ivstart = yasm_expr_get_intnum(&bsd->vstart, 0);
        if (!bsd->ivstart) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("vstart expression is too complex"));
            yasm_errwarn_propagate(info->errwarns, bsd->vstart->line);
            return 1;
        } else
            bsd->ivstart = yasm_intnum_copy(bsd->ivstart);
    } else
        bsd->ivstart = NULL;

    /* Calculate section integer length. */
    bsd->length = yasm_calc_bc_dist(yasm_section_bcs_first(sect),
                                    yasm_section_bcs_last(sect));

    TAILQ_INSERT_TAIL(&info->lma_groups, group, link);
    return 0;
}

static int
bin_vma_create_group(yasm_section *sect, /*@null@*/ void *d)
{
    bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    bin_section_data *bsd = yasm_section_get_data(sect, &bin_section_data_cb);
    bin_group *group;

    assert(info != NULL);
    assert(bsd != NULL);

    group = yasm_xmalloc(sizeof(bin_group));
    group->section = sect;
    group->bsd = bsd;
    TAILQ_INIT(&group->follow_groups);

    TAILQ_INSERT_TAIL(&info->vma_groups, group, link);
    return 0;
}

/* Calculates new start address based on alignment constraint.
 * Start is modified (rounded up) to the closest aligned value greater than
 * what was passed in.
 * Align must be a power of 2.
 */
static void
bin_objfmt_align(yasm_intnum *start, const yasm_intnum *align)
{
    /* Because alignment is always a power of two, we can use some bit
     * trickery to do this easily.
     */
    yasm_intnum *align_intn =
        yasm_intnum_create_uint(yasm_intnum_get_uint(align)-1);
    yasm_intnum_calc(align_intn, YASM_EXPR_AND, start);
    if (!yasm_intnum_is_zero(align_intn)) {
        /* start = (start & ~(align-1)) + align; */
        yasm_intnum_set_uint(align_intn, yasm_intnum_get_uint(align)-1);
        yasm_intnum_calc(align_intn, YASM_EXPR_NOT, NULL);
        yasm_intnum_calc(align_intn, YASM_EXPR_AND, start);
        yasm_intnum_set(start, align);
        yasm_intnum_calc(start, YASM_EXPR_ADD, align_intn);
    }
    yasm_intnum_destroy(align_intn);
}

/* Recursive function to assign start addresses.
 * Updates start, last, and vdelta parameters as it goes along.
 * The tmp parameter is just a working intnum so one doesn't have to be
 * locally allocated for this purpose.
 */
static void
group_assign_start_recurse(bin_group *group, yasm_intnum *start,
                           yasm_intnum *last, yasm_intnum *vdelta,
                           yasm_intnum *tmp, yasm_errwarns *errwarns)
{
    bin_group *follow_group;

    /* Determine LMA */
    if (group->bsd->istart) {
        yasm_intnum_set(group->bsd->istart, start);
        if (group->bsd->align) {
            bin_objfmt_align(group->bsd->istart, group->bsd->align);
            if (yasm_intnum_compare(start, group->bsd->istart) != 0) {
                yasm_warn_set(YASM_WARN_GENERAL,
                    N_("start inconsistent with align; using aligned value"));
                yasm_errwarn_propagate(errwarns, group->bsd->start->line);
            }
        }
    } else {
        group->bsd->istart = yasm_intnum_copy(start);
        if (group->bsd->align != 0)
            bin_objfmt_align(group->bsd->istart, group->bsd->align);
    }

    /* Determine VMA if either just valign specified or if no v* specified */
    if (!group->bsd->vstart) {
        if (!group->bsd->vfollows && !group->bsd->valign) {
            /* No v* specified, set VMA=LMA+vdelta. */
            group->bsd->ivstart = yasm_intnum_copy(group->bsd->istart);
            yasm_intnum_calc(group->bsd->ivstart, YASM_EXPR_ADD, vdelta);
        } else if (!group->bsd->vfollows) {
            /* Just valign specified: set VMA=LMA+vdelta, align VMA, then add
             * delta between unaligned and aligned to vdelta parameter.
             */
            group->bsd->ivstart = yasm_intnum_copy(group->bsd->istart);
            yasm_intnum_calc(group->bsd->ivstart, YASM_EXPR_ADD, vdelta);
            yasm_intnum_set(tmp, group->bsd->ivstart);
            bin_objfmt_align(group->bsd->ivstart, group->bsd->valign);
            yasm_intnum_calc(vdelta, YASM_EXPR_ADD, group->bsd->ivstart);
            yasm_intnum_calc(vdelta, YASM_EXPR_SUB, tmp);
        }
    }

    /* Find the maximum end value */
    yasm_intnum_set(tmp, group->bsd->istart);
    yasm_intnum_calc(tmp, YASM_EXPR_ADD, group->bsd->length);
    if (yasm_intnum_compare(tmp, last) > 0)     /* tmp > last */
        yasm_intnum_set(last, tmp);

    /* Recurse for each following group. */
    TAILQ_FOREACH(follow_group, &group->follow_groups, link) {
        /* Following sections have to follow this one,
         * so add length to start.
         */
        yasm_intnum_set(start, group->bsd->istart);
        yasm_intnum_calc(start, YASM_EXPR_ADD, group->bsd->length);

        group_assign_start_recurse(follow_group, start, last, vdelta, tmp,
                                   errwarns);
    }
}

/* Recursive function to assign start addresses.
 * Updates start parameter as it goes along.
 * The tmp parameter is just a working intnum so one doesn't have to be
 * locally allocated for this purpose.
 */
static void
group_assign_vstart_recurse(bin_group *group, yasm_intnum *start,
                            yasm_errwarns *errwarns)
{
    bin_group *follow_group;

    /* Determine VMA section alignment as necessary.
     * Default to LMA alignment if not specified.
     */
    if (!group->bsd->valign)
        group->bsd->valign = yasm_intnum_copy(group->bsd->align);
    else {
        unsigned long align = yasm_section_get_align(group->section);
        yasm_intnum *align_intn = yasm_intnum_create_uint(align);
        if (yasm_intnum_compare(align_intn, group->bsd->valign) > 0) {
            yasm_warn_set(YASM_WARN_GENERAL,
                N_("section `%s' internal align of %lu is greater than `%s' of %lu; using `%s'"),
                yasm_section_get_name(group->section),
                yasm_intnum_get_uint(align_intn),
                N_("valign"),
                yasm_intnum_get_uint(group->bsd->valign),
                N_("valign"));
            yasm_errwarn_propagate(errwarns, 0);
        }
        yasm_intnum_destroy(align_intn);
    }

    /* Determine VMA */
    if (group->bsd->ivstart) {
        yasm_intnum_set(group->bsd->ivstart, start);
        if (group->bsd->valign) {
            bin_objfmt_align(group->bsd->ivstart, group->bsd->valign);
            if (yasm_intnum_compare(start, group->bsd->ivstart) != 0) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("vstart inconsistent with valign"));
                yasm_errwarn_propagate(errwarns, group->bsd->vstart->line);
            }
        }
    } else {
        group->bsd->ivstart = yasm_intnum_copy(start);
        if (group->bsd->valign)
            bin_objfmt_align(group->bsd->ivstart, group->bsd->valign);
    }

    /* Recurse for each following group. */
    TAILQ_FOREACH(follow_group, &group->follow_groups, link) {
        /* Following sections have to follow this one,
         * so add length to start.
         */
        yasm_intnum_set(start, group->bsd->ivstart);
        yasm_intnum_calc(start, YASM_EXPR_ADD, group->bsd->length);

        group_assign_vstart_recurse(follow_group, start, errwarns);
    }
}

static /*@null@*/ const yasm_intnum *
get_ssym_value(yasm_symrec *sym)
{
    bin_symrec_data *bsymd = yasm_symrec_get_data(sym, &bin_symrec_data_cb);
    bin_section_data *bsd;

    if (!bsymd)
        return NULL;

    bsd = yasm_section_get_data(bsymd->section, &bin_section_data_cb);
    assert(bsd != NULL);

    switch (bsymd->which) {
        case SSYM_START: return bsd->istart;
        case SSYM_VSTART: return bsd->ivstart;
        case SSYM_LENGTH: return bsd->length;
    }
    return NULL;
}

static /*@only@*/ yasm_expr *
bin_objfmt_expr_xform(/*@returned@*/ /*@only@*/ yasm_expr *e,
                      /*@unused@*/ /*@null@*/ void *d)
{
    int i;
    for (i=0; i<e->numterms; i++) {
        /*@dependent@*/ yasm_section *sect;
        /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
        /*@null@*/ yasm_intnum *dist;
        /*@null@*/ const yasm_intnum *ssymval;

        /* Transform symrecs or precbcs that reference sections into
         * vstart + intnum(dist).
         */
        if (((e->terms[i].type == YASM_EXPR_SYM &&
             yasm_symrec_get_label(e->terms[i].data.sym, &precbc)) ||
            (e->terms[i].type == YASM_EXPR_PRECBC &&
             (precbc = e->terms[i].data.precbc))) &&
            (sect = yasm_bc_get_section(precbc)) &&
            (dist = yasm_calc_bc_dist(yasm_section_bcs_first(sect), precbc))) {
            bin_section_data *bsd;
            bsd = yasm_section_get_data(sect, &bin_section_data_cb);
            assert(bsd != NULL);
            yasm_intnum_calc(dist, YASM_EXPR_ADD, bsd->ivstart);
            e->terms[i].type = YASM_EXPR_INT;
            e->terms[i].data.intn = dist;
        }

        /* Transform our special symrecs into the appropriate value */
        if (e->terms[i].type == YASM_EXPR_SYM &&
            (ssymval = get_ssym_value(e->terms[i].data.sym))) {
            e->terms[i].type = YASM_EXPR_INT;
            e->terms[i].data.intn = yasm_intnum_copy(ssymval);
        }
    }

    return e;
}

typedef struct map_output_info {
    /* address width */
    int bytes;

    /* intnum output static data areas */
    unsigned char *buf;
    yasm_intnum *intn;

    /* symrec output information */
    unsigned long count;
    yasm_section *section;  /* NULL for EQUs */

    yasm_object *object;    /* object */
    FILE *f;                /* map output file */
} map_output_info;

static int
map_prescan_bytes(yasm_section *sect, void *d)
{
    bin_section_data *bsd = yasm_section_get_data(sect, &bin_section_data_cb);
    map_output_info *info = (map_output_info *)d;

    assert(bsd != NULL);
    assert(info != NULL);

    while (!yasm_intnum_check_size(bsd->length, info->bytes * 8, 0, 0))
        info->bytes *= 2;
    while (!yasm_intnum_check_size(bsd->istart, info->bytes * 8, 0, 0))
        info->bytes *= 2;
    while (!yasm_intnum_check_size(bsd->ivstart, info->bytes * 8, 0, 0))
        info->bytes *= 2;

    return 0;
}

static void
map_print_intnum(const yasm_intnum *intn, map_output_info *info)
{
    size_t i;
    yasm_intnum_get_sized(intn, info->buf, info->bytes, info->bytes*8, 0, 0,
                          0);
    for (i=info->bytes; i != 0; i--)
        fprintf(info->f, "%02X", info->buf[i-1]);
}

static void
map_sections_summary(bin_groups *groups, map_output_info *info)
{
    bin_group *group;
    TAILQ_FOREACH(group, groups, link) {
        bin_section_data *bsd = group->bsd;

        assert(bsd != NULL);
        assert(info != NULL);

        map_print_intnum(bsd->ivstart, info);
        fprintf(info->f, "  ");

        yasm_intnum_set(info->intn, bsd->ivstart);
        yasm_intnum_calc(info->intn, YASM_EXPR_ADD, bsd->length);
        map_print_intnum(info->intn, info);
        fprintf(info->f, "  ");

        map_print_intnum(bsd->istart, info);
        fprintf(info->f, "  ");

        yasm_intnum_set(info->intn, bsd->istart);
        yasm_intnum_calc(info->intn, YASM_EXPR_ADD, bsd->length);
        map_print_intnum(info->intn, info);
        fprintf(info->f, "  ");

        map_print_intnum(bsd->length, info);
        fprintf(info->f, "  ");

        fprintf(info->f, "%-*s", 10, bsd->bss ? "nobits" : "progbits");
        fprintf(info->f, "%s\n", yasm_section_get_name(group->section));

        /* Recurse to loop through follow groups */
        map_sections_summary(&group->follow_groups, info);
    }
}

static void
map_sections_detail(bin_groups *groups, map_output_info *info)
{
    bin_group *group;
    TAILQ_FOREACH(group, groups, link) {
        bin_section_data *bsd = group->bsd;
        size_t i;
        const char *s;

        s = yasm_section_get_name(group->section);
        fprintf(info->f, "---- Section %s ", s);
        for (i=0; i<(65-strlen(s)); i++)
            fputc('-', info->f);

        fprintf(info->f, "\n\nclass:     %s",
                bsd->bss ? "nobits" : "progbits");
        fprintf(info->f, "\nlength:    ");
        map_print_intnum(bsd->length, info);
        fprintf(info->f, "\nstart:     ");
        map_print_intnum(bsd->istart, info);
        fprintf(info->f, "\nalign:     ");
        map_print_intnum(bsd->align, info);
        fprintf(info->f, "\nfollows:   %s",
                bsd->follows ? bsd->follows : "not defined");
        fprintf(info->f, "\nvstart:    ");
        map_print_intnum(bsd->ivstart, info);
        fprintf(info->f, "\nvalign:    ");
        map_print_intnum(bsd->valign, info);
        fprintf(info->f, "\nvfollows:  %s\n\n",
                bsd->vfollows ? bsd->vfollows : "not defined");

        /* Recurse to loop through follow groups */
        map_sections_detail(&group->follow_groups, info);
    }
}

static int
map_symrec_count(yasm_symrec *sym, void *d)
{
    map_output_info *info = (map_output_info *)d;
    /*@dependent@*/ yasm_bytecode *precbc;

    assert(info != NULL);

    /* TODO: autodetect wider size */
    if (!info->section && yasm_symrec_get_equ(sym)) {
        info->count++;
    } else if (yasm_symrec_get_label(sym, &precbc) &&
               yasm_bc_get_section(precbc) == info->section) {
        info->count++;
    }
    return 0;
}

static int
map_symrec_output(yasm_symrec *sym, void *d)
{
    map_output_info *info = (map_output_info *)d;
    const yasm_expr *equ;
    /*@dependent@*/ yasm_bytecode *precbc;
    /*@only@*/ char *name = yasm_symrec_get_global_name(sym, info->object);

    assert(info != NULL);

    if (!info->section && (equ = yasm_symrec_get_equ(sym))) {
        yasm_expr *realequ = yasm_expr_copy(equ);
        realequ = yasm_expr__level_tree
            (realequ, 1, 1, 1, 0, bin_objfmt_expr_xform, NULL);
        yasm_intnum_set(info->intn, yasm_expr_get_intnum(&realequ, 0));
        yasm_expr_destroy(realequ);
        map_print_intnum(info->intn, info);
        fprintf(info->f, "  %s\n", name);
    } else if (yasm_symrec_get_label(sym, &precbc) &&
               yasm_bc_get_section(precbc) == info->section) {
        bin_section_data *bsd =
            yasm_section_get_data(info->section, &bin_section_data_cb);

        /* Real address */
        yasm_intnum_set_uint(info->intn, yasm_bc_next_offset(precbc));
        yasm_intnum_calc(info->intn, YASM_EXPR_ADD, bsd->istart);
        map_print_intnum(info->intn, info);
        fprintf(info->f, "  ");

        /* Virtual address */
        yasm_intnum_set_uint(info->intn, yasm_bc_next_offset(precbc));
        yasm_intnum_calc(info->intn, YASM_EXPR_ADD, bsd->ivstart);
        map_print_intnum(info->intn, info);

        /* Name */
        fprintf(info->f, "  %s\n", name);
    }
    yasm_xfree(name);
    return 0;
}

static void
map_sections_symbols(bin_groups *groups, map_output_info *info)
{
    bin_group *group;
    TAILQ_FOREACH(group, groups, link) {
        info->count = 0;
        info->section = group->section;
        yasm_symtab_traverse(info->object->symtab, info, map_symrec_count);

        if (info->count > 0) {
            const char *s = yasm_section_get_name(group->section);
            size_t i;
            fprintf(info->f, "---- Section %s ", s);
            for (i=0; i<(65-strlen(s)); i++)
                fputc('-', info->f);
            fprintf(info->f, "\n\n%-*s%-*s%s\n",
                    info->bytes*2+2, "Real",
                    info->bytes*2+2, "Virtual",
                    "Name");
            yasm_symtab_traverse(info->object->symtab, info,
                                 map_symrec_output);
            fprintf(info->f, "\n\n");
        }

        /* Recurse to loop through follow groups */
        map_sections_symbols(&group->follow_groups, info);
    }
}

static void
output_map(bin_objfmt_output_info *info)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)info->object->objfmt;
    FILE *f;
    int i;
    map_output_info mapinfo;

    if (objfmt_bin->map_flags == NO_MAP)
        return;

    if (objfmt_bin->map_flags == MAP_NONE)
        objfmt_bin->map_flags = MAP_BRIEF;          /* default to brief */

    if (!objfmt_bin->map_filename)
        f = stdout;                                 /* default to stdout */
    else {
        f = fopen(objfmt_bin->map_filename, "wt");
        if (!f) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("unable to open map file `%s'"),
                          objfmt_bin->map_filename);
            yasm_errwarn_propagate(info->errwarns, 0);
            return;
        }
    }

    mapinfo.object = info->object;
    mapinfo.f = f;

    /* Temporary intnum */
    mapinfo.intn = info->tmp_intn;

    /* Prescan all values to figure out what width we should make the output
     * fields.  Start with a minimum of 4.
     */
    mapinfo.bytes = 4;
    while (!yasm_intnum_check_size(info->origin, mapinfo.bytes * 8, 0, 0))
        mapinfo.bytes *= 2;
    yasm_object_sections_traverse(info->object, &mapinfo, map_prescan_bytes);
    mapinfo.buf = yasm_xmalloc(mapinfo.bytes);

    fprintf(f, "\n- YASM Map file ");
    for (i=0; i<63; i++)
        fputc('-', f);
    fprintf(f, "\n\nSource file:  %s\n", info->object->src_filename);
    fprintf(f, "Output file:  %s\n\n", info->object->obj_filename);

    fprintf(f, "-- Program origin ");
    for (i=0; i<61; i++)
        fputc('-', f);
    fprintf(f, "\n\n");
    map_print_intnum(info->origin, &mapinfo);
    fprintf(f, "\n\n");

    if (objfmt_bin->map_flags & MAP_BRIEF) {
        fprintf(f, "-- Sections (summary) ");
        for (i=0; i<57; i++)
            fputc('-', f);
        fprintf(f, "\n\n%-*s%-*s%-*s%-*s%-*s%-*s%s\n",
                mapinfo.bytes*2+2, "Vstart",
                mapinfo.bytes*2+2, "Vstop",
                mapinfo.bytes*2+2, "Start",
                mapinfo.bytes*2+2, "Stop",
                mapinfo.bytes*2+2, "Length",
                10, "Class", "Name");

        map_sections_summary(&info->lma_groups, &mapinfo);
        fprintf(f, "\n");
    }

    if (objfmt_bin->map_flags & MAP_SECTIONS) {
        fprintf(f, "-- Sections (detailed) ");
        for (i=0; i<56; i++)
            fputc('-', f);
        fprintf(f, "\n\n");
        map_sections_detail(&info->lma_groups, &mapinfo);
    }

    if (objfmt_bin->map_flags & MAP_SYMBOLS) {
        fprintf(f, "-- Symbols ");
        for (i=0; i<68; i++)
            fputc('-', f);
        fprintf(f, "\n\n");

        /* We do two passes for EQU and each section; the first pass
         * determines the byte width to use for the value and whether any
         * symbols are present, the second pass actually outputs the text.
         */

        /* EQUs */
        mapinfo.count = 0;
        mapinfo.section = NULL;
        yasm_symtab_traverse(info->object->symtab, &mapinfo, map_symrec_count);

        if (mapinfo.count > 0) {
            fprintf(f, "---- No Section ");
            for (i=0; i<63; i++)
                fputc('-', f);
            fprintf(f, "\n\n%-*s%s\n", mapinfo.bytes*2+2, "Value", "Name");
            yasm_symtab_traverse(info->object->symtab, &mapinfo,
                                 map_symrec_output);
            fprintf(f, "\n\n");
        }

        /* Other sections */
        map_sections_symbols(&info->lma_groups, &mapinfo);
    }

    if (f != stdout)
        fclose(f);

    yasm_xfree(mapinfo.buf);
}

/* Check for LMA overlap using a simple N^2 algorithm. */
static int
check_lma_overlap(yasm_section *sect, /*@null@*/ void *d)
{
    bin_section_data *bsd, *bsd2;
    yasm_section *other = (yasm_section *)d;
    yasm_intnum *overlap;

    if (!d)
        return yasm_object_sections_traverse(yasm_section_get_object(sect),
                                             sect, check_lma_overlap);
    if (sect == other)
        return 0;

    bsd = yasm_section_get_data(sect, &bin_section_data_cb);
    bsd2 = yasm_section_get_data(other, &bin_section_data_cb);

    if (yasm_intnum_is_zero(bsd->length) ||
        yasm_intnum_is_zero(bsd2->length))
        return 0;

    if (yasm_intnum_compare(bsd->istart, bsd2->istart) <= 0) {
        overlap = yasm_intnum_copy(bsd->istart);
        yasm_intnum_calc(overlap, YASM_EXPR_ADD, bsd->length);
        yasm_intnum_calc(overlap, YASM_EXPR_SUB, bsd2->istart);
    } else {
        overlap = yasm_intnum_copy(bsd2->istart);
        yasm_intnum_calc(overlap, YASM_EXPR_ADD, bsd2->length);
        yasm_intnum_calc(overlap, YASM_EXPR_SUB, bsd->istart);
    }

    if (yasm_intnum_sign(overlap) > 0) {
        yasm_error_set(YASM_ERROR_GENERAL,
                       N_("sections `%s' and `%s' overlap by %lu bytes"),
                       yasm_section_get_name(sect),
                       yasm_section_get_name(other),
                       yasm_intnum_get_uint(overlap));
        yasm_intnum_destroy(overlap);
        return -1;
    }

    yasm_intnum_destroy(overlap);
    return 0;
}

static int
bin_objfmt_output_value(yasm_value *value, unsigned char *buf,
                        unsigned int destsize,
                        /*@unused@*/ unsigned long offset, yasm_bytecode *bc,
                        int warn, /*@null@*/ void *d)
{
    /*@null@*/ bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
    /*@dependent@*/ yasm_section *sect;

    assert(info != NULL);

    /* Binary objects we need to resolve against object, not against section. */
    if (value->rel) {
        unsigned int rshift = (unsigned int)value->rshift;
        yasm_expr *syme;
        /*@null@*/ const yasm_intnum *ssymval;

        if (yasm_symrec_is_abs(value->rel)) {
            syme = yasm_expr_create_ident(yasm_expr_int(
                yasm_intnum_create_uint(0)), bc->line);
        } else if (yasm_symrec_get_label(value->rel, &precbc)
                   && (sect = yasm_bc_get_section(precbc))) {
            syme = yasm_expr_create_ident(yasm_expr_sym(value->rel), bc->line);
        } else if ((ssymval = get_ssym_value(value->rel))) {
            syme = yasm_expr_create_ident(yasm_expr_int(
                yasm_intnum_copy(ssymval)), bc->line);
        } else
            goto done;

        /* Handle PC-relative */
        if (value->curpos_rel) {
            yasm_expr *sube;
            sube = yasm_expr_create(YASM_EXPR_SUB, yasm_expr_precbc(bc),
                yasm_expr_int(yasm_intnum_create_uint(bc->len*bc->mult_int)),
                bc->line);
            syme = yasm_expr_create(YASM_EXPR_SUB, yasm_expr_expr(syme),
                                    yasm_expr_expr(sube), bc->line);
            value->curpos_rel = 0;
            value->ip_rel = 0;
        }

        if (value->rshift > 0)
            syme = yasm_expr_create(YASM_EXPR_SHR, yasm_expr_expr(syme),
                yasm_expr_int(yasm_intnum_create_uint(rshift)), bc->line);

        /* Add into absolute portion */
        if (!value->abs)
            value->abs = syme;
        else
            value->abs =
                yasm_expr_create(YASM_EXPR_ADD, yasm_expr_expr(value->abs),
                                 yasm_expr_expr(syme), bc->line);
        value->rel = NULL;
        value->rshift = 0;
    }
done:
    /* Simplify absolute portion of value, transforming symrecs */
    if (value->abs)
        value->abs = yasm_expr__level_tree
            (value->abs, 1, 1, 1, 0, bin_objfmt_expr_xform, NULL);

    /* Output */
    switch (yasm_value_output_basic(value, buf, destsize, bc, warn,
                                    info->object->arch)) {
        case -1:
            return 1;
        case 0:
            break;
        default:
            return 0;
    }

    /* Couldn't output, assume it contains an external reference. */
    yasm_error_set(YASM_ERROR_GENERAL,
        N_("binary object format does not support external references"));
    return 1;
}

static int
bin_objfmt_output_bytecode(yasm_bytecode *bc, /*@null@*/ void *d)
{
    /*@null@*/ bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    /*@null@*/ /*@only@*/ unsigned char *bigbuf;
    unsigned long size = REGULAR_OUTBUF_SIZE;
    int gap;

    assert(info != NULL);

    bigbuf = yasm_bc_tobytes(bc, info->buf, &size, &gap, info,
                             bin_objfmt_output_value, NULL);

    /* Don't bother doing anything else if size ended up being 0. */
    if (size == 0) {
        if (bigbuf)
            yasm_xfree(bigbuf);
        return 0;
    }

    /* Warn that gaps are converted to 0 and write out the 0's. */
    if (gap) {
        unsigned long left;
        yasm_warn_set(YASM_WARN_UNINIT_CONTENTS,
            N_("uninitialized space declared in code/data section: zeroing"));
        /* Write out in chunks */
        memset(info->buf, 0, REGULAR_OUTBUF_SIZE);
        left = size;
        while (left > REGULAR_OUTBUF_SIZE) {
            fwrite(info->buf, REGULAR_OUTBUF_SIZE, 1, info->f);
            left -= REGULAR_OUTBUF_SIZE;
        }
        fwrite(info->buf, left, 1, info->f);
    } else {
        /* Output buf (or bigbuf if non-NULL) to file */
        fwrite(bigbuf ? bigbuf : info->buf, (size_t)size, 1, info->f);
    }

    /* If bigbuf was allocated, free it */
    if (bigbuf)
        yasm_xfree(bigbuf);

    return 0;
}

/* Check to ensure bytecode is res* (for BSS sections) */
static int
bin_objfmt_no_output_bytecode(yasm_bytecode *bc, /*@null@*/ void *d)
{
    /*@null@*/ bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;
    /*@null@*/ /*@only@*/ unsigned char *bigbuf;
    unsigned long size = REGULAR_OUTBUF_SIZE;
    int gap;

    assert(info != NULL);

    bigbuf = yasm_bc_tobytes(bc, info->buf, &size, &gap, info,
                             bin_objfmt_output_value, NULL);

    /* If bigbuf was allocated, free it */
    if (bigbuf)
        yasm_xfree(bigbuf);

    /* Don't bother doing anything else if size ended up being 0. */
    if (size == 0)
        return 0;

    /* Warn if not a gap. */
    if (!gap) {
        yasm_warn_set(YASM_WARN_GENERAL,
            N_("initialized space declared in nobits section: ignoring"));
    }

    return 0;
}

static int
bin_objfmt_output_section(yasm_section *sect, /*@null@*/ void *d)
{
    bin_section_data *bsd = yasm_section_get_data(sect, &bin_section_data_cb);
    /*@null@*/ bin_objfmt_output_info *info = (bin_objfmt_output_info *)d;

    assert(bsd != NULL);
    assert(info != NULL);

    if (bsd->bss) {
        yasm_section_bcs_traverse(sect, info->errwarns,
                                  info, bin_objfmt_no_output_bytecode);
    } else {
        yasm_intnum_set(info->tmp_intn, bsd->istart);
        yasm_intnum_calc(info->tmp_intn, YASM_EXPR_SUB, info->origin);
        if (yasm_intnum_sign(info->tmp_intn) < 0) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("section `%s' starts before origin (ORG)"),
                           yasm_section_get_name(sect));
            yasm_errwarn_propagate(info->errwarns, 0);
            return 0;
        }
        if (!yasm_intnum_check_size(info->tmp_intn, sizeof(long)*8, 0, 1)) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("section `%s' start value too large"),
                           yasm_section_get_name(sect));
            yasm_errwarn_propagate(info->errwarns, 0);
            return 0;
        }
        if (fseek(info->f, yasm_intnum_get_int(info->tmp_intn) + info->start,
                  SEEK_SET) < 0)
            yasm__fatal(N_("could not seek on output file"));
        yasm_section_bcs_traverse(sect, info->errwarns,
                                  info, bin_objfmt_output_bytecode);
    }

    return 0;
}

static void
bin_objfmt_cleanup(bin_objfmt_output_info *info)
{
    bin_group *group, *group_temp;

    yasm_xfree(info->buf);
    yasm_intnum_destroy(info->origin);
    yasm_intnum_destroy(info->tmp_intn);

    TAILQ_FOREACH_SAFE(group, &info->lma_groups, link, group_temp)
        bin_group_destroy(group);

    TAILQ_FOREACH_SAFE(group, &info->vma_groups, link, group_temp)
        bin_group_destroy(group);
}

static void
bin_objfmt_output(yasm_object *object, FILE *f, /*@unused@*/ int all_syms,
                  yasm_errwarns *errwarns)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)object->objfmt;
    bin_objfmt_output_info info;
    bin_group *group, *lma_group, *vma_group, *group_temp;
    yasm_intnum *start, *last, *vdelta;
    bin_groups unsorted_groups, bss_groups;

    info.start = ftell(f);

    /* Set ORG to 0 unless otherwise specified */
    if (objfmt_bin->org) {
        info.origin = yasm_expr_get_intnum(&objfmt_bin->org, 0);
        if (!info.origin) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("ORG expression is too complex"));
            yasm_errwarn_propagate(errwarns, objfmt_bin->org->line);
            return;
        }
        if (yasm_intnum_sign(info.origin) < 0) {
            yasm_error_set(YASM_ERROR_VALUE, N_("ORG expression is negative"));
            yasm_errwarn_propagate(errwarns, objfmt_bin->org->line);
            return;
        }
        info.origin = yasm_intnum_copy(info.origin);
    } else
        info.origin = yasm_intnum_create_uint(0);

    info.object = object;
    info.errwarns = errwarns;
    info.f = f;
    info.buf = yasm_xmalloc(REGULAR_OUTBUF_SIZE);
    info.tmp_intn = yasm_intnum_create_uint(0);
    TAILQ_INIT(&info.lma_groups);
    TAILQ_INIT(&info.vma_groups);

    /* Check symbol table */
    yasm_symtab_traverse(object->symtab, &info, bin_objfmt_check_sym);

    /* Create section groups */
    if (yasm_object_sections_traverse(object, &info, bin_lma_create_group)) {
        bin_objfmt_cleanup(&info);
        return;     /* error detected */
    }

    /* Determine section order according to LMA.
     * Sections can be ordered either by (priority):
     *  - follows
     *  - start
     *  - progbits/nobits setting
     *  - order in the input file
     */

    /* Look at each group with follows specified, and find the section
     * that group is supposed to follow.
     */
    TAILQ_FOREACH_SAFE(lma_group, &info.lma_groups, link, group_temp) {
        if (lma_group->bsd->follows) {
            bin_group *found;
            /* Need to find group containing section this section follows. */
            found =
                find_group_by_name(&info.lma_groups, lma_group->bsd->follows);
            if (!found) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("section `%s' follows an invalid or unknown section `%s'"),
                               yasm_section_get_name(lma_group->section),
                               lma_group->bsd->follows);
                yasm_errwarn_propagate(errwarns, 0);
                bin_objfmt_cleanup(&info);
                return;
            }

            /* Check for loops */
            if (lma_group->section == found->section ||
                find_group_by_section(&lma_group->follow_groups,
                                      found->section)) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("follows loop between section `%s' and section `%s'"),
                               yasm_section_get_name(lma_group->section),
                               yasm_section_get_name(found->section));
                yasm_errwarn_propagate(errwarns, 0);
                bin_objfmt_cleanup(&info);
                return;
            }

            /* Remove this section from main lma groups list */
            TAILQ_REMOVE(&info.lma_groups, lma_group, link);
            /* Add it after the section it's supposed to follow. */
            TAILQ_INSERT_TAIL(&found->follow_groups, lma_group, link);
        }
    }

    /* Sort the top-level groups according to their start address.
     * Use Shell sort for ease of implementation.
     * If no start address is specified for a section, don't change the order,
     * and move BSS sections to a separate list so they can be moved to the
     * end of the lma list after all other sections are sorted.
     */
    unsorted_groups = info.lma_groups;  /* structure copy */
    TAILQ_INIT(&info.lma_groups);
    TAILQ_INIT(&bss_groups);
    TAILQ_FOREACH_SAFE(lma_group, &unsorted_groups, link, group_temp) {
        bin_group *before;

        if (!lma_group->bsd->istart) {
            if (lma_group->bsd->bss)
                TAILQ_INSERT_TAIL(&bss_groups, lma_group, link);
            else
                TAILQ_INSERT_TAIL(&info.lma_groups, lma_group, link);
            continue;
        }

        before = NULL;
        TAILQ_FOREACH(group, &info.lma_groups, link) {
            if (!group->bsd->istart)
                continue;
            if (yasm_intnum_compare(group->bsd->istart,
                                    lma_group->bsd->istart) > 0) {
                before = group;
                break;
            }
        }
        if (before)
            TAILQ_INSERT_BEFORE(before, lma_group, link);
        else
            TAILQ_INSERT_TAIL(&info.lma_groups, lma_group, link);
    }

    /* Move the pure-BSS sections to the end of the LMA list. */
    TAILQ_FOREACH_SAFE(group, &bss_groups, link, group_temp)
        TAILQ_INSERT_TAIL(&info.lma_groups, group, link);
    TAILQ_INIT(&bss_groups);    /* For sanity */

    /* Assign a LMA start address to every section.
     * Also assign VMA=LMA unless otherwise specified.
     *
     * We need to assign VMA=LMA here (while walking the tree) for the case:
     *  sect1 start=0 (size=0x11)
     *  sect2 follows=sect1 valign=16 (size=0x104)
     *  sect3 follows=sect2 valign=16
     * Where the valign of sect2 will result in a sect3 vaddr higher than a
     * naive segment-by-segment interpretation (where sect3 and sect2 would
     * have a VMA overlap).
     *
     * Algorithm for VMA=LMA setting:
     * Start with delta=0.
     * If there's no virtual attributes, we simply set VMA = LMA+delta.
     * If there's only valign specified, we set VMA = aligned LMA, and add
     * any new alignment difference to delta.
     *
     * We could do the LMA start and VMA=LMA steps in two separate steps,
     * but it's easier to just recurse once.
     */
    start = yasm_intnum_copy(info.origin);
    last = yasm_intnum_copy(info.origin);
    vdelta = yasm_intnum_create_uint(0);
    TAILQ_FOREACH(lma_group, &info.lma_groups, link) {
        if (lma_group->bsd->istart)
            yasm_intnum_set(start, lma_group->bsd->istart);
        group_assign_start_recurse(lma_group, start, last, vdelta,
                                   info.tmp_intn, errwarns);
        yasm_intnum_set(start, last);
    }
    yasm_intnum_destroy(last);
    yasm_intnum_destroy(vdelta);

    /*
     * Determine section order according to VMA
     */

    /* Create section groups */
    if (yasm_object_sections_traverse(object, &info, bin_vma_create_group)) {
        yasm_intnum_destroy(start);
        bin_objfmt_cleanup(&info);
        return;     /* error detected */
    }

    /* Look at each group with vfollows specified, and find the section
     * that group is supposed to follow.
     */
    TAILQ_FOREACH_SAFE(vma_group, &info.vma_groups, link, group_temp) {
        if (vma_group->bsd->vfollows) {
            bin_group *found;
            /* Need to find group containing section this section follows. */
            found = find_group_by_name(&info.vma_groups,
                                       vma_group->bsd->vfollows);
            if (!found) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("section `%s' vfollows an invalid or unknown section `%s'"),
                               yasm_section_get_name(vma_group->section),
                               vma_group->bsd->vfollows);
                yasm_errwarn_propagate(errwarns, 0);
                yasm_intnum_destroy(start);
                bin_objfmt_cleanup(&info);
                return;
            }

            /* Check for loops */
            if (vma_group->section == found->section ||
                find_group_by_section(&vma_group->follow_groups,
                                      found->section)) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("vfollows loop between section `%s' and section `%s'"),
                               yasm_section_get_name(vma_group->section),
                               yasm_section_get_name(found->section));
                yasm_errwarn_propagate(errwarns, 0);
                bin_objfmt_cleanup(&info);
                return;
            }

            /* Remove this section from main lma groups list */
            TAILQ_REMOVE(&info.vma_groups, vma_group, link);
            /* Add it after the section it's supposed to follow. */
            TAILQ_INSERT_TAIL(&found->follow_groups, vma_group, link);
        }
    }

    /* Due to the combination of steps above, we now know that all top-level
     * groups have integer ivstart:
     * Vstart Vfollows Valign   Handled by
     *     No       No     No   group_assign_start_recurse()
     *     No       No    Yes   group_assign_start_recurse()
     *     No      Yes    -     vfollows loop (above)
     *    Yes      -      -     bin_lma_create_group()
     */
    TAILQ_FOREACH(vma_group, &info.vma_groups, link) {
        yasm_intnum_set(start, vma_group->bsd->ivstart);
        group_assign_vstart_recurse(vma_group, start, errwarns);
    }

    /* Output map file */
    output_map(&info);

    /* Ensure we don't have overlapping progbits LMAs.
     * Use a dumb O(N^2) algorithm as the number of sections is essentially
     * always low.
     */
    if (yasm_object_sections_traverse(object, NULL, check_lma_overlap)) {
        yasm_errwarn_propagate(errwarns, 0);
        yasm_intnum_destroy(start);
        bin_objfmt_cleanup(&info);
        return;
    }

    /* Output sections */
    yasm_object_sections_traverse(object, &info, bin_objfmt_output_section);

    /* Clean up */
    yasm_intnum_destroy(start);
    bin_objfmt_cleanup(&info);
}

static void
bin_objfmt_destroy(yasm_objfmt *objfmt)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)objfmt;
    if (objfmt_bin->map_filename)
        yasm_xfree(objfmt_bin->map_filename);
    yasm_expr_destroy(objfmt_bin->org);
    yasm_xfree(objfmt);
}

static void
define_section_symbol(yasm_symtab *symtab, yasm_section *sect,
                      const char *sectname, const char *suffix,
                      enum bin_ssym which, unsigned long line)
{
    yasm_symrec *sym;
    bin_symrec_data *bsymd = yasm_xmalloc(sizeof(bin_symrec_data));
    char *symname = yasm_xmalloc(8+strlen(sectname)+strlen(suffix)+1);

    strcpy(symname, "section.");
    strcat(symname, sectname);
    strcat(symname, suffix);

    bsymd->section = sect;
    bsymd->which = which;

    sym = yasm_symtab_declare(symtab, symname, YASM_SYM_EXTERN, line);
    yasm_xfree(symname);
    yasm_symrec_add_data(sym, &bin_symrec_data_cb, bsymd);
}

static void
bin_objfmt_init_new_section(yasm_section *sect, unsigned long line)
{
    yasm_object *object = yasm_section_get_object(sect);
    const char *sectname = yasm_section_get_name(sect);
    /*yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)object->objfmt;*/
    bin_section_data *data;

    data = yasm_xmalloc(sizeof(bin_section_data));
    data->bss = 0;
    data->align = NULL;
    data->valign = NULL;
    data->start = NULL;
    data->vstart = NULL;
    data->follows = NULL;
    data->vfollows = NULL;
    data->istart = NULL;
    data->ivstart = NULL;
    data->length = NULL;
    yasm_section_add_data(sect, &bin_section_data_cb, data);

    define_section_symbol(object->symtab, sect, sectname, ".start",
                          SSYM_START, line);
    define_section_symbol(object->symtab, sect, sectname, ".vstart",
                          SSYM_VSTART, line);
    define_section_symbol(object->symtab, sect, sectname, ".length",
                          SSYM_LENGTH, line);
}

static yasm_section *
bin_objfmt_add_default_section(yasm_object *object)
{
    yasm_section *retval;
    int isnew;

    retval = yasm_object_get_general(object, ".text", 0, 1, 0, &isnew, 0);
    if (isnew)
        yasm_section_set_default(retval, 1);
    return retval;
}

/* GAS-style flags */
static int
bin_helper_gasflags(void *obj, yasm_valparam *vp, unsigned long line, void *d,
                    /*@unused@*/ uintptr_t arg)
{
    /* TODO */
    return 0;
}

static /*@observer@*/ /*@null@*/ yasm_section *
bin_objfmt_section_switch(yasm_object *object, yasm_valparamhead *valparams,
                          /*@unused@*/ /*@null@*/
                          yasm_valparamhead *objext_valparams,
                          unsigned long line)
{
    yasm_valparam *vp;
    yasm_section *retval;
    int isnew;
    int flags_override = 0;
    const char *sectname;
    bin_section_data *bsd = NULL;

    struct bin_section_switch_data {
        /*@only@*/ /*@null@*/ char *follows;
        /*@only@*/ /*@null@*/ char *vfollows;
        /*@only@*/ /*@null@*/ yasm_expr *start;
        /*@only@*/ /*@null@*/ yasm_expr *vstart;
        /*@only@*/ /*@null@*/ yasm_intnum *align;
        /*@only@*/ /*@null@*/ yasm_intnum *valign;
        unsigned long bss;
        unsigned long code;
    } data;

    static const yasm_dir_help help[] = {
        { "follows", 1, yasm_dir_helper_string,
          offsetof(struct bin_section_switch_data, follows), 0 },
        { "vfollows", 1, yasm_dir_helper_string,
          offsetof(struct bin_section_switch_data, vfollows), 0 },
        { "start", 1, yasm_dir_helper_expr,
          offsetof(struct bin_section_switch_data, start), 0 },
        { "vstart", 1, yasm_dir_helper_expr,
          offsetof(struct bin_section_switch_data, vstart), 0 },
        { "align", 1, yasm_dir_helper_intn,
          offsetof(struct bin_section_switch_data, align), 0 },
        { "valign", 1, yasm_dir_helper_intn,
          offsetof(struct bin_section_switch_data, valign), 0 },
        { "nobits", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, bss), 1 },
        { "progbits", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, bss), 0 },
        { "code", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, code), 1 },
        { "data", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, code), 0 },
        { "execute", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, code), 1 },
        { "noexecute", 0, yasm_dir_helper_flag_set,
          offsetof(struct bin_section_switch_data, code), 0 },
        { "gasflags", 1, bin_helper_gasflags, 0, 0 }
    };

    vp = yasm_vps_first(valparams);
    sectname = yasm_vp_string(vp);
    if (!sectname)
        return NULL;
    vp = yasm_vps_next(vp);

    retval = yasm_object_find_general(object, sectname);
    if (retval) {
        bsd = yasm_section_get_data(retval, &bin_section_data_cb);
        assert(bsd != NULL);
        data.follows = bsd->follows;
        data.vfollows = bsd->vfollows;
        data.start = bsd->start;
        data.vstart = bsd->vstart;
        data.align = NULL;
        data.valign = NULL;
        data.bss = bsd->bss;
        data.code = yasm_section_is_code(retval);
    } else {
        data.follows = NULL;
        data.vfollows = NULL;
        data.start = NULL;
        data.vstart = NULL;
        data.align = NULL;
        data.valign = NULL;
        data.bss = strcmp(sectname, ".bss") == 0;
        data.code = strcmp(sectname, ".text") == 0;
    }

    flags_override = yasm_dir_helper(object, vp, line, help, NELEMS(help),
                                     &data, yasm_dir_helper_valparam_warn);
    if (flags_override < 0)
        return NULL;    /* error occurred */

    if (data.start && data.follows) {
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("cannot combine `start' and `follows' section attributes"));
        return NULL;
    }

    if (data.vstart && data.vfollows) {
        yasm_error_set(YASM_ERROR_GENERAL,
            N_("cannot combine `vstart' and `vfollows' section attributes"));
        return NULL;
    }

    if (data.align) {
        unsigned long align = yasm_intnum_get_uint(data.align);

        /* Alignments must be a power of two. */
        if (!is_exp2(align)) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("argument to `%s' is not a power of two"),
                           "align");
            return NULL;
        }
    } else
        data.align = bsd ? bsd->align : NULL;

    if (data.valign) {
        unsigned long valign = yasm_intnum_get_uint(data.valign);

        /* Alignments must be a power of two. */
        if (!is_exp2(valign)) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("argument to `%s' is not a power of two"),
                           "valign");
            return NULL;
        }
    } else
        data.valign = bsd ? bsd->valign : NULL;

    retval = yasm_object_get_general(object, sectname, 0, (int)data.code,
                                     (int)data.bss, &isnew, line);

    bsd = yasm_section_get_data(retval, &bin_section_data_cb);

    if (isnew || yasm_section_is_default(retval)) {
        yasm_section_set_default(retval, 0);
    }

    /* Update section flags */
    bsd->bss = data.bss;
    bsd->align = data.align;
    bsd->valign = data.valign;
    bsd->start = data.start;
    bsd->vstart = data.vstart;
    bsd->follows = data.follows;
    bsd->vfollows = data.vfollows;

    return retval;
}

static /*@observer@*/ /*@null@*/ yasm_symrec *
bin_objfmt_get_special_sym(yasm_object *object, const char *name,
                           const char *parser)
{
    return NULL;
}

static void
bin_objfmt_dir_org(yasm_object *object,
                   /*@null@*/ yasm_valparamhead *valparams,
                   /*@unused@*/ /*@null@*/
                   yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)object->objfmt;
    yasm_valparam *vp;

    /* We only allow a single ORG in a program. */
    if (objfmt_bin->org) {
        yasm_error_set(YASM_ERROR_GENERAL, N_("program origin redefined"));
        return;
    }

    /* ORG takes just a simple expression as param */
    vp = yasm_vps_first(valparams);
    objfmt_bin->org = yasm_vp_expr(vp, object->symtab, line);
    if (!objfmt_bin->org) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("argument to ORG must be expression"));
        return;
    }
}

struct bin_dir_map_data {
    unsigned long flags;
    /*@only@*/ /*@null@*/ char *filename;
};

static int
dir_map_filename(void *obj, yasm_valparam *vp, unsigned long line, void *data)
{
    struct bin_dir_map_data *mdata = (struct bin_dir_map_data *)data;
    const char *filename;

    if (mdata->filename) {
        yasm_warn_set(YASM_WARN_GENERAL, N_("map file already specified"));
        return 0;
    }

    filename = yasm_vp_string(vp);
    if (!filename) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("unexpected expression in [map]"));
        return -1;
    }
    mdata->filename = yasm__xstrdup(filename);

    return 1;
}

static void
bin_objfmt_dir_map(yasm_object *object,
                   /*@null@*/ yasm_valparamhead *valparams,
                   /*@unused@*/ /*@null@*/
                   yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *)object->objfmt;

    struct bin_dir_map_data data;

    static const yasm_dir_help help[] = {
        { "all", 0, yasm_dir_helper_flag_or,
          offsetof(struct bin_dir_map_data, flags),
          MAP_BRIEF|MAP_SECTIONS|MAP_SYMBOLS },
        { "brief", 0, yasm_dir_helper_flag_or,
          offsetof(struct bin_dir_map_data, flags), MAP_BRIEF },
        { "sections", 0, yasm_dir_helper_flag_or,
          offsetof(struct bin_dir_map_data, flags), MAP_SECTIONS },
        { "segments", 0, yasm_dir_helper_flag_or,
          offsetof(struct bin_dir_map_data, flags), MAP_SECTIONS },
        { "symbols", 0, yasm_dir_helper_flag_or,
          offsetof(struct bin_dir_map_data, flags), MAP_SYMBOLS }
    };

    data.flags = objfmt_bin->map_flags | MAP_NONE;
    data.filename = objfmt_bin->map_filename;

    if (valparams && yasm_dir_helper(object, yasm_vps_first(valparams), line, help,
                                     NELEMS(help), &data, dir_map_filename) < 0)
        return;     /* error occurred */

    objfmt_bin->map_flags = data.flags;
    objfmt_bin->map_filename = data.filename;
}

static void
bin_section_data_destroy(void *data)
{
    bin_section_data *bsd = (bin_section_data *)data;
    if (bsd->start)
        yasm_expr_destroy(bsd->start);
    if (bsd->vstart)
        yasm_expr_destroy(bsd->vstart);
    if (bsd->follows)
        yasm_xfree(bsd->follows);
    if (bsd->vfollows)
        yasm_xfree(bsd->vfollows);
    if (bsd->istart)
        yasm_intnum_destroy(bsd->istart);
    if (bsd->ivstart)
        yasm_intnum_destroy(bsd->ivstart);
    if (bsd->length)
        yasm_intnum_destroy(bsd->length);
    yasm_xfree(data);
}

static void
bin_section_data_print(void *data, FILE *f, int indent_level)
{
    bin_section_data *bsd = (bin_section_data *)data;

    fprintf(f, "%*sbss=%d\n", indent_level, "", bsd->bss);

    fprintf(f, "%*salign=", indent_level, "");
    if (bsd->align)
        yasm_intnum_print(bsd->align, f);
    else
        fprintf(f, "(nil)");
    fprintf(f, "\n%*svalign=", indent_level, "");
    if (bsd->valign)
        yasm_intnum_print(bsd->valign, f);
    else
        fprintf(f, "(nil)");

    fprintf(f, "\n%*sstart=", indent_level, "");
    yasm_expr_print(bsd->start, f);
    fprintf(f, "\n%*svstart=", indent_level, "");
    yasm_expr_print(bsd->vstart, f);

    fprintf(f, "\n%*sfollows=", indent_level, "");
    if (bsd->follows)
        fprintf(f, "\"%s\"", bsd->follows);
    else
        fprintf(f, "(nil)");
    fprintf(f, "\n%*svfollows=", indent_level, "");
    if (bsd->vfollows)
        fprintf(f, "\"%s\"", bsd->vfollows);
    else
        fprintf(f, "(nil)");

    fprintf(f, "\n%*sistart=", indent_level, "");
    if (bsd->istart)
        yasm_intnum_print(bsd->istart, f);
    else
        fprintf(f, "(nil)");
    fprintf(f, "\n%*sivstart=", indent_level, "");
    if (bsd->ivstart)
        yasm_intnum_print(bsd->ivstart, f);
    else
        fprintf(f, "(nil)");

    fprintf(f, "\n%*slength=", indent_level, "");
    if (bsd->length)
        yasm_intnum_print(bsd->length, f);
    else
        fprintf(f, "(nil)");
    fprintf(f, "\n");
}

static void
bin_symrec_data_destroy(void *data)
{
    yasm_xfree(data);
}

static void
bin_symrec_data_print(void *data, FILE *f, int indent_level)
{
    bin_symrec_data *bsymd = (bin_symrec_data *)data;

    fprintf(f, "%*ssection=\"%s\"\n", indent_level, "",
            yasm_section_get_name(bsymd->section));
    fprintf(f, "%*swhich=", indent_level, "");
    switch (bsymd->which) {
        case SSYM_START: fprintf(f, "START"); break;
        case SSYM_VSTART: fprintf(f, "VSTART"); break;
        case SSYM_LENGTH: fprintf(f, "LENGTH"); break;
    }
    fprintf(f, "\n");
}


/* Define valid debug formats to use with this object format */
static const char *bin_objfmt_dbgfmt_keywords[] = {
    "null",
    NULL
};

static const yasm_directive bin_objfmt_directives[] = {
    { "org",    "nasm", bin_objfmt_dir_org,     YASM_DIR_ARG_REQUIRED },
    { "map",    "nasm", bin_objfmt_dir_map,     YASM_DIR_ANY },
    { NULL, NULL, NULL, 0 }
};

static const char *bin_nasm_stdmac[] = {
    "%imacro org 1+.nolist",
    "[org %1]",
    "%endmacro",
    NULL
};

static const yasm_stdmac bin_objfmt_stdmacs[] = {
    { "nasm", "nasm", bin_nasm_stdmac },
    { "tasm", "tasm", bin_nasm_stdmac },
    { NULL, NULL, NULL }
};

/* Define objfmt structure -- see objfmt.h for details */
yasm_objfmt_module yasm_bin_LTX_objfmt = {
    "Flat format binary",
    "bin",
    NULL,
    16,
    0,
    bin_objfmt_dbgfmt_keywords,
    "null",
    bin_objfmt_directives,
    bin_objfmt_stdmacs,
    bin_objfmt_create,
    bin_objfmt_output,
    bin_objfmt_destroy,
    bin_objfmt_add_default_section,
    bin_objfmt_init_new_section,
    bin_objfmt_section_switch,
    bin_objfmt_get_special_sym
};

#define EXE_HEADER_SIZE 0x200

/* DOS .EXE binaries are just raw binaries with a header */
yasm_objfmt_module yasm_dosexe_LTX_objfmt;

static yasm_objfmt *
dosexe_objfmt_create(yasm_object *object)
{
    yasm_objfmt_bin *objfmt_bin = (yasm_objfmt_bin *) bin_objfmt_create(object);
    objfmt_bin->objfmt.module = &yasm_dosexe_LTX_objfmt;
    return (yasm_objfmt *)objfmt_bin;
}

static unsigned long
get_sym(yasm_object *object, const char *name) {
    yasm_symrec *symrec = yasm_symtab_get(object->symtab, name);
    yasm_bytecode *prevbc;
    if (!symrec)
        return 0;
    if (!yasm_symrec_get_label(symrec, &prevbc))
        return 0;
    return prevbc->offset + prevbc->len;
}

static void
dosexe_objfmt_output(yasm_object *object, FILE *f, /*@unused@*/ int all_syms,
                  yasm_errwarns *errwarns)
{
    unsigned long tot_size, size, bss_size;
    unsigned long start, bss;
    unsigned char c;

    fseek(f, EXE_HEADER_SIZE, SEEK_SET);

    bin_objfmt_output(object, f, all_syms, errwarns);

    tot_size = ftell(f);

    /* if there is a __bss_start symbol, data after it is 0, no need to write
     * it.  */
    bss = get_sym(object, "__bss_start");
    if (bss)
        size = bss;
    else
        size = tot_size;
    bss_size = tot_size - size;
#ifdef HAVE_FTRUNCATE
    if (size != tot_size)
        ftruncate(fileno(f), EXE_HEADER_SIZE + size);
#endif
    fseek(f, 0, SEEK_SET);

    /* magic */
    fwrite("MZ", 1, 2, f);

    /* file size */
    c = size & 0xff;
    fwrite(&c, 1, 1, f);
    c = !!(size & 0x100);
    fwrite(&c, 1, 1, f);
    c = ((size + 511) >> 9) & 0xff;
    fwrite(&c, 1, 1, f);
    c = ((size + 511) >> 17) & 0xff;
    fwrite(&c, 1, 1, f);

    /* relocation # */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* header size */
    c = EXE_HEADER_SIZE / 16;
    fwrite(&c, 1, 1, f);
    c = 0;
    fwrite(&c, 1, 1, f);

    /* minimum paragraph # */
    bss_size = (bss_size + 15) >> 4;
    c = bss_size & 0xff;
    fwrite(&c, 1, 1, f);
    c = (bss_size >> 8) & 0xff;
    fwrite(&c, 1, 1, f);

    /* maximum paragraph # */
    c = 0xFF;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* relative value of stack segment */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* SP at start */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* header checksum */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* IP at start */
    start = get_sym(object, "start");
    if (!start) {
        yasm_error_set(YASM_ERROR_GENERAL,
                N_("%s: could not find symbol `start'"));
        return;
    }
    c = start & 0xff;
    fwrite(&c, 1, 1, f);
    c = (start >> 8) & 0xff;
    fwrite(&c, 1, 1, f);

    /* CS start */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);

    /* reloc start */
    c = 0x22;
    fwrite(&c, 1, 1, f);
    c = 0;
    fwrite(&c, 1, 1, f);

    /* Overlay number */
    c = 0;
    fwrite(&c, 1, 1, f);
    fwrite(&c, 1, 1, f);
}


/* Define objfmt structure -- see objfmt.h for details */
yasm_objfmt_module yasm_dosexe_LTX_objfmt = {
    "DOS .EXE format binary",
    "dosexe",
    "exe",
    16,
    0,
    bin_objfmt_dbgfmt_keywords,
    "null",
    bin_objfmt_directives,
    bin_objfmt_stdmacs,
    dosexe_objfmt_create,
    dosexe_objfmt_output,
    bin_objfmt_destroy,
    bin_objfmt_add_default_section,
    bin_objfmt_init_new_section,
    bin_objfmt_section_switch,
    bin_objfmt_get_special_sym
};
