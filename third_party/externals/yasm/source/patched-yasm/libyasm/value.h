/**
 * \file libyasm/value.h
 * \brief YASM value interface.
 *
 * \license
 *  Copyright (C) 2006-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
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
 * \endlicense
 */
#ifndef YASM_VALUE_H
#define YASM_VALUE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Initialize a #yasm_value with just an expression.  No processing is
 * performed, the expression is simply stuck into value.abs and the other
 * fields are initialized.  Use yasm_expr_extract_value() to perform "smart"
 * processing into a #yasm_value.  This function is intended for use during
 * parsing simply to ensure all fields of the value are initialized; after
 * the parse is complete, yasm_value_extract() should be called to finalize
 * the value.  The value defaults to unsigned.
 * \param value     value to be initialized
 * \param e         expression (kept)
 * \param size      value size (in bits)
 */
YASM_LIB_DECL
void yasm_value_initialize(/*@out@*/ yasm_value *value,
                           /*@null@*/ /*@kept@*/ yasm_expr *e,
                           unsigned int size);

/** Initialize a #yasm_value with just a symrec.  No processing is performed,
 * the symrec is simply stuck into value.rel and the other fields are
 * initialized.
 * \param value     value to be initialized
 * \param sym       symrec
 * \param size      value size (in bits)
 */
YASM_LIB_DECL
void yasm_value_init_sym(/*@out@*/ yasm_value *value,
                         /*@null@*/ yasm_symrec *sym, unsigned int size);

/** Initialize a #yasm_value as a copy of another yasm_value.  Any expressions
 * within orig are copied, so it's safe to delete the copy.
 * \param value     value (copy to create)
 * \param orig      original value
 */
YASM_LIB_DECL
void yasm_value_init_copy(yasm_value *value, const yasm_value *orig);

/** Frees any memory inside value; does not free value itself.
 * \param value     value
 */
YASM_LIB_DECL
void yasm_value_delete(yasm_value *value);

/** Set a value to be relative to the current assembly position rather than
 * relative to the section start.
 * \param value     value
 * \param bc        bytecode containing value
 * \param ip_rel    if nonzero, indicates IP-relative data relocation,
 *                  sometimes used to generate special relocations
 * \note If value is just an absolute value, will get an absolute symrec to
 *       reference to (via bc's symbol table).
 */
YASM_LIB_DECL
void yasm_value_set_curpos_rel(yasm_value *value, yasm_bytecode *bc,
                               unsigned int ip_rel);

/** Perform yasm_value_finalize_expr() on a value that already exists from
 * being initialized with yasm_value_initialize().
 * \param value         value
 * \param precbc        previous bytecode to bytecode containing value
 * \return Nonzero if value could not be split.
 */
YASM_LIB_DECL
int yasm_value_finalize(yasm_value *value, /*@null@*/ yasm_bytecode *precbc);

/** Break a #yasm_expr into a #yasm_value constituent parts.  Extracts
 * the relative portion of the value, SEG and WRT portions, and top-level
 * right shift, if any.  Places the remaining expr into the absolute
 * portion of the value.  Essentially a combination of yasm_value_initialize()
 * and yasm_value_finalize().  First expands references to symrecs in
 * absolute sections by expanding with the absolute section start plus the
 * symrec offset within the absolute section.
 * \param value         value to store split portions into
 * \param e             expression input
 * \param precbc        previous bytecode to bytecode containing expression
 * \param size          value size (in bits)
 * \return Nonzero if the expr could not be split into a value for some
 *         reason (e.g. the relative portion was not added, but multiplied,
 *         etc).
 * \warning Do not use e after this call.  Even if an error is returned, e
 *          is stored into value.
 * \note This should only be called after the parse is complete.  Calling
 *       before the parse is complete will usually result in an error return.
 */
YASM_LIB_DECL
int yasm_value_finalize_expr(/*@out@*/ yasm_value *value,
                             /*@null@*/ /*@kept@*/ yasm_expr *e,
                             /*@null@*/ yasm_bytecode *precbc,
                             unsigned int size);

/** Get value if absolute or PC-relative section-local relative.  Returns NULL
 * otherwise.
 * \param value         value
 * \param bc            current bytecode (for PC-relative calculation); if
 *                      NULL, NULL is returned for PC-relative values.
 * \param calc_bc_dist  if nonzero, calculates bytecode distances in absolute
 *                      portion of value
 * \note Adds in value.rel (correctly) if PC-relative and in the same section
 *       as bc (and there is no WRT or SEG).
 * \return Intnum if can be resolved to integer value, otherwise NULL.
 */
YASM_LIB_DECL
/*@null@*/ /*@only@*/ yasm_intnum *yasm_value_get_intnum
    (yasm_value *value, /*@null@*/ yasm_bytecode *bc, int calc_bc_dist);

/** Output value if constant or PC-relative section-local.  This should be
 * used from objfmt yasm_output_value_func() functions.
 * functions.
 * \param value         value
 * \param buf           buffer for byte representation
 * \param destsize      destination size (in bytes)
 * \param bc            current bytecode (usually passed into higher-level
 *                      calling function)
 * \param warn          enables standard warnings: zero for none;
 *                      nonzero for overflow/underflow floating point and
 *                      integer warnings
 * \param arch          architecture
 * \note Adds in value.rel (correctly) if PC-relative and in the same section
 *       as bc (and there is no WRT or SEG); if this is not the desired
 *       behavior, e.g. a reloc is needed in this case, don't use this
 *       function!
 * \return 0 if no value output due to value needing relocation;
 *         1 if value output; -1 if error.
 */
YASM_LIB_DECL
int yasm_value_output_basic
    (yasm_value *value, /*@out@*/ unsigned char *buf, size_t destsize,
     yasm_bytecode *bc, int warn, yasm_arch *arch);

/** Print a value.  For debugging purposes.
 * \param value         value
 * \param indent_level  indentation level
 * \param f             file
 */
YASM_LIB_DECL
void yasm_value_print(const yasm_value *value, FILE *f, int indent_level);

#endif
