/*
 * NASM-style list format
 *
 *  Copyright (C) 2004-2007  Peter Johnson
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

/* NOTE: For this code to generate relocation information, the relocations
 * have to be added by the object format to each section in program source
 * order.
 *
 * This should not be an issue, as program source order == section bytecode
 * order, so unless the object formats are very obtuse with their bytecode
 * iteration, this should just happen.
 */

#define REGULAR_BUF_SIZE    1024

yasm_listfmt_module yasm_nasm_LTX_listfmt;

typedef struct sectreloc {
    /*@reldef@*/ SLIST_ENTRY(sectreloc) link;
    yasm_section *sect;
    /*@null@*/ yasm_reloc *next_reloc;  /* next relocation in section */
    unsigned long next_reloc_addr;
} sectreloc;

typedef struct bcreloc {
    /*@reldef@*/ STAILQ_ENTRY(bcreloc) link;
    unsigned long offset;       /* start of reloc from start of bytecode */
    size_t size;                /* size of reloc in bytes */
    int rel;                    /* PC/IP-relative or "absolute" */
} bcreloc;

typedef struct nasm_listfmt_output_info {
    yasm_arch *arch;
    /*@reldef@*/ STAILQ_HEAD(bcrelochead, bcreloc) bcrelocs;
    /*@null@*/ yasm_reloc *next_reloc;  /* next relocation in section */
    unsigned long next_reloc_addr;
} nasm_listfmt_output_info;


static /*@null@*/ /*@only@*/ yasm_listfmt *
nasm_listfmt_create(const char *in_filename, const char *obj_filename)
{
    yasm_listfmt_base *listfmt = yasm_xmalloc(sizeof(yasm_listfmt_base));
    listfmt->module = &yasm_nasm_LTX_listfmt;
    return (yasm_listfmt *)listfmt;
}

static void
nasm_listfmt_destroy(/*@only@*/ yasm_listfmt *listfmt)
{
    yasm_xfree(listfmt);
}

static int
nasm_listfmt_output_value(yasm_value *value, unsigned char *buf,
                          unsigned int destsize, unsigned long offset,
                          yasm_bytecode *bc, int warn, /*@null@*/ void *d)
{
    /*@null@*/ nasm_listfmt_output_info *info = (nasm_listfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ yasm_intnum *intn;
    unsigned int valsize = value->size;

    assert(info != NULL);

    /* Output */
    switch (yasm_value_output_basic(value, buf, destsize, bc, warn,
                                    info->arch)) {
        case -1:
            return 1;
        case 0:
            break;
        default:
            return 0;
    }

    /* Generate reloc if needed */
    if (info->next_reloc && info->next_reloc_addr == bc->offset+offset) {
        bcreloc *reloc = yasm_xmalloc(sizeof(bcreloc));
        reloc->offset = offset;
        reloc->size = destsize;
        reloc->rel = value->curpos_rel;
        STAILQ_INSERT_TAIL(&info->bcrelocs, reloc, link);

        /* Get next reloc's info */
        info->next_reloc = yasm_section_reloc_next(info->next_reloc);
        if (info->next_reloc) {
            yasm_intnum *addr;
            yasm_symrec *sym;
            yasm_reloc_get(info->next_reloc, &addr, &sym);
            info->next_reloc_addr = yasm_intnum_get_uint(addr);
        }
    }

    if (value->abs) {
        intn = yasm_expr_get_intnum(&value->abs, 0);
        if (intn)
            return yasm_arch_intnum_tobytes(info->arch, intn, buf, destsize,
                                            valsize, 0, bc, 0);
        else {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("relocation too complex"));
            return 1;
        }
    } else {
        int retval;
        intn = yasm_intnum_create_uint(0);
        retval = yasm_arch_intnum_tobytes(info->arch, intn, buf, destsize,
                                          valsize, 0, bc, 0);
        yasm_intnum_destroy(intn);
        return retval;
    }

    return 0;
}

static void
nasm_listfmt_output(yasm_listfmt *listfmt, FILE *f, yasm_linemap *linemap,
                    yasm_arch *arch)
{
    yasm_bytecode *bc;
    const char *source;
    unsigned long line = 1;
    unsigned long listline = 1;
    /*@only@*/ unsigned char *buf;
    nasm_listfmt_output_info info;
    /*@reldef@*/ SLIST_HEAD(sectrelochead, sectreloc) reloc_hist;
    /*@null@*/ sectreloc *last_hist = NULL;
    /*@null@*/ bcreloc *reloc = NULL;
    yasm_section *sect;

    SLIST_INIT(&reloc_hist);

    info.arch = arch;

    buf = yasm_xmalloc(REGULAR_BUF_SIZE);

    while (!yasm_linemap_get_source(linemap, line, &bc, &source)) {
        if (!bc) {
            fprintf(f, "%6lu %*s%s\n", listline++, 32, "", source);
        } else {
            /* get the next relocation for the bytecode's section */
            sect = yasm_bc_get_section(bc);
            if (!last_hist || last_hist->sect != sect) {
                int found = 0;

                /* look through reloc_hist for matching section */
                SLIST_FOREACH(last_hist, &reloc_hist, link) {
                    if (last_hist->sect == sect) {
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    /* not found, add to list*/
                    last_hist = yasm_xmalloc(sizeof(sectreloc));
                    last_hist->sect = sect;
                    last_hist->next_reloc = yasm_section_relocs_first(sect);

                    if (last_hist->next_reloc) {
                        yasm_intnum *addr;
                        yasm_symrec *sym;
                        yasm_reloc_get(last_hist->next_reloc, &addr, &sym);
                        last_hist->next_reloc_addr =
                            yasm_intnum_get_uint(addr);
                    }

                    SLIST_INSERT_HEAD(&reloc_hist, last_hist, link);
                }
            }

            info.next_reloc = last_hist->next_reloc;
            info.next_reloc_addr = last_hist->next_reloc_addr;
            STAILQ_INIT(&info.bcrelocs);

            /* loop over bytecodes on this line (usually only one) */
            while (bc && bc->line == line) {
                /*@null@*/ /*@only@*/ unsigned char *bigbuf;
                unsigned long size = REGULAR_BUF_SIZE;
                long multiple;
                unsigned long offset = bc->offset;
                unsigned char *origp, *p;
                int gap;

                /* convert bytecode into bytes, recording relocs along the
                 * way
                 */
                bigbuf = yasm_bc_tobytes(bc, buf, &size, &gap, &info,
                                         nasm_listfmt_output_value, NULL);
                yasm_bc_get_multiple(bc, &multiple, 1);
                if (multiple <= 0)
                    size = 0;
                else
                    size /= multiple;

                /* output bytes with reloc information */
                origp = bigbuf ? bigbuf : buf;
                p = origp;
                reloc = STAILQ_FIRST(&info.bcrelocs);
                if (gap) {
                    fprintf(f, "%6lu %08lX <gap>%*s%s\n", listline++, offset,
                            18, "", source ? source : "");
                } else while (size > 0) {
                    int i;

                    fprintf(f, "%6lu %08lX ", listline++, offset);
                    for (i=0; i<18 && size > 0; size--) {
                        if (reloc && (unsigned long)(p-origp) ==
                                     reloc->offset) {
                            fprintf(f, "%c", reloc->rel ? '(' : '[');
                            i++;
                        }
                        fprintf(f, "%02X", *(p++));
                        i+=2;
                        if (reloc && (unsigned long)(p-origp) ==
                                     reloc->offset+reloc->size) {
                            fprintf(f, "%c", reloc->rel ? ')' : ']');
                            i++;
                            reloc = STAILQ_NEXT(reloc, link);
                        }
                    }
                    if (size > 0)
                        fprintf(f, "-");
                    else {
                        if (multiple > 1) {
                            fprintf(f, "<rept>");
                            i += 6;
                        }
                        fprintf(f, "%*s", 18-i+1, "");
                    }
                    if (source) {
                        fprintf(f, "    %s", source);
                        source = NULL;
                    }
                    fprintf(f, "\n");
                }

                if (bigbuf)
                    yasm_xfree(bigbuf);
                bc = STAILQ_NEXT(bc, link);
            }

            /* delete bcrelocs (newly generated next bytecode if any) */
            reloc = STAILQ_FIRST(&info.bcrelocs);
            while (reloc) {
                bcreloc *reloc2 = STAILQ_NEXT(reloc, link);
                yasm_xfree(reloc);
                reloc = reloc2;
            }

            /* save reloc context */
            last_hist->next_reloc = info.next_reloc;
            last_hist->next_reloc_addr = info.next_reloc_addr;
        }
        line++;
    }

    /* delete reloc history */
    while (!SLIST_EMPTY(&reloc_hist)) {
        last_hist = SLIST_FIRST(&reloc_hist);
        SLIST_REMOVE_HEAD(&reloc_hist, link);
        yasm_xfree(last_hist);
    }

    yasm_xfree(buf);
}

/* Define listfmt structure -- see listfmt.h for details */
yasm_listfmt_module yasm_nasm_LTX_listfmt = {
    "NASM-style list format",
    "nasm",
    nasm_listfmt_create,
    nasm_listfmt_destroy,
    nasm_listfmt_output
};
