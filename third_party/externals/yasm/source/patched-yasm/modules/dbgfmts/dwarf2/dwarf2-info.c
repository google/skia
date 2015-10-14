/*
 * DWARF2 debugging format - info and abbreviation tables
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

#include "dwarf2-dbgfmt.h"

#define DW_LANG_Mips_Assembler  0x8001

/* Tag encodings */
typedef enum {
    DW_TAG_padding = 0x00,
    DW_TAG_array_type = 0x01,
    DW_TAG_class_type = 0x02,
    DW_TAG_entry_point = 0x03,
    DW_TAG_enumeration_type = 0x04,
    DW_TAG_formal_parameter = 0x05,
    DW_TAG_imported_declaration = 0x08,
    DW_TAG_label = 0x0a,
    DW_TAG_lexical_block = 0x0b,
    DW_TAG_member = 0x0d,
    DW_TAG_pointer_type = 0x0f,
    DW_TAG_reference_type = 0x10,
    DW_TAG_compile_unit = 0x11,
    DW_TAG_string_type = 0x12,
    DW_TAG_structure_type = 0x13,
    DW_TAG_subroutine_type = 0x15,
    DW_TAG_typedef = 0x16,
    DW_TAG_union_type = 0x17,
    DW_TAG_unspecified_parameters = 0x18,
    DW_TAG_variant = 0x19,
    DW_TAG_common_block = 0x1a,
    DW_TAG_common_inclusion = 0x1b,
    DW_TAG_inheritance = 0x1c,
    DW_TAG_inlined_subroutine = 0x1d,
    DW_TAG_module = 0x1e,
    DW_TAG_ptr_to_member_type = 0x1f,
    DW_TAG_set_type = 0x20,
    DW_TAG_subrange_type = 0x21,
    DW_TAG_with_stmt = 0x22,
    DW_TAG_access_declaration = 0x23,
    DW_TAG_base_type = 0x24,
    DW_TAG_catch_block = 0x25,
    DW_TAG_const_type = 0x26,
    DW_TAG_constant = 0x27,
    DW_TAG_enumerator = 0x28,
    DW_TAG_file_type = 0x29,
    DW_TAG_friend = 0x2a,
    DW_TAG_namelist = 0x2b,
    DW_TAG_namelist_item = 0x2c,
    DW_TAG_packed_type = 0x2d,
    DW_TAG_subprogram = 0x2e,
    DW_TAG_template_type_param = 0x2f,
    DW_TAG_template_value_param = 0x30,
    DW_TAG_thrown_type = 0x31,
    DW_TAG_try_block = 0x32,
    DW_TAG_variant_part = 0x33,
    DW_TAG_variable = 0x34,
    DW_TAG_volatile_type = 0x35
} dwarf_tag;

/* Attribute form encodings */
typedef enum {
    DW_FORM_addr = 0x01,
    DW_FORM_block2 = 0x03,
    DW_FORM_block4 = 0x04,
    DW_FORM_data2 = 0x05,
    DW_FORM_data4 = 0x06,
    DW_FORM_data8 = 0x07,
    DW_FORM_string = 0x08,
    DW_FORM_block = 0x09,
    DW_FORM_block1 = 0x0a,
    DW_FORM_data1 = 0x0b,
    DW_FORM_flag = 0x0c,
    DW_FORM_sdata = 0x0d,
    DW_FORM_strp = 0x0e,
    DW_FORM_udata = 0x0f,
    DW_FORM_ref_addr = 0x10,
    DW_FORM_ref1 = 0x11,
    DW_FORM_ref2 = 0x12,
    DW_FORM_ref4 = 0x13,
    DW_FORM_ref8 = 0x14,
    DW_FORM_ref_udata = 0x15,
    DW_FORM_indirect = 0x16
} dwarf_form;

/* Attribute encodings */
typedef enum {
    DW_AT_sibling = 0x01,
    DW_AT_location = 0x02,
    DW_AT_name = 0x03,
    DW_AT_ordering = 0x09,
    DW_AT_subscr_data = 0x0a,
    DW_AT_byte_size = 0x0b,
    DW_AT_bit_offset = 0x0c,
    DW_AT_bit_size = 0x0d,
    DW_AT_element_list = 0x0f,
    DW_AT_stmt_list = 0x10,
    DW_AT_low_pc = 0x11,
    DW_AT_high_pc = 0x12,
    DW_AT_language = 0x13,
    DW_AT_member = 0x14,
    DW_AT_discr = 0x15,
    DW_AT_discr_value = 0x16,
    DW_AT_visibility = 0x17,
    DW_AT_import = 0x18,
    DW_AT_string_length = 0x19,
    DW_AT_common_reference = 0x1a,
    DW_AT_comp_dir = 0x1b,
    DW_AT_const_value = 0x1c,
    DW_AT_containing_type = 0x1d,
    DW_AT_default_value = 0x1e,
    DW_AT_inline = 0x20,
    DW_AT_is_optional = 0x21,
    DW_AT_lower_bound = 0x22,
    DW_AT_producer = 0x25,
    DW_AT_prototyped = 0x27,
    DW_AT_return_addr = 0x2a,
    DW_AT_start_scope = 0x2c,
    DW_AT_stride_size = 0x2e,
    DW_AT_upper_bound = 0x2f,
    DW_AT_abstract_origin = 0x31,
    DW_AT_accessibility = 0x32,
    DW_AT_address_class = 0x33,
    DW_AT_artificial = 0x34,
    DW_AT_base_types = 0x35,
    DW_AT_calling_convention = 0x36,
    DW_AT_count = 0x37,
    DW_AT_data_member_location = 0x38,
    DW_AT_decl_column = 0x39,
    DW_AT_decl_file = 0x3a,
    DW_AT_decl_line = 0x3b,
    DW_AT_declaration = 0x3c,
    DW_AT_discr_list = 0x3d,
    DW_AT_encoding = 0x3e,
    DW_AT_external = 0x3f,
    DW_AT_frame_base = 0x40,
    DW_AT_friend = 0x41,
    DW_AT_identifier_case = 0x42,
    DW_AT_macro_info = 0x43,
    DW_AT_namelist_items = 0x44,
    DW_AT_priority = 0x45,
    DW_AT_segment = 0x46,
    DW_AT_specification = 0x47,
    DW_AT_static_link = 0x48,
    DW_AT_type = 0x49,
    DW_AT_use_location = 0x4a,
    DW_AT_variable_parameter = 0x4b,
    DW_AT_virtuality = 0x4c,
    DW_AT_vtable_elem_location = 0x4d
} dwarf_attribute;

typedef struct dwarf2_abbrev_attr {
    STAILQ_ENTRY(dwarf2_abbrev_attr) link;
    dwarf_attribute name;
    dwarf_form form;
} dwarf2_abbrev_attr;

typedef struct dwarf2_abbrev {
    unsigned long id;
    dwarf_tag tag;
    int has_children;
    STAILQ_HEAD(dwarf2_abbrev_attrhead, dwarf2_abbrev_attr) attrs;
} dwarf2_abbrev;

/* Bytecode callback function prototypes */

static void dwarf2_abbrev_bc_destroy(void *contents);
static void dwarf2_abbrev_bc_print(const void *contents, FILE *f,
                                   int indent_level);
static int dwarf2_abbrev_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int dwarf2_abbrev_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */

static const yasm_bytecode_callback dwarf2_abbrev_bc_callback = {
    dwarf2_abbrev_bc_destroy,
    dwarf2_abbrev_bc_print,
    yasm_bc_finalize_common,
    NULL,
    dwarf2_abbrev_bc_calc_len,
    yasm_bc_expand_common,
    dwarf2_abbrev_bc_tobytes,
    0
};


static unsigned long
dwarf2_add_abbrev_attr(dwarf2_abbrev *abbrev, dwarf_attribute name,
                       dwarf_form form)
{
    dwarf2_abbrev_attr *attr = yasm_xmalloc(sizeof(dwarf2_abbrev_attr));
    attr->name = name;
    attr->form = form;
    STAILQ_INSERT_TAIL(&abbrev->attrs, attr, link);
    return yasm_size_uleb128(name) + yasm_size_uleb128(form);
}

static void
dwarf2_append_expr(yasm_section *sect, /*@only@*/ yasm_expr *expr,
                   unsigned int size, int leb)
{
    yasm_datavalhead dvs;
    yasm_bytecode *bc;

    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_expr(expr));
    if (leb == 0)
        bc = yasm_bc_create_data(&dvs, size, 0, NULL, 0);
    else
        bc = yasm_bc_create_leb128(&dvs, leb<0, 0);
    yasm_bc_finalize(bc, yasm_dwarf2__append_bc(sect, bc));
    yasm_bc_calc_len(bc, NULL, NULL);
}

static void
dwarf2_append_str(yasm_section *sect, const char *str)
{
    yasm_datavalhead dvs;
    yasm_bytecode *bc;

    yasm_dvs_initialize(&dvs);
    yasm_dvs_append(&dvs, yasm_dv_create_string(yasm__xstrdup(str),
                                                strlen(str)));
    bc = yasm_bc_create_data(&dvs, 1, 1, NULL, 0);
    yasm_bc_finalize(bc, yasm_dwarf2__append_bc(sect, bc));
    yasm_bc_calc_len(bc, NULL, NULL);
}

yasm_section *
yasm_dwarf2__generate_info(yasm_object *object, yasm_section *debug_line,
                           yasm_section *main_code)
{
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = (yasm_dbgfmt_dwarf2 *)object->dbgfmt;
    int new;
    yasm_bytecode *abc;
    dwarf2_abbrev *abbrev;
    dwarf2_head *head;
    char *buf;
    yasm_section *debug_abbrev =
        yasm_object_get_general(object, ".debug_abbrev", 4, 0, 0, &new, 0);
    yasm_section *debug_info =
        yasm_object_get_general(object, ".debug_info", 4, 0, 0, &new, 0);

    yasm_section_set_align(debug_abbrev, 0, 0);
    yasm_section_set_align(debug_info, 0, 0);

    /* Create abbreviation table entry for compilation unit */
    abbrev = yasm_xmalloc(sizeof(dwarf2_abbrev));
    abc = yasm_bc_create_common(&dwarf2_abbrev_bc_callback, abbrev, 0);
    abbrev->id = 1;
    abbrev->tag = DW_TAG_compile_unit;
    abbrev->has_children = 0;
    abc->len = yasm_size_uleb128(abbrev->id) + yasm_size_uleb128(abbrev->tag)
        + 3;
    STAILQ_INIT(&abbrev->attrs);
    yasm_dwarf2__append_bc(debug_abbrev, abc);

    /* info header */
    head = yasm_dwarf2__add_head(dbgfmt_dwarf2, debug_info, debug_abbrev, 1, 0);

    /* Generate abbreviations at the same time as info (since they're linked
     * and we're only generating one piece of info).
     */

    /* generating info using abbrev 1 */
    dwarf2_append_expr(debug_info,
        yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(1)), 0),
        0, 1);

    /* statement list (line numbers) */
    abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_stmt_list, DW_FORM_data4);
    dwarf2_append_expr(debug_info,
        yasm_expr_create_ident(yasm_expr_sym(
            yasm_dwarf2__bc_sym(object->symtab,
                                yasm_section_bcs_first(debug_line))), 0),
        dbgfmt_dwarf2->sizeof_offset, 0);

    if (main_code) {
        yasm_symrec *first;
        first = yasm_dwarf2__bc_sym(object->symtab,
                                    yasm_section_bcs_first(main_code));
        /* All code is contiguous in one section */
        abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_low_pc, DW_FORM_addr);
        dwarf2_append_expr(debug_info,
            yasm_expr_create_ident(yasm_expr_sym(first), 0),
            dbgfmt_dwarf2->sizeof_address, 0);

        abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_high_pc, DW_FORM_addr);
        dwarf2_append_expr(debug_info,
            yasm_expr_create(YASM_EXPR_ADD, yasm_expr_sym(first),
                yasm_expr_int(yasm_calc_bc_dist(
                    yasm_section_bcs_first(main_code),
                    yasm_section_bcs_last(main_code))), 0),
            dbgfmt_dwarf2->sizeof_address, 0);
    }

    /* input filename */
    abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_name, DW_FORM_string);
    dwarf2_append_str(debug_info, object->src_filename);

    /* compile directory (current working directory) */
    abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_comp_dir, DW_FORM_string);
    buf = yasm__getcwd();
    dwarf2_append_str(debug_info, buf);
    yasm_xfree(buf);

    /* producer - assembler name */
    abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_producer, DW_FORM_string);
    if (getenv("YASM_TEST_SUITE"))
        dwarf2_append_str(debug_info, "yasm HEAD");
    else
        dwarf2_append_str(debug_info, PACKAGE_STRING);

    /* language - no standard code for assembler, use MIPS as a substitute */
    abc->len += dwarf2_add_abbrev_attr(abbrev, DW_AT_language, DW_FORM_data2);
    dwarf2_append_expr(debug_info,
        yasm_expr_create_ident(yasm_expr_int(
            yasm_intnum_create_uint(DW_LANG_Mips_Assembler)), 0), 2, 0);

    /* Terminate list of abbreviations */
    abbrev = yasm_xmalloc(sizeof(dwarf2_abbrev));
    abc = yasm_bc_create_common(&dwarf2_abbrev_bc_callback, abbrev, 0);
    abbrev->id = 0;
    abbrev->tag = 0;
    abbrev->has_children = 0;
    STAILQ_INIT(&abbrev->attrs);
    abc->len = 1;
    yasm_dwarf2__append_bc(debug_abbrev, abc);

    /* mark end of info */
    yasm_dwarf2__set_head_end(head, yasm_section_bcs_last(debug_info));

    return debug_info;
}

static void
dwarf2_abbrev_bc_destroy(void *contents)
{
    dwarf2_abbrev *abbrev = (dwarf2_abbrev *)contents;
    dwarf2_abbrev_attr *n1, *n2;

    /* Delete attributes */
    n1 = STAILQ_FIRST(&abbrev->attrs);
    while (n1) {
        n2 = STAILQ_NEXT(n1, link);
        yasm_xfree(n1);
        n1 = n2;
    }

    yasm_xfree(contents);
}

static void
dwarf2_abbrev_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
dwarf2_abbrev_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                          void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a dwarf2 aranges head bytecode"));
    /*@notreached@*/
    return 0;
}

static int
dwarf2_abbrev_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                         unsigned char *bufstart, void *d,
                         yasm_output_value_func output_value,
                         yasm_output_reloc_func output_reloc)
{
    dwarf2_abbrev *abbrev = (dwarf2_abbrev *)bc->contents;
    unsigned char *buf = *bufp;
    dwarf2_abbrev_attr *attr;

    if (abbrev->id == 0) {
        YASM_WRITE_8(buf, 0);
        *bufp = buf;
        return 0;
    }

    buf += yasm_get_uleb128(abbrev->id, buf);
    buf += yasm_get_uleb128(abbrev->tag, buf);
    YASM_WRITE_8(buf, abbrev->has_children);

    STAILQ_FOREACH(attr, &abbrev->attrs, link) {
        buf += yasm_get_uleb128(attr->name, buf);
        buf += yasm_get_uleb128(attr->form, buf);
    }

    YASM_WRITE_8(buf, 0);
    YASM_WRITE_8(buf, 0);

    *bufp = buf;
    return 0;
}

