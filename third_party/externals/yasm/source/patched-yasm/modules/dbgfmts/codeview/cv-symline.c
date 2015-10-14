/*
 * CodeView debugging format - symbol and line information
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

#include "cv-dbgfmt.h"

enum cv8_symheadtype {
    CV8_DEBUG_SYMS      = 0xF1, /* CV5 symbol information */
    CV8_LINE_NUMS       = 0xF2, /* line numbers for a section */
    CV8_FILE_STRTAB     = 0xF3, /* filename string table */
    CV8_FILE_INFO       = 0xF4  /* source file info */
};

enum cv_symtype {
    /* Non-modal Symbols */
    CV_S_COMPILE        = 0x0001,       /* Compile Flag */
    CV_S_REGISTER       = 0x0002,       /* Register */
    CV_S_CONSTANT       = 0x0003,       /* Constant */
    CV_S_UDT            = 0x0004,       /* User-defined Type */
    CV_S_SSEARCH        = 0x0005,       /* Start Search */
    CV_S_END            = 0x0006,       /* End of Block */
    CV_S_SKIP           = 0x0007,       /* Skip Record */
    CV_S_OBJNAME        = 0x0009,       /* Object File Name */
    CV_S_ENDARG         = 0x000a,       /* End of Arguments */
    CV_S_COBOLUDT       = 0x000b,       /* COBOL User-defined Type */
    CV_S_MANYREG        = 0x000c,       /* Many Registers */
    CV_S_RETURN         = 0x000d,       /* Function Return */
    CV_S_ENTRYTHIS      = 0x000e,       /* "this" at Method Entry */

    /* Symbols for 16:16 Segmented Architectures */
    CV_S_BPREL16        = 0x0100,       /* BP Relative 16:16 */
    CV_S_LDATA16        = 0x0101,       /* Local Data 16:16 */
    CV_S_GDATA16        = 0x0102,       /* Global Data Symbol 16:16 */
    CV_S_PUB16          = 0x0103,       /* Public Symbol 16:16 */
    CV_S_LPROC16        = 0x0104,       /* Local Start 16:16 */
    CV_S_GPROC16        = 0x0105,       /* Global Start 16:16 */
    CV_S_THUNK16        = 0x0106,       /* Thunk Start 16:16 */
    CV_S_BLOCK16        = 0x0107,       /* Block Start 16:16 */
    CV_S_WITH16         = 0x0108,       /* With Start 16:16 */
    CV_S_LABEL16        = 0x0109,       /* Code Label 16:16 */
    CV_S_CEXMODEL16     = 0x0110,       /* Change Execution Model 16:16 */
    CV_S_VFTPATH16      = 0x010b,       /* Virtual Function Table Path 16:16 */
    CV_S_REGREL16       = 0x010c,       /* Register Relative 16:16 */

    /* Symbols for 16:32 Segmented Architectures */
    CV_S_BPREL32        = 0x0200,       /* BP Relative 16:32 */
    CV_S_LDATA32        = 0x0201,       /* Local Data 16:32 */
    CV_S_GDATA32        = 0x0202,       /* Global Data Symbol 16:32 */
    CV_S_PUB32          = 0x0203,       /* Public Symbol 16:32 */
    CV_S_LPROC32        = 0x0204,       /* Local Start 16:32 */
    CV_S_GPROC32        = 0x0205,       /* Global Start 16:32 */
    CV_S_THUNK32        = 0x0206,       /* Thunk Start 16:32 */
    CV_S_BLOCK32        = 0x0207,       /* Block Start 16:32 */
    CV_S_WITH32         = 0x0208,       /* With Start 16:32 */
    CV_S_LABEL32        = 0x0209,       /* Code Label 16:32 */
    CV_S_CEXMODEL32     = 0x0210,       /* Change Execution Model 16:32 */
    CV_S_VFTPATH32      = 0x020b,       /* Virtual Function Table Path 16:32 */
    CV_S_REGREL32       = 0x020c,       /* Register Relative 16:32 */
    CV_S_LTHREAD32      = 0x020d,       /* Local Thread Storage 16:32 */
    CV_S_GTHREAD32      = 0x020e,       /* Global Thread Storage 16:32 */

    /* Symbols for MIPS */
    CV_S_LPROCMIPS      = 0x0300,       /* Local procedure start MIPS */
    CV_S_GPROCMIPS      = 0x0301,       /* Global procedure start MIPS */

    /* Symbols for CV8 - strings are 0 terminated rather than length-prefix.
     * Incomplete and unofficial.
     */
    CV8_S_OBJNAME       = 0x1101,       /* Object File Name */
    CV8_S_LABEL32       = 0x1105,       /* Code Label 16:32 */
    CV8_S_LDATA32       = 0x110c,       /* Local Data 16:32 */
    CV8_S_GDATA32       = 0x110d,       /* Global Data 16:32 */
    CV8_S_LPROC32       = 0x1110,       /* Local Start 16:32 */
    CV8_S_COMPILE       = 0x1116        /* Compile Flag */
};

typedef struct cv8_symhead {
    enum cv8_symheadtype type;
    yasm_bytecode *start_prevbc;
    yasm_bytecode *end_prevbc;
    int first;      /* nonzero if first symhead in section */
} cv8_symhead;

typedef struct cv8_fileinfo {
    const cv_filename *fn;
} cv8_fileinfo;

/* Note: each line number group is associated with a file AND a section */
typedef struct cv8_linepair {
    unsigned long offset;
    unsigned long line;
} cv8_linepair;

/* Decrease linked list overhead a bit doing it this way */
typedef struct cv8_lineset {
    STAILQ_ENTRY(cv8_lineset) link;
    cv8_linepair pairs[126];
    size_t num_pairs;
} cv8_lineset;

typedef struct cv8_lineinfo {
    STAILQ_ENTRY(cv8_lineinfo) link;
    const cv_filename *fn;      /* filename associated with line numbers */
    yasm_section *sect;         /* section line numbers are for */
    yasm_symrec *sectsym;       /* symbol for beginning of sect */
    unsigned long num_linenums;
    STAILQ_HEAD(cv8_lineset_head, cv8_lineset) linesets;
} cv8_lineinfo;

/* Symbols use a bit of meta-programming to encode formats: each character
 * of format represents the output generated, as follows:
 * 'b' : 1 byte value (integer)
 * 'h' : 2 byte value (integer)
 * 'w' : 4 byte value (integer)
 * 'Y' : symrec SECREL+SECTION (pointer)
 * 'T' : type index (integer)
 * 'S' : length-prefixed string (pointer)
 * 'Z' : 0-terminated string (pointer)
 */
typedef struct cv_sym {
    enum cv_symtype type;
    const char *format;
    union {
        unsigned long i;
        void *p;
    } args[10];
} cv_sym;

/* Bytecode callback function prototypes */
static void cv8_symhead_bc_destroy(void *contents);
static void cv8_symhead_bc_print(const void *contents, FILE *f,
                                 int indent_level);
static int cv8_symhead_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int cv8_symhead_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

static void cv8_fileinfo_bc_destroy(void *contents);
static void cv8_fileinfo_bc_print(const void *contents, FILE *f,
                                  int indent_level);
static int cv8_fileinfo_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int cv8_fileinfo_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

static void cv8_lineinfo_bc_destroy(void *contents);
static void cv8_lineinfo_bc_print(const void *contents, FILE *f,
                                  int indent_level);
static int cv8_lineinfo_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int cv8_lineinfo_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

static void cv_sym_bc_destroy(void *contents);
static void cv_sym_bc_print(const void *contents, FILE *f, int indent_level);
static int cv_sym_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int cv_sym_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */
static const yasm_bytecode_callback cv8_symhead_bc_callback = {
    cv8_symhead_bc_destroy,
    cv8_symhead_bc_print,
    yasm_bc_finalize_common,
    NULL,
    cv8_symhead_bc_calc_len,
    yasm_bc_expand_common,
    cv8_symhead_bc_tobytes,
    0
};

static const yasm_bytecode_callback cv8_fileinfo_bc_callback = {
    cv8_fileinfo_bc_destroy,
    cv8_fileinfo_bc_print,
    yasm_bc_finalize_common,
    NULL,
    cv8_fileinfo_bc_calc_len,
    yasm_bc_expand_common,
    cv8_fileinfo_bc_tobytes,
    0
};

static const yasm_bytecode_callback cv8_lineinfo_bc_callback = {
    cv8_lineinfo_bc_destroy,
    cv8_lineinfo_bc_print,
    yasm_bc_finalize_common,
    NULL,
    cv8_lineinfo_bc_calc_len,
    yasm_bc_expand_common,
    cv8_lineinfo_bc_tobytes,
    0
};

static const yasm_bytecode_callback cv_sym_bc_callback = {
    cv_sym_bc_destroy,
    cv_sym_bc_print,
    yasm_bc_finalize_common,
    NULL,
    cv_sym_bc_calc_len,
    yasm_bc_expand_common,
    cv_sym_bc_tobytes,
    0
};

static cv8_symhead *cv8_add_symhead(yasm_section *sect, unsigned long type,
                                    int first);
static void cv8_set_symhead_end(cv8_symhead *head, yasm_bytecode *end_prevbc);

static yasm_bytecode *cv8_add_fileinfo
    (yasm_section *sect, const cv_filename *fn);

static unsigned long cv_sym_size(const cv_sym *cvs);


static cv_sym *
cv8_add_sym_objname(yasm_section *sect, /*@keep@*/ char *objname)
{
    yasm_bytecode *bc;
    cv_sym *cvs = yasm_xmalloc(sizeof(cv_sym));
    cvs->type = CV8_S_OBJNAME;
    cvs->format = "wZ";
    cvs->args[0].i = 0;         /* signature (0=asm) */
    cvs->args[1].p = objname;   /* object filename */

    bc = yasm_bc_create_common(&cv_sym_bc_callback, cvs, 0);
    bc->len = cv_sym_size(cvs);
    yasm_cv__append_bc(sect, bc);
    return cvs;
}

static cv_sym *
cv8_add_sym_compile(yasm_object *object, yasm_section *sect,
                    /*@keep@*/ char *creator)
{
    yasm_bytecode *bc;
    cv_sym *cvs = yasm_xmalloc(sizeof(cv_sym));
    cvs->type = CV8_S_COMPILE;
    cvs->format = "wwwwZh";
    cvs->args[0].i = 3;         /* language (3=Masm) */

    /* target processor; 0xD0 = AMD64 */
    if (strcmp(yasm_arch_keyword(object->arch), "x86") == 0) {
        if (strcmp(yasm_arch_get_machine(object->arch), "amd64") == 0)
            cvs->args[1].i = 0xD0;
        else
            cvs->args[1].i = 0x6;       /* 686, FIXME */
    } else
        cvs->args[1].i = 0;             /* XXX: unknown */

    cvs->args[2].i = 0;         /* flags (assume 0 for now) */
    cvs->args[3].i = 0;         /* creator version number (assume 0 for now) */
    cvs->args[4].p = creator;   /* creator string */
    cvs->args[5].i = 0;         /* no pairs of key/value */

    bc = yasm_bc_create_common(&cv_sym_bc_callback, cvs, 0);
    bc->len = cv_sym_size(cvs);
    yasm_cv__append_bc(sect, bc);
    return cvs;
}

static cv_sym *
cv8_add_sym_label(yasm_section *sect, yasm_symrec *sym)
{
    yasm_bytecode *bc;
    cv_sym *cvs = yasm_xmalloc(sizeof(cv_sym));
    cvs->type = CV8_S_LABEL32;
    cvs->format = "YbZ";
    cvs->args[0].p = sym;       /* symrec for label */
    cvs->args[1].i = 0;         /* flags (assume 0 for now) */
    cvs->args[2].p = yasm__xstrdup(yasm_symrec_get_name(sym));

    bc = yasm_bc_create_common(&cv_sym_bc_callback, cvs, 0);
    bc->len = cv_sym_size(cvs);
    yasm_cv__append_bc(sect, bc);
    return cvs;
}

static cv_sym *
cv8_add_sym_data(yasm_section *sect, unsigned long type, yasm_symrec *sym,
                 int is_global)
{
    yasm_bytecode *bc;
    cv_sym *cvs = yasm_xmalloc(sizeof(cv_sym));
    cvs->type = is_global ? CV8_S_GDATA32 : CV8_S_LDATA32;
    cvs->format = "wYZ";
    cvs->args[0].i = type;      /* type index */
    cvs->args[1].p = sym;       /* symrec for label */
    cvs->args[2].p = yasm__xstrdup(yasm_symrec_get_name(sym));

    bc = yasm_bc_create_common(&cv_sym_bc_callback, cvs, 0);
    bc->len = cv_sym_size(cvs);
    yasm_cv__append_bc(sect, bc);
    return cvs;
}

static size_t
cv_dbgfmt_add_file(yasm_dbgfmt_cv *dbgfmt_cv, size_t filenum,
                   const char *filename)
{
    char *pathname;
    size_t i;
    yasm_md5_context context;
    FILE *f;
    unsigned char *buf;
    size_t len;

    /* Put the filename into the filename table */
    if (filenum == 0) {
        /* Look to see if we already have that filename in the table */
        for (; filenum<dbgfmt_cv->filenames_size; filenum++) {
            if (!dbgfmt_cv->filenames[filenum].filename ||
                strcmp(dbgfmt_cv->filenames[filenum].filename, filename) == 0)
                break;
        }
    } else
        filenum--;      /* array index is 0-based */

    /* Realloc table if necessary */
    if (filenum >= dbgfmt_cv->filenames_allocated) {
        size_t old_allocated = dbgfmt_cv->filenames_allocated;
        dbgfmt_cv->filenames_allocated = filenum+32;
        dbgfmt_cv->filenames = yasm_xrealloc(dbgfmt_cv->filenames,
            sizeof(cv_filename)*dbgfmt_cv->filenames_allocated);
        for (i=old_allocated; i<dbgfmt_cv->filenames_allocated; i++) {
            dbgfmt_cv->filenames[i].pathname = NULL;
            dbgfmt_cv->filenames[i].filename = NULL;
            dbgfmt_cv->filenames[i].str_off = 0;
            dbgfmt_cv->filenames[i].info_off = 0;
        }
    }

    /* Calculate MD5 checksum of file */
    buf = yasm_xmalloc(1024);
    yasm_md5_init(&context);
    f = fopen(filename, "rb");
    if (!f)
        yasm__fatal(N_("codeview: could not open source file"));
    while ((len = fread(buf, 1, 1024, f)) > 0)
        yasm_md5_update(&context, buf, (unsigned long)len);
    yasm_md5_final(dbgfmt_cv->filenames[filenum].digest, &context);
    fclose(f);
    yasm_xfree(buf);

    /* Actually save in table */
    if (dbgfmt_cv->filenames[filenum].pathname)
        yasm_xfree(dbgfmt_cv->filenames[filenum].pathname);
    if (dbgfmt_cv->filenames[filenum].filename)
        yasm_xfree(dbgfmt_cv->filenames[filenum].filename);

    pathname = yasm__abspath(filename);
    dbgfmt_cv->filenames[filenum].pathname = pathname;
    dbgfmt_cv->filenames[filenum].filename = yasm__xstrdup(filename);

    /* Update table size */
    if (filenum >= dbgfmt_cv->filenames_size)
        dbgfmt_cv->filenames_size = filenum + 1;

    return filenum;
}

static yasm_bytecode *
cv_append_str(yasm_section *sect, const char *str)
{
    yasm_datavalhead dvs;
    yasm_bytecode *bc;

    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_string(yasm__xstrdup(str),
                                                strlen(str)));
    bc = yasm_bc_create_data(&dvs, 1, 1, NULL, 0);
    yasm_bc_finalize(bc, yasm_cv__append_bc(sect, bc));
    yasm_bc_calc_len(bc, NULL, NULL);
    return bc;
}

typedef struct cv_line_info {
    yasm_section *debug_symline;
    yasm_object *object;
    yasm_dbgfmt_cv *dbgfmt_cv;
    yasm_linemap *linemap;
    yasm_errwarns *errwarns;
    unsigned int num_lineinfos;
    STAILQ_HEAD(cv8_lineinfo_head, cv8_lineinfo) cv8_lineinfos;
    /*@null@*/ cv8_lineinfo *cv8_cur_li;
    /*@null@*/ cv8_lineset *cv8_cur_ls;
} cv_line_info;

static int
cv_generate_line_bc(yasm_bytecode *bc, /*@null@*/ void *d)
{
    cv_line_info *info = (cv_line_info *)d;
    yasm_dbgfmt_cv *dbgfmt_cv = info->dbgfmt_cv;
    size_t i;
    const char *filename;
    unsigned long line;
    /*@null@*/ yasm_bytecode *nextbc = yasm_bc__next(bc);
    yasm_section *sect = yasm_bc_get_section(bc);

    if (nextbc && bc->offset == nextbc->offset)
        return 0;

    yasm_linemap_lookup(info->linemap, bc->line, &filename, &line);

    if (!info->cv8_cur_li
        || strcmp(filename, info->cv8_cur_li->fn->filename) != 0) {
        yasm_bytecode *sectbc;
        char symname[8];

        /* first see if we already have a lineinfo that is for this section and
         * filename
         */
        STAILQ_FOREACH(info->cv8_cur_li, &info->cv8_lineinfos, link) {
            if (sect == info->cv8_cur_li->sect
                && strcmp(filename, info->cv8_cur_li->fn->filename) == 0)
                break;
        }

        if (info->cv8_cur_li) {
            info->cv8_cur_ls = STAILQ_LAST(&info->cv8_cur_li->linesets,
                                           cv8_lineset, link);
            goto done;          /* found one */
        }

        /* Nope; find file */
        for (i=0; i<dbgfmt_cv->filenames_size; i++) {
            if (strcmp(filename, dbgfmt_cv->filenames[i].filename) == 0)
                break;
        }
        if (i >= dbgfmt_cv->filenames_size)
            yasm_internal_error(N_("could not find filename in table"));

        /* and create new lineinfo structure */
        info->cv8_cur_li = yasm_xmalloc(sizeof(cv8_lineinfo));
        info->cv8_cur_li->fn = &dbgfmt_cv->filenames[i];
        info->cv8_cur_li->sect = sect;
        sectbc = yasm_section_bcs_first(sect);
        if (sectbc->symrecs && sectbc->symrecs[0])
            info->cv8_cur_li->sectsym = sectbc->symrecs[0];
        else {
            sprintf(symname, ".%06u", info->num_lineinfos++);
            info->cv8_cur_li->sectsym =
                yasm_symtab_define_label(info->object->symtab, symname, sectbc,
                                         1, 0);
        }
        info->cv8_cur_li->num_linenums = 0;
        STAILQ_INIT(&info->cv8_cur_li->linesets);
        STAILQ_INSERT_TAIL(&info->cv8_lineinfos, info->cv8_cur_li, link);
        info->cv8_cur_ls = NULL;
    }
done:

    /* build new lineset if necessary */
    if (!info->cv8_cur_ls || info->cv8_cur_ls->num_pairs >= 126) {
        info->cv8_cur_ls = yasm_xmalloc(sizeof(cv8_lineset));
        info->cv8_cur_ls->num_pairs = 0;
        STAILQ_INSERT_TAIL(&info->cv8_cur_li->linesets, info->cv8_cur_ls, link);
    }

    /* add linepair for this bytecode */
    info->cv8_cur_ls->pairs[info->cv8_cur_ls->num_pairs].offset = bc->offset;
    info->cv8_cur_ls->pairs[info->cv8_cur_ls->num_pairs].line =
        0x80000000 | line;
    info->cv8_cur_ls->num_pairs++;
    info->cv8_cur_li->num_linenums++;

    return 0;
}

static int
cv_generate_line_section(yasm_section *sect, /*@null@*/ void *d)
{
    cv_line_info *info = (cv_line_info *)d;

    if (!yasm_section_is_code(sect))
        return 0;       /* not code, so no line data for this section */

    info->cv8_cur_li = NULL;
    info->cv8_cur_ls = NULL;

    yasm_section_bcs_traverse(sect, info->errwarns, info, cv_generate_line_bc);

    return 0;
}

static int
cv_generate_filename(const char *filename, void *d)
{
    cv_dbgfmt_add_file((yasm_dbgfmt_cv *)d, 0, filename);
    return 0;
}

static int
cv_generate_sym(yasm_symrec *sym, void *d)
{
    cv_line_info *info = (cv_line_info *)d;
    yasm_bytecode *precbc;
    const char *name = yasm_symrec_get_name(sym);

    /* only care about labels (for now).  Don't put in symbols starting with
     * ".", as these are typically internally generated ones (like section
     * symbols).
     */
    if (name[0] == '.' || !yasm_symrec_get_label(sym, &precbc))
        return 0;

    /* TODO: add data types; until then, just mark everything as UBYTE */
    if (yasm_section_is_code(yasm_bc_get_section(precbc)))
        cv8_add_sym_label(info->debug_symline, sym);
    else
        cv8_add_sym_data(info->debug_symline, 0x20, sym,
            yasm_symrec_get_visibility(sym) & YASM_SYM_GLOBAL?1:0);
    return 0;
}

yasm_section *
yasm_cv__generate_symline(yasm_object *object, yasm_linemap *linemap,
                          yasm_errwarns *errwarns)
{
    yasm_dbgfmt_cv *dbgfmt_cv = (yasm_dbgfmt_cv *)object->dbgfmt;
    cv_line_info info;
    int new;
    size_t i;
    cv8_symhead *head;
    cv8_lineinfo *li;
    yasm_bytecode *bc;
    unsigned long off;

    /* Generate filenames based on linemap */
    yasm_linemap_traverse_filenames(linemap, dbgfmt_cv,
                                    cv_generate_filename);

    info.object = object;
    info.dbgfmt_cv = dbgfmt_cv;
    info.linemap = linemap;
    info.errwarns = errwarns;
    info.debug_symline =
        yasm_object_get_general(object, ".debug$S", 1, 0, 0, &new, 0);
    info.num_lineinfos = 0;
    STAILQ_INIT(&info.cv8_lineinfos);
    info.cv8_cur_li = NULL;
    info.cv8_cur_ls = NULL;

    /* source filenames string table */
    head = cv8_add_symhead(info.debug_symline, CV8_FILE_STRTAB, 1);
    cv_append_str(info.debug_symline, "");
    off = 1;
    for (i=0; i<dbgfmt_cv->filenames_size; i++) {
        if (!dbgfmt_cv->filenames[i].pathname) {
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("codeview file number %d unassigned"), i+1);
            yasm_errwarn_propagate(errwarns, 0);
            continue;
        }
        bc = cv_append_str(info.debug_symline,
                           dbgfmt_cv->filenames[i].pathname);
        dbgfmt_cv->filenames[i].str_off = off;
        off += bc->len;
    }
    cv8_set_symhead_end(head, yasm_section_bcs_last(info.debug_symline));

    /* Align 4 */
    bc = yasm_bc_create_align
        (yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(4)), 0),
         NULL, NULL, NULL, 0);
    yasm_bc_finalize(bc, yasm_cv__append_bc(info.debug_symline, bc));
    yasm_bc_calc_len(bc, NULL, NULL);

    /* source file info table */
    head = cv8_add_symhead(info.debug_symline, CV8_FILE_INFO, 0);
    off = 0;
    for (i=0; i<dbgfmt_cv->filenames_size; i++) {
        if (!dbgfmt_cv->filenames[i].pathname)
            continue;
        bc = cv8_add_fileinfo(info.debug_symline, &dbgfmt_cv->filenames[i]);
        dbgfmt_cv->filenames[i].info_off = off;
        off += bc->len;
    }
    cv8_set_symhead_end(head, yasm_section_bcs_last(info.debug_symline));

    /* Already aligned 4 */

    /* Generate line numbers for sections */
    yasm_object_sections_traverse(object, (void *)&info,
                                  cv_generate_line_section);

    /* Output line numbers for sections */
    STAILQ_FOREACH(li, &info.cv8_lineinfos, link) {
        head = cv8_add_symhead(info.debug_symline, CV8_LINE_NUMS, 0);
        bc = yasm_bc_create_common(&cv8_lineinfo_bc_callback, li, 0);
        bc->len = 24+li->num_linenums*8;
        yasm_cv__append_bc(info.debug_symline, bc);
        cv8_set_symhead_end(head, yasm_section_bcs_last(info.debug_symline));
    }

    /* Already aligned 4 */

    /* Output debugging symbols */
    head = cv8_add_symhead(info.debug_symline, CV8_DEBUG_SYMS, 0);
    /* add object and compile flag first */
    cv8_add_sym_objname(info.debug_symline,
                        yasm__abspath(object->obj_filename));
    if (getenv("YASM_TEST_SUITE"))
        cv8_add_sym_compile(object, info.debug_symline,
                            yasm__xstrdup("yasm HEAD"));
    else
        cv8_add_sym_compile(object, info.debug_symline,
                            yasm__xstrdup(PACKAGE_STRING));
    /* then iterate through symbol table */
    yasm_symtab_traverse(object->symtab, &info, cv_generate_sym);
    cv8_set_symhead_end(head, yasm_section_bcs_last(info.debug_symline));

    /* Align 4 at end */
    bc = yasm_bc_create_align
        (yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(4)), 0),
         NULL, NULL, NULL, 0);
    yasm_bc_finalize(bc, yasm_cv__append_bc(info.debug_symline, bc));
    yasm_bc_calc_len(bc, NULL, NULL);

    return info.debug_symline;
}

static void
cv_out_sym(yasm_symrec *sym, unsigned long off, yasm_bytecode *bc,
           unsigned char **bufp, void *d, yasm_output_value_func output_value)
{
    yasm_value val;

    /* sym in its section */
    yasm_value_init_sym(&val, sym, 32);
    val.section_rel = 1;
    output_value(&val, *bufp, 4, off, bc, 0, d);
    *bufp += 4;

    /* section index */
    yasm_value_init_sym(&val, sym, 16);
    val.seg_of = 1;
    output_value(&val, *bufp, 2, off+4, bc, 0, d);
    *bufp += 2;
}

static cv8_symhead *
cv8_add_symhead(yasm_section *sect, unsigned long type, int first)
{
    cv8_symhead *head;
    yasm_bytecode *bc;

    head = yasm_xmalloc(sizeof(cv8_symhead));
    head->type = type;
    head->first = first;
    head->start_prevbc = yasm_section_bcs_last(sect);

    bc = yasm_bc_create_common(&cv8_symhead_bc_callback, head, 0);
    if (first)
        bc->len = 12;
    else
        bc->len = 8;

    head->end_prevbc = bc;
    yasm_cv__append_bc(sect, bc);
    return head;
}

static void
cv8_set_symhead_end(cv8_symhead *head, yasm_bytecode *end_prevbc)
{
    head->end_prevbc = end_prevbc;
}

static void
cv8_symhead_bc_destroy(void *contents)
{
    yasm_xfree(contents);
}

static void
cv8_symhead_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
cv8_symhead_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                        void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a codeview symhead bytecode"));
    /*@notreached@*/
    return 0;
}

static int
cv8_symhead_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                       unsigned char *bufstart, void *d,
                       yasm_output_value_func output_value,
                       yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    cv8_symhead *head = (cv8_symhead *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_intnum *intn, *cval;

    cval = yasm_intnum_create_uint(4);
    /* Output "version" if first */
    if (head->first) {
        yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
        buf += 4;
    }

    /* Type contained - 4 bytes */
    yasm_intnum_set_uint(cval, head->type);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Total length of info (following this field) - 4 bytes */
    yasm_intnum_set_uint(cval, bc->len);
    intn = yasm_calc_bc_dist(head->start_prevbc, head->end_prevbc);
    yasm_intnum_calc(intn, YASM_EXPR_SUB, cval);
    yasm_arch_intnum_tobytes(object->arch, intn, buf, 4, 32, 0, bc, 0);
    buf += 4;
    yasm_intnum_destroy(intn);

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}

static yasm_bytecode *
cv8_add_fileinfo(yasm_section *sect, const cv_filename *fn)
{
    cv8_fileinfo *fi;
    yasm_bytecode *bc;

    fi = yasm_xmalloc(sizeof(cv8_fileinfo));
    fi->fn = fn;

    bc = yasm_bc_create_common(&cv8_fileinfo_bc_callback, fi, 0);
    bc->len = 24;

    yasm_cv__append_bc(sect, bc);
    return bc;
}

static void
cv8_fileinfo_bc_destroy(void *contents)
{
    yasm_xfree(contents);
}

static void
cv8_fileinfo_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
cv8_fileinfo_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                         void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a codeview fileinfo bytecode"));
    /*@notreached@*/
    return 0;
}

static int
cv8_fileinfo_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                        unsigned char *bufstart, void *d,
                        yasm_output_value_func output_value,
                        yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    cv8_fileinfo *fi = (cv8_fileinfo *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_intnum *cval;
    int i;

    /* Offset in filename string table */
    cval = yasm_intnum_create_uint(fi->fn->str_off);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Checksum type/length */
    yasm_intnum_set_uint(cval, 0x0110);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 2, 16, 0, bc, 0);
    buf += 2;

    /* Checksum */
    for (i=0; i<16; i++)
        YASM_WRITE_8(buf, fi->fn->digest[i]);

    /* Pad */
    YASM_WRITE_8(buf, 0);
    YASM_WRITE_8(buf, 0);

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}

static void
cv8_lineinfo_bc_destroy(void *contents)
{
    cv8_lineinfo *li = (cv8_lineinfo *)contents;
    cv8_lineset *ls1, *ls2;

    /* delete line sets */
    ls1 = STAILQ_FIRST(&li->linesets);
    while (ls1) {
        ls2 = STAILQ_NEXT(ls1, link);
        yasm_xfree(ls1);
        ls1 = ls2;
    }

    yasm_xfree(contents);
}

static void
cv8_lineinfo_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
cv8_lineinfo_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                         void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a codeview linehead bytecode"));
    /*@notreached@*/
    return 0;
}

static int
cv8_lineinfo_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                        unsigned char *bufstart, void *d,
                        yasm_output_value_func output_value,
                        yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    cv8_lineinfo *li = (cv8_lineinfo *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_intnum *cval;
    unsigned long i;
    cv8_lineset *ls;

    /* start offset and section */
    cv_out_sym(li->sectsym, (unsigned long)(buf - bufstart), bc, &buf,
               d, output_value);

    /* Two bytes of pad/alignment */
    YASM_WRITE_8(buf, 0);
    YASM_WRITE_8(buf, 0);

    /* Section length covered by line number info */
    cval = yasm_calc_bc_dist(yasm_section_bcs_first(li->sect),
                             yasm_section_bcs_last(li->sect));
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Offset of source file in info table */
    yasm_intnum_set_uint(cval, li->fn->info_off);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Number of line number pairs */
    yasm_intnum_set_uint(cval, li->num_linenums);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Number of bytes of line number pairs + 12 (no, I don't know why) */
    yasm_intnum_set_uint(cval, li->num_linenums*8+12);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
    buf += 4;

    /* Offset / line number pairs */
    i = 0;
    STAILQ_FOREACH(ls, &li->linesets, link) {
        unsigned long j;
        for (j=0; i<li->num_linenums && j<126; i++, j++) {
            /* offset in section */
            yasm_intnum_set_uint(cval, ls->pairs[j].offset);
            yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
            buf += 4;

            /* line number in file */
            yasm_intnum_set_uint(cval, ls->pairs[j].line);
            yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 0);
            buf += 4;
        }
    }

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}

static unsigned long
cv_sym_size(const cv_sym *cvs)
{
    const char *ch = cvs->format;
    unsigned long len = 4;  /* sym length and type */
    unsigned long slen;
    int arg = 0;

    while (*ch) {
        switch (*ch) {
            case 'b':
                len++;
                arg++;
                break;
            case 'h':
                len += 2;
                arg++;
                break;
            case 'w':
                len += 4;
                arg++;
                break;
            case 'Y':
                len += 6;       /* XXX: will be 4 in 16-bit version */
                arg++;
                break;
            case 'T':
                len += 4;       /* XXX: will be 2 in CV4 */
                arg++;
                break;
            case 'S':
                len += 1;       /* XXX: is this 1 or 2? */
                slen = (unsigned long)strlen((const char *)cvs->args[arg++].p);
                len += slen <= 0xff ? slen : 0xff;
                break;
            case 'Z':
                len +=
                    (unsigned long)strlen((const char *)cvs->args[arg++].p) + 1;
                break;
            default:
                yasm_internal_error(N_("unknown sym format character"));
        }
        ch++;
    }

    return len;
}

static void
cv_sym_bc_destroy(void *contents)
{
    cv_sym *cvs = (cv_sym *)contents;
    const char *ch = cvs->format;
    int arg = 0;

    while (*ch) {
        switch (*ch) {
            case 'b':
            case 'h':
            case 'w':
            case 'Y':
            case 'T':
                arg++;
                break;  /* nothing to destroy */
            case 'S':
            case 'Z':
                yasm_xfree(cvs->args[arg++].p);
                break;
            default:
                yasm_internal_error(N_("unknown sym format character"));
        }
        ch++;
    }

    yasm_xfree(contents);
}

static void
cv_sym_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
cv_sym_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                   void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a codeview sym bytecode"));
    /*@notreached@*/
    return 0;
}

static int
cv_sym_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                  unsigned char *bufstart, void *d,
                  yasm_output_value_func output_value,
                  yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    cv_sym *cvs = (cv_sym *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_intnum *cval;
    const char *ch = cvs->format;
    size_t len;
    int arg = 0;

    /* Total length of record (following this field) - 2 bytes */
    cval = yasm_intnum_create_uint(bc->len-2);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 2, 16, 0, bc, 1);
    buf += 2;

    /* Type contained - 2 bytes */
    yasm_intnum_set_uint(cval, cvs->type);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 2, 16, 0, bc, 0);
    buf += 2;

    while (*ch) {
        switch (*ch) {
            case 'b':
                YASM_WRITE_8(buf, cvs->args[arg].i);
                arg++;
                break;
            case 'h':
                yasm_intnum_set_uint(cval, cvs->args[arg++].i);
                yasm_arch_intnum_tobytes(object->arch, cval, buf, 2, 16, 0,
                                         bc, 0);
                buf += 2;
                break;
            case 'w':
                yasm_intnum_set_uint(cval, cvs->args[arg++].i);
                yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0,
                                         bc, 0);
                buf += 4;
                break;
            case 'Y':
                cv_out_sym((yasm_symrec *)cvs->args[arg++].p,
                           (unsigned long)(buf-bufstart), bc, &buf, d,
                           output_value);
                break;
            case 'T':
                yasm_intnum_set_uint(cval, cvs->args[arg++].i);
                yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0,
                                         bc, 0);
                buf += 4;       /* XXX: will be 2 in CV4 */
                break;
            case 'S':
                len = strlen((char *)cvs->args[arg].p);
                len = len <= 0xff ? len : 0xff;
                YASM_WRITE_8(buf, len);
                memcpy(buf, (char *)cvs->args[arg].p, len);
                buf += len;
                arg++;
                break;
            case 'Z':
                len = strlen((char *)cvs->args[arg].p)+1;
                memcpy(buf, (char *)cvs->args[arg].p, len);
                buf += len;
                arg++;
                break;
            default:
                yasm_internal_error(N_("unknown leaf format character"));
        }
        ch++;
    }

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}
