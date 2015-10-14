/*
 * Relocatable Dynamic Object File Format (RDOFF) version 2 format
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
#include <util.h>

#include <libyasm.h>


#define REGULAR_OUTBUF_SIZE     1024

#define RDF_MAGIC       "RDOFF2"

/* Maximum size of an import/export label (including trailing zero) */
#define EXIM_LABEL_MAX          64

/* Maximum size of library or module name (including trailing zero) */
#define MODLIB_NAME_MAX         128

/* Maximum number of segments that we can handle in one file */
#define RDF_MAXSEGS             64

/* Record types that may present the RDOFF header */
#define RDFREC_GENERIC          0
#define RDFREC_RELOC            1
#define RDFREC_IMPORT           2
#define RDFREC_GLOBAL           3
#define RDFREC_DLL              4
#define RDFREC_BSS              5
#define RDFREC_SEGRELOC         6
#define RDFREC_FARIMPORT        7
#define RDFREC_MODNAME          8
#define RDFREC_COMMON           10

/* Flags for ExportRec/ImportRec */
#define SYM_DATA        1
#define SYM_FUNCTION    2

/* Flags for ExportRec */
#define SYM_GLOBAL      4

/* Flags for ImportRec */
#define SYM_IMPORT      8
#define SYM_FAR         16

typedef struct rdf_reloc {
    yasm_reloc reloc;
    enum {
        RDF_RELOC_NORM,     /* normal */
        RDF_RELOC_REL,      /* relative to current position */
        RDF_RELOC_SEG       /* segment containing symbol */
    } type;                         /* type of relocation */
    unsigned int size;
    unsigned int refseg;
} rdf_reloc;

typedef struct rdf_section_data {
    /*@dependent@*/ yasm_symrec *sym;   /* symbol created for this section */
    long scnum;             /* section number (0=first section) */
    enum {
        RDF_SECT_BSS = 0,
        RDF_SECT_CODE = 1,
        RDF_SECT_DATA = 2,
        RDF_SECT_COMMENT = 3,
        RDF_SECT_LCOMMENT = 4,
        RDF_SECT_PCOMMENT = 5,
        RDF_SECT_SYMDEBUG = 6,
        RDF_SECT_LINEDEBUG = 7
    } type;                 /* section type */
    unsigned int reserved;  /* reserved data */
    unsigned long size;     /* size of raw data (section data) in bytes */

    unsigned char *raw_data;    /* raw section data, only used during output */
} rdf_section_data;

typedef struct rdf_symrec_data {
    unsigned int segment;               /* assigned RDF "segment" index */
} rdf_symrec_data;

typedef STAILQ_HEAD(xdf_str_head, xdf_str) xdf_str_head;
typedef struct xdf_str {
    STAILQ_ENTRY(xdf_str) link;
    /*@owned@*/ char *str;
} xdf_str;

typedef struct yasm_objfmt_rdf {
    yasm_objfmt_base objfmt;                /* base structure */

    long parse_scnum;               /* sect numbering in parser */

    /*@owned@*/ xdf_str_head module_names;
    /*@owned@*/ xdf_str_head library_names;
} yasm_objfmt_rdf;

typedef struct rdf_objfmt_output_info {
    yasm_object *object;
    yasm_objfmt_rdf *objfmt_rdf;
    yasm_errwarns *errwarns;
    /*@dependent@*/ FILE *f;
    /*@only@*/ unsigned char *buf;
    yasm_section *sect;
    /*@dependent@*/ rdf_section_data *rsd;

    unsigned long indx;             /* symbol "segment" (extern/common only) */

    unsigned long bss_size;         /* total BSS size */
} rdf_objfmt_output_info;

static void rdf_section_data_destroy(/*@only@*/ void *d);
static void rdf_section_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback rdf_section_data_cb = {
    rdf_section_data_destroy,
    rdf_section_data_print
};

static void rdf_symrec_data_destroy(/*@only@*/ void *d);
static void rdf_symrec_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback rdf_symrec_data_cb = {
    rdf_symrec_data_destroy,
    rdf_symrec_data_print
};

yasm_objfmt_module yasm_rdf_LTX_objfmt;


static /*@dependent@*/ rdf_symrec_data *
rdf_objfmt_sym_set_data(yasm_symrec *sym, unsigned int segment)
{
    rdf_symrec_data *rsymd = yasm_xmalloc(sizeof(rdf_symrec_data));

    rsymd->segment = segment;

    yasm_symrec_add_data(sym, &rdf_symrec_data_cb, rsymd);
    return rsymd;
}

static yasm_objfmt *
rdf_objfmt_create(yasm_object *object)
{
    yasm_objfmt_rdf *objfmt_rdf = yasm_xmalloc(sizeof(yasm_objfmt_rdf));

    /* We theoretically support all arches, so don't check.
     * Really we only support byte-addressable ones.
     */

    objfmt_rdf->parse_scnum = 0;    /* section numbering starts at 0 */

    STAILQ_INIT(&objfmt_rdf->module_names);
    STAILQ_INIT(&objfmt_rdf->library_names);

    objfmt_rdf->objfmt.module = &yasm_rdf_LTX_objfmt;

    return (yasm_objfmt *)objfmt_rdf;
}

static int
rdf_objfmt_output_value(yasm_value *value, unsigned char *buf,
                        unsigned int destsize, unsigned long offset,
                        yasm_bytecode *bc, int warn, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    yasm_objfmt_rdf *objfmt_rdf;
    /*@dependent@*/ /*@null@*/ yasm_intnum *intn;
    unsigned long intn_minus;
    unsigned long intn_plus;
    int retval;
    unsigned int valsize = value->size;

    assert(info != NULL);
    objfmt_rdf = info->objfmt_rdf;

    if (value->abs)
        value->abs = yasm_expr_simplify(value->abs, 1);

    /* Try to output constant and PC-relative section-local first.
     * Note this does NOT output any value with a SEG, WRT, external,
     * cross-section, or non-PC-relative reference (those are handled below).
     */
    switch (yasm_value_output_basic(value, buf, destsize, bc, warn,
                                    info->object->arch)) {
        case -1:
            return 1;
        case 0:
            break;
        default:
            return 0;
    }

    if (value->section_rel) {
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("rdf: relocation too complex"));
        return 1;
    }

    if (value->rel && value->wrt) {
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("rdf: WRT not supported"));
        return 1;
    }

    intn_minus = 0;
    intn_plus = 0;
    if (value->rel) {
        rdf_reloc *reloc;
        /*@null@*/ rdf_symrec_data *rsymd;
        /*@dependent@*/ yasm_bytecode *precbc;

        reloc = yasm_xmalloc(sizeof(rdf_reloc));
        reloc->reloc.addr = yasm_intnum_create_uint(bc->offset + offset);
        reloc->reloc.sym = value->rel;
        reloc->size = valsize/8;

        if (value->seg_of)
            reloc->type = RDF_RELOC_SEG;
        else if (value->curpos_rel) {
            reloc->type = RDF_RELOC_REL;
            /* Adjust to start of section, so subtract out the bytecode
             * offset.
             */
            intn_minus = bc->offset;
        } else
            reloc->type = RDF_RELOC_NORM;

        if (yasm_symrec_get_label(value->rel, &precbc)) {
            /* local, set the value to be the offset, and the refseg to the
             * segment number.
             */
            /*@dependent@*/ /*@null@*/ rdf_section_data *csectd;
            /*@dependent@*/ yasm_section *sect;

            sect = yasm_bc_get_section(precbc);
            csectd = yasm_section_get_data(sect, &rdf_section_data_cb);
            if (!csectd)
                yasm_internal_error(N_("didn't understand section"));
            reloc->refseg = csectd->scnum;
            intn_plus = yasm_bc_next_offset(precbc);
        } else {
            /* must be common/external */
            rsymd = yasm_symrec_get_data(reloc->reloc.sym,
                                         &rdf_symrec_data_cb);
            if (!rsymd)
                yasm_internal_error(
                    N_("rdf: no symbol data for relocated symbol"));
            reloc->refseg = rsymd->segment;
        }

        yasm_section_add_reloc(info->sect, (yasm_reloc *)reloc, yasm_xfree);
    }

    if (intn_minus > 0) {
        intn = yasm_intnum_create_uint(intn_minus);
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);
    } else
        intn = yasm_intnum_create_uint(intn_plus);

    if (value->abs) {
        yasm_intnum *intn2 = yasm_expr_get_intnum(&value->abs, 0);
        if (!intn2) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("rdf: relocation too complex"));
            yasm_intnum_destroy(intn);
            return 1;
        }
        yasm_intnum_calc(intn, YASM_EXPR_ADD, intn2);
    }

    retval = yasm_arch_intnum_tobytes(info->object->arch, intn, buf, destsize,
                                      valsize, 0, bc, warn);
    yasm_intnum_destroy(intn);
    return retval;
}

static int
rdf_objfmt_output_bytecode(yasm_bytecode *bc, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    /*@null@*/ /*@only@*/ unsigned char *bigbuf;
    unsigned long size = REGULAR_OUTBUF_SIZE;
    int gap;

    assert(info != NULL);

    bigbuf = yasm_bc_tobytes(bc, info->buf, &size, &gap, info,
                             rdf_objfmt_output_value, NULL);

    /* Don't bother doing anything else if size ended up being 0. */
    if (size == 0) {
        if (bigbuf)
            yasm_xfree(bigbuf);
        return 0;
    }

    /* Warn that gaps are converted to 0 and write out the 0's. */
    if (gap) {
        yasm_warn_set(YASM_WARN_UNINIT_CONTENTS,
                      N_("uninitialized space: zeroing"));
        /* Write out in chunks */
        memset(&info->rsd->raw_data[info->rsd->size], 0, size);
    } else {
        /* Output buf (or bigbuf if non-NULL) to file */
        memcpy(&info->rsd->raw_data[info->rsd->size],
               bigbuf ? bigbuf : info->buf, (size_t)size);
    }

    info->rsd->size += size;

    /* If bigbuf was allocated, free it */
    if (bigbuf)
        yasm_xfree(bigbuf);

    return 0;
}

static int
rdf_objfmt_output_section_mem(yasm_section *sect, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ rdf_section_data *rsd;
    unsigned long size;

    assert(info != NULL);
    rsd = yasm_section_get_data(sect, &rdf_section_data_cb);
    assert(rsd != NULL);

    size = yasm_bc_next_offset(yasm_section_bcs_last(sect));

    if (rsd->type == RDF_SECT_BSS) {
        /* Don't output BSS sections, but remember length
         * TODO: Check for non-reserve bytecodes?
         */
        info->bss_size += size;
        return 0;
    }

    /* Empty?  Go on to next section */
    if (size == 0)
        return 0;

    /* See UGH comment in output() for why we're doing this */
    rsd->raw_data = yasm_xmalloc(size);
    rsd->size = 0;

    info->sect = sect;
    info->rsd = rsd;
    yasm_section_bcs_traverse(sect, info->errwarns, info,
                              rdf_objfmt_output_bytecode);

    /* Sanity check final section size */
    if (rsd->size != size)
        yasm_internal_error(
            N_("rdf: section computed size did not match actual size"));

    return 0;
}

static int
rdf_objfmt_output_section_reloc(yasm_section *sect, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ rdf_section_data *rsd;
    rdf_reloc *reloc;

    assert(info != NULL);
    rsd = yasm_section_get_data(sect, &rdf_section_data_cb);
    assert(rsd != NULL);

    if (rsd->type == RDF_SECT_BSS) {
        /* Don't output BSS sections. */
        return 0;
    }

    /* Empty?  Go on to next section */
    if (rsd->size == 0)
        return 0;

    reloc = (rdf_reloc *)yasm_section_relocs_first(sect);
    while (reloc) {
        unsigned char *localbuf = info->buf;

        if (reloc->type == RDF_RELOC_SEG)
            YASM_WRITE_8(localbuf, RDFREC_SEGRELOC);
        else
            YASM_WRITE_8(localbuf, RDFREC_RELOC);
        YASM_WRITE_8(localbuf, 8);              /* record length */
        /* Section number, +0x40 if relative reloc */
        YASM_WRITE_8(localbuf, rsd->scnum +
                     (reloc->type == RDF_RELOC_REL ? 0x40 : 0));
        yasm_intnum_get_sized(reloc->reloc.addr, localbuf, 4, 32, 0, 0, 0);
        localbuf += 4;                          /* offset of relocation */
        YASM_WRITE_8(localbuf, reloc->size);        /* size of relocation */
        YASM_WRITE_16_L(localbuf, reloc->refseg);   /* relocated symbol */
        fwrite(info->buf, 10, 1, info->f);

        reloc = (rdf_reloc *)yasm_section_reloc_next((yasm_reloc *)reloc);
    }

    return 0;
}

static int
rdf_objfmt_output_section_file(yasm_section *sect, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ rdf_section_data *rsd;
    unsigned char *localbuf;

    assert(info != NULL);
    rsd = yasm_section_get_data(sect, &rdf_section_data_cb);
    assert(rsd != NULL);

    if (rsd->type == RDF_SECT_BSS) {
        /* Don't output BSS sections. */
        return 0;
    }

    /* Empty?  Go on to next section */
    if (rsd->size == 0)
        return 0;

    /* Section header */
    localbuf = info->buf;
    YASM_WRITE_16_L(localbuf, rsd->type);       /* type */
    YASM_WRITE_16_L(localbuf, rsd->scnum);      /* number */
    YASM_WRITE_16_L(localbuf, rsd->reserved);   /* reserved */
    YASM_WRITE_32_L(localbuf, rsd->size);       /* length */
    fwrite(info->buf, 10, 1, info->f);

    /* Section data */
    fwrite(rsd->raw_data, rsd->size, 1, info->f);

    /* Free section data */
    yasm_xfree(rsd->raw_data);
    rsd->raw_data = NULL;

    return 0;
}

#define FLAG_EXT    0x1000
#define FLAG_GLOB   0x2000
#define FLAG_SET    0x4000
#define FLAG_CLR    0x8000
#define FLAG_MASK   0x0fff

static int
rdf_helper_flag(void *obj, yasm_valparam *vp, unsigned long line, void *d,
                uintptr_t flag)
{
    yasm_symrec *sym = (yasm_symrec *)obj;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);
    unsigned int *flags = (unsigned int *)d;

    if (((vis & YASM_SYM_GLOBAL) && (flag & FLAG_GLOB)) ||
        ((vis & YASM_SYM_EXTERN) && (flag & FLAG_EXT))) {
        if (flag & FLAG_SET)
            *flags |= flag & FLAG_MASK;
        else if (flag & FLAG_CLR)
            *flags &= ~(flag & FLAG_MASK);
    }
    return 0;
}

static unsigned int
rdf_parse_flags(yasm_symrec *sym)
{
    /*@dependent@*/ /*@null@*/ yasm_valparamhead *objext_valparams =
        yasm_symrec_get_objext_valparams(sym);
    unsigned int flags = 0;

    static const yasm_dir_help help[] = {
        { "data", 0, rdf_helper_flag, 0,
          FLAG_EXT|FLAG_GLOB|FLAG_SET|SYM_DATA },
        { "object", 0, rdf_helper_flag, 0,
          FLAG_EXT|FLAG_GLOB|FLAG_SET|SYM_DATA },
        { "proc", 0, rdf_helper_flag, 0,
          FLAG_EXT|FLAG_GLOB|FLAG_SET|SYM_FUNCTION },
        { "function", 0, rdf_helper_flag, 0,
          FLAG_EXT|FLAG_GLOB|FLAG_SET|SYM_FUNCTION },
        { "import", 0, rdf_helper_flag, 0, FLAG_EXT|FLAG_SET|SYM_IMPORT },
        { "export", 0, rdf_helper_flag, 0, FLAG_GLOB|FLAG_SET|SYM_GLOBAL },
        { "far", 0, rdf_helper_flag, 0, FLAG_EXT|FLAG_SET|SYM_FAR },
        { "near", 0, rdf_helper_flag, 0, FLAG_EXT|FLAG_CLR|SYM_FAR }
    };

    if (!objext_valparams)
        return 0;

    yasm_dir_helper(sym, yasm_vps_first(objext_valparams), 0, help,
                    NELEMS(help), &flags, yasm_dir_helper_valparam_warn);

    return flags;
}

static int
rdf_objfmt_output_sym(yasm_symrec *sym, /*@null@*/ void *d)
{
    /*@null@*/ rdf_objfmt_output_info *info = (rdf_objfmt_output_info *)d;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);
    /*@only@*/ char *name;
    size_t len;
    unsigned long value = 0;
    unsigned int scnum = 0;
    /*@dependent@*/ /*@null@*/ yasm_section *sect;
    /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
    unsigned char *localbuf;

    assert(info != NULL);

    if (vis == YASM_SYM_LOCAL || vis == YASM_SYM_DLOCAL)
        return 0;   /* skip local syms */

    /* Look at symrec for value/scnum/etc. */
    if (yasm_symrec_get_label(sym, &precbc)) {
        /*@dependent@*/ /*@null@*/ rdf_section_data *csectd;

        if (precbc)
            sect = yasm_bc_get_section(precbc);
        else
            sect = NULL;
        if (!sect)
            return 0;

        /* it's a label: get value and offset. */
        csectd = yasm_section_get_data(sect, &rdf_section_data_cb);
        if (csectd)
            scnum = csectd->scnum;
        else
            yasm_internal_error(N_("didn't understand section"));
        value = yasm_bc_next_offset(precbc);
    } else if (yasm_symrec_get_equ(sym)) {
        yasm_warn_set(YASM_WARN_GENERAL,
            N_("rdf does not support exporting EQU/absolute values"));
        yasm_errwarn_propagate(info->errwarns, yasm_symrec_get_decl_line(sym));
        return 0;
    }

    name = yasm_symrec_get_global_name(sym, info->object);
    len = strlen(name);

    if (len > EXIM_LABEL_MAX-1) {
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("label name too long, truncating to %d bytes"),
                      EXIM_LABEL_MAX);
        len = EXIM_LABEL_MAX-1;
    }

    localbuf = info->buf;
    if (vis & YASM_SYM_GLOBAL) {
        YASM_WRITE_8(localbuf, RDFREC_GLOBAL);
        YASM_WRITE_8(localbuf, 6+len+1);        /* record length */
        YASM_WRITE_8(localbuf, rdf_parse_flags(sym));   /* flags */
        YASM_WRITE_8(localbuf, scnum);          /* segment referred to */
        YASM_WRITE_32_L(localbuf, value);       /* offset */
    } else {
        /* Save symbol segment in symrec data (for later reloc gen) */
        scnum = info->indx++;
        rdf_objfmt_sym_set_data(sym, scnum);

        if (vis & YASM_SYM_COMMON) {
            /*@dependent@*/ /*@null@*/ yasm_expr **csize_expr;
            const yasm_intnum *intn;
            /*@dependent@*/ /*@null@*/ yasm_valparamhead *objext_valparams =
                yasm_symrec_get_objext_valparams(sym);
            unsigned long addralign = 0;

            YASM_WRITE_8(localbuf, RDFREC_COMMON);
            YASM_WRITE_8(localbuf, 8+len+1);    /* record length */
            YASM_WRITE_16_L(localbuf, scnum);   /* segment allocated */

            /* size */
            csize_expr = yasm_symrec_get_common_size(sym);
            assert(csize_expr != NULL);
            intn = yasm_expr_get_intnum(csize_expr, 1);
            if (!intn) {
                yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                    N_("COMMON data size not an integer expression"));
            } else
                value = yasm_intnum_get_uint(intn);
            YASM_WRITE_32_L(localbuf, value);

            /* alignment */
            if (objext_valparams) {
                yasm_valparam *vp = yasm_vps_first(objext_valparams);
                for (; vp; vp = yasm_vps_next(vp)) {
                    if (!vp->val) {
                        /*@only@*/ /*@null@*/ yasm_expr *align_expr;
                        /*@dependent@*/ /*@null@*/
                        const yasm_intnum *align_intn;

                        if (!(align_expr = yasm_vp_expr(vp,
                                info->object->symtab,
                                yasm_symrec_get_decl_line(sym))) ||
                            !(align_intn = yasm_expr_get_intnum(&align_expr,
                                                                0))) {
                            yasm_error_set(YASM_ERROR_VALUE,
                                N_("argument to `%s' is not an integer"),
                                vp->val);
                            if (align_expr)
                                yasm_expr_destroy(align_expr);
                            continue;
                        }
                        addralign = yasm_intnum_get_uint(align_intn);
                        yasm_expr_destroy(align_expr);

                        /* Alignments must be a power of two. */
                        if (!is_exp2(addralign)) {
                            yasm_error_set(YASM_ERROR_VALUE,
                                N_("alignment constraint is not a power of two"));
                            continue;
                        }
                    } else
                        yasm_warn_set(YASM_WARN_GENERAL,
                            N_("Unrecognized qualifier `%s'"), vp->val);
                }
            }
            YASM_WRITE_16_L(localbuf, addralign);
        } else if (vis & YASM_SYM_EXTERN) {
            unsigned int flags = rdf_parse_flags(sym);
            if (flags & SYM_FAR) {
                YASM_WRITE_8(localbuf, RDFREC_FARIMPORT);
                flags &= ~SYM_FAR;
            } else
                YASM_WRITE_8(localbuf, RDFREC_IMPORT);
            YASM_WRITE_8(localbuf, 3+len+1);    /* record length */
            YASM_WRITE_8(localbuf, flags);      /* flags */
            YASM_WRITE_16_L(localbuf, scnum);   /* segment allocated */
        }
    }

    /* Symbol name */
    memcpy(localbuf, name, len);
    localbuf += len;
    YASM_WRITE_8(localbuf, 0);          /* 0-terminated name */
    yasm_xfree(name);

    fwrite(info->buf, (unsigned long)(localbuf-info->buf), 1, info->f);

    yasm_errwarn_propagate(info->errwarns, yasm_symrec_get_decl_line(sym));
    return 0;
}

static void
rdf_objfmt_output(yasm_object *object, FILE *f, int all_syms,
                  yasm_errwarns *errwarns)
{
    yasm_objfmt_rdf *objfmt_rdf = (yasm_objfmt_rdf *)object->objfmt;
    rdf_objfmt_output_info info;
    unsigned char *localbuf;
    long headerlen, filelen;
    xdf_str *cur;
    size_t len;

    info.object = object;
    info.objfmt_rdf = objfmt_rdf;
    info.errwarns = errwarns;
    info.f = f;
    info.buf = yasm_xmalloc(REGULAR_OUTBUF_SIZE);
    info.bss_size = 0;

    /* Allocate space for file header by seeking forward */
    if (fseek(f, (long)strlen(RDF_MAGIC)+8, SEEK_SET) < 0) {
        yasm__fatal(N_("could not seek on output file"));
        /*@notreached@*/
        return;
    }

    /* Output custom header records (library and module, etc) */
    cur = STAILQ_FIRST(&objfmt_rdf->module_names);
    while (cur) {
        len = strlen(cur->str)+1;
        localbuf = info.buf;
        YASM_WRITE_8(localbuf, RDFREC_MODNAME);         /* record type */
        YASM_WRITE_8(localbuf, len);                    /* record length */
        fwrite(info.buf, 2, 1, f);
        fwrite(cur->str, len, 1, f);
        cur = STAILQ_NEXT(cur, link);
    }

    cur = STAILQ_FIRST(&objfmt_rdf->library_names);
    while (cur) {
        len = strlen(cur->str)+1;
        localbuf = info.buf;
        YASM_WRITE_8(localbuf, RDFREC_DLL);             /* record type */
        YASM_WRITE_8(localbuf, len);                    /* record length */
        fwrite(info.buf, 2, 1, f);
        fwrite(cur->str, len, 1, f);
        cur = STAILQ_NEXT(cur, link);
    }

    /* Output symbol table */
    info.indx = objfmt_rdf->parse_scnum;
    yasm_symtab_traverse(object->symtab, &info, rdf_objfmt_output_sym);

    /* UGH! Due to the fact the relocs go at the beginning of the file, and
     * we only know if we have relocs when we output the sections, we have
     * to output the section data before we have output the relocs.  But
     * we also don't know how much space to preallocate for relocs, so....
     * we output into memory buffers first (thus the UGH).
     *
     * Stupid object format design, if you ask me (basically all other
     * object formats put the relocs *after* the section data to avoid this
     * exact problem).
     *
     * We also calculate the total size of all BSS sections here.
     */
    if (yasm_object_sections_traverse(object, &info,
                                      rdf_objfmt_output_section_mem))
        return;

    /* Output all relocs */
    if (yasm_object_sections_traverse(object, &info,
                                      rdf_objfmt_output_section_reloc))
        return;

    /* Output BSS record */
    if (info.bss_size > 0) {
        localbuf = info.buf;
        YASM_WRITE_8(localbuf, RDFREC_BSS);             /* record type */
        YASM_WRITE_8(localbuf, 4);                      /* record length */
        YASM_WRITE_32_L(localbuf, info.bss_size);       /* total BSS size */
        fwrite(info.buf, 6, 1, f);
    }

    /* Determine header length */
    headerlen = ftell(f);
    if (headerlen == -1) {
        yasm__fatal(N_("could not get file position on output file"));
        /*@notreached@*/
        return;
    }

    /* Section data (to file) */
    if (yasm_object_sections_traverse(object, &info,
                                      rdf_objfmt_output_section_file))
        return;

    /* NULL section to end file */
    memset(info.buf, 0, 10);
    fwrite(info.buf, 10, 1, f);

    /* Determine object length */
    filelen = ftell(f);
    if (filelen == -1) {
        yasm__fatal(N_("could not get file position on output file"));
        /*@notreached@*/
        return;
    }

    /* Write file header */
    if (fseek(f, 0, SEEK_SET) < 0) {
        yasm__fatal(N_("could not seek on output file"));
        /*@notreached@*/
        return;
    }

    fwrite(RDF_MAGIC, strlen(RDF_MAGIC), 1, f);
    localbuf = info.buf;
    YASM_WRITE_32_L(localbuf, filelen-10);              /* object size */
    YASM_WRITE_32_L(localbuf, headerlen-14);            /* header size */
    fwrite(info.buf, 8, 1, f);

    yasm_xfree(info.buf);
}

static void
rdf_objfmt_destroy(yasm_objfmt *objfmt)
{
    yasm_objfmt_rdf *objfmt_rdf = (yasm_objfmt_rdf *)objfmt;
    xdf_str *cur, *next;

    cur = STAILQ_FIRST(&objfmt_rdf->module_names);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        yasm_xfree(cur->str);
        yasm_xfree(cur);
        cur = next;
    }

    cur = STAILQ_FIRST(&objfmt_rdf->library_names);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        yasm_xfree(cur->str);
        yasm_xfree(cur);
        cur = next;
    }

    yasm_xfree(objfmt);
}

static void
rdf_objfmt_init_new_section(yasm_section *sect, unsigned long line)
{
    yasm_object *object = yasm_section_get_object(sect);
    const char *sectname = yasm_section_get_name(sect);
    yasm_objfmt_rdf *objfmt_rdf = (yasm_objfmt_rdf *)object->objfmt;
    rdf_section_data *data;
    yasm_symrec *sym;

    data = yasm_xmalloc(sizeof(rdf_section_data));
    data->scnum = objfmt_rdf->parse_scnum++;
    data->type = 0;
    data->reserved = 0;
    data->size = 0;
    data->raw_data = NULL;
    yasm_section_add_data(sect, &rdf_section_data_cb, data);

    sym = yasm_symtab_define_label(object->symtab, sectname,
                                   yasm_section_bcs_first(sect), 1, line);
    data->sym = sym;
}

static yasm_section *
rdf_objfmt_add_default_section(yasm_object *object)
{
    yasm_section *retval;
    rdf_section_data *rsd;
    int isnew;

    retval = yasm_object_get_general(object, ".text", 0, 1, 0, &isnew, 0);
    if (isnew) {
        rsd = yasm_section_get_data(retval, &rdf_section_data_cb);
        rsd->type = RDF_SECT_CODE;
        rsd->reserved = 0;
        yasm_section_set_default(retval, 1);
    }
    return retval;
}

static int
rdf_helper_set_type(void *obj, yasm_valparam *vp, unsigned long line,
                    void *d, uintptr_t newtype)
{
    unsigned int *type = (unsigned int *)d;
    *type = newtype;
    return 0;
}

struct rdf_section_switch_data {
    /*@only@*/ /*@null@*/ yasm_intnum *reserved_intn;
    unsigned int type;
};

static int
rdf_helper_set_reserved(void *obj, yasm_valparam *vp, unsigned long line,
                        void *d)
{
    struct rdf_section_switch_data *data = (struct rdf_section_switch_data *)d;

    if (!vp->val && vp->type == YASM_PARAM_EXPR)
        return yasm_dir_helper_intn(obj, vp, line, &data->reserved_intn, 0);
    else
        return yasm_dir_helper_valparam_warn(obj, vp, line, d);
}

static /*@observer@*/ /*@null@*/ yasm_section *
rdf_objfmt_section_switch(yasm_object *object, yasm_valparamhead *valparams,
                          /*@unused@*/ /*@null@*/
                          yasm_valparamhead *objext_valparams,
                          unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    yasm_section *retval;
    int isnew;
    unsigned int reserved = 0;
    int flags_override = 0;
    const char *sectname;
    rdf_section_data *rsd;

    struct rdf_section_switch_data data;

    static const yasm_dir_help help[] = {
        { "bss", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_BSS },
        { "code", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_CODE },
        { "text", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_CODE },
        { "data", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_DATA },
        { "comment", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_COMMENT },
        { "lcomment", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_LCOMMENT },
        { "pcomment", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_PCOMMENT },
        { "symdebug", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_SYMDEBUG },
        { "linedebug", 0, rdf_helper_set_type,
          offsetof(struct rdf_section_switch_data, type), RDF_SECT_LINEDEBUG },
        { "reserved", 1, yasm_dir_helper_intn,
          offsetof(struct rdf_section_switch_data, reserved_intn), 0 }
    };

    data.reserved_intn = NULL;
    data.type = 0xffff;

    vp = yasm_vps_first(valparams);
    sectname = yasm_vp_string(vp);
    if (!sectname)
        return NULL;
    vp = yasm_vps_next(vp);

    if (strcmp(sectname, ".text") == 0)
        data.type = RDF_SECT_CODE;
    else if (strcmp(sectname, ".data") == 0)
        data.type = RDF_SECT_DATA;
    else if (strcmp(sectname, ".bss") == 0)
        data.type = RDF_SECT_BSS;

    flags_override = yasm_dir_helper(object, vp, line, help, NELEMS(help),
                                     &data, rdf_helper_set_reserved);
    if (flags_override < 0)
        return NULL;    /* error occurred */

    if (data.type == 0xffff) {
        yasm_error_set(YASM_ERROR_VALUE,
                       N_("new segment declared without type code"));
        data.type = RDF_SECT_DATA;
    }

    if (data.reserved_intn) {
        reserved = yasm_intnum_get_uint(data.reserved_intn);
        yasm_intnum_destroy(data.reserved_intn);
    }

    retval = yasm_object_get_general(object, sectname, 0, 1,
                                     data.type == RDF_SECT_BSS, &isnew, line);

    rsd = yasm_section_get_data(retval, &rdf_section_data_cb);

    if (isnew || yasm_section_is_default(retval)) {
        yasm_section_set_default(retval, 0);
        rsd->type = data.type;
        rsd->reserved = reserved;
    } else if (flags_override)
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("section flags ignored on section redeclaration"));
    return retval;
}

static /*@observer@*/ /*@null@*/ yasm_symrec *
rdf_objfmt_get_special_sym(yasm_object *object, const char *name,
                           const char *parser)
{
    return NULL;
}

static void
rdf_section_data_destroy(void *data)
{
    rdf_section_data *rsd = (rdf_section_data *)data;
    if (rsd->raw_data)
        yasm_xfree(rsd->raw_data);
    yasm_xfree(data);
}

static void
rdf_section_data_print(void *data, FILE *f, int indent_level)
{
    rdf_section_data *rsd = (rdf_section_data *)data;

    fprintf(f, "%*ssym=\n", indent_level, "");
    yasm_symrec_print(rsd->sym, f, indent_level+1);
    fprintf(f, "%*sscnum=%ld\n", indent_level, "", rsd->scnum);
    fprintf(f, "%*stype=0x%x\n", indent_level, "", rsd->type);
    fprintf(f, "%*sreserved=0x%x\n", indent_level, "", rsd->reserved);
    fprintf(f, "%*ssize=%ld\n", indent_level, "", rsd->size);
}

static void
rdf_symrec_data_destroy(void *data)
{
    yasm_xfree(data);
}

static void
rdf_symrec_data_print(void *data, FILE *f, int indent_level)
{
    rdf_symrec_data *rsymd = (rdf_symrec_data *)data;

    fprintf(f, "%*ssymtab segment=%u\n", indent_level, "", rsymd->segment);
}

static void
rdf_objfmt_add_libmodule(yasm_object *object, char *name, int lib)
{
    yasm_objfmt_rdf *objfmt_rdf = (yasm_objfmt_rdf *)object->objfmt;
    xdf_str *str;

    /* Add to list */
    str = yasm_xmalloc(sizeof(xdf_str));
    str->str = name;
    if (lib)
        STAILQ_INSERT_TAIL(&objfmt_rdf->library_names, str, link);
    else
        STAILQ_INSERT_TAIL(&objfmt_rdf->module_names, str, link);

    if (strlen(str->str) > MODLIB_NAME_MAX-1) {
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("name too long, truncating to %d bytes"),
                      MODLIB_NAME_MAX);
        str->str[MODLIB_NAME_MAX-1] = '\0';
    }
}

static void
dir_library(yasm_object *object, yasm_valparamhead *valparams,
            yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    rdf_objfmt_add_libmodule(object, yasm__xstrdup(yasm_vp_string(vp)), 1);
}

static void
dir_module(yasm_object *object, yasm_valparamhead *valparams,
           yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp = yasm_vps_first(valparams);
    rdf_objfmt_add_libmodule(object, yasm__xstrdup(yasm_vp_string(vp)), 0);
}

/* Define valid debug formats to use with this object format */
static const char *rdf_objfmt_dbgfmt_keywords[] = {
    "null",
    NULL
};

static const yasm_directive rdf_objfmt_directives[] = {
    { "library",        "nasm", dir_library,    YASM_DIR_ARG_REQUIRED },
    { "module",         "nasm", dir_module,     YASM_DIR_ARG_REQUIRED },
    { NULL, NULL, NULL, 0 }
};

static const char *rdf_nasm_stdmac[] = {
    "%imacro library 1+.nolist",
    "[library %1]",
    "%endmacro",
    "%imacro module 1+.nolist",
    "[module %1]",
    "%endmacro",
    NULL
};

static const yasm_stdmac rdf_objfmt_stdmacs[] = {
    { "nasm", "nasm", rdf_nasm_stdmac },
    { NULL, NULL, NULL }
};

/* Define objfmt structure -- see objfmt.h for details */
yasm_objfmt_module yasm_rdf_LTX_objfmt = {
    "Relocatable Dynamic Object File Format (RDOFF) v2.0",
    "rdf",
    "rdf",
    32,
    0,
    rdf_objfmt_dbgfmt_keywords,
    "null",
    rdf_objfmt_directives,
    rdf_objfmt_stdmacs,
    rdf_objfmt_create,
    rdf_objfmt_output,
    rdf_objfmt_destroy,
    rdf_objfmt_add_default_section,
    rdf_objfmt_init_new_section,
    rdf_objfmt_section_switch,
    rdf_objfmt_get_special_sym
};
