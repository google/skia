/**
 * \file libyasm/bytecode.h
 * \brief YASM bytecode interface.
 *
 * \license
 *  Copyright (C) 2001-2007  Peter Johnson
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
#ifndef YASM_BYTECODE_H
#define YASM_BYTECODE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** A data value (opaque type). */
typedef struct yasm_dataval yasm_dataval;
/** A list of data values. */
typedef struct yasm_datavalhead yasm_datavalhead;

/** Linked list of data values. */
/*@reldef@*/ STAILQ_HEAD(yasm_datavalhead, yasm_dataval);

/** Add a dependent span for a bytecode.
 * \param add_span_data add_span_data passed into bc_calc_len()
 * \param bc            bytecode containing span
 * \param id            non-zero identifier for span; may be any non-zero value
 *                      if <0, expand is called for any change;
 *                      if >0, expand is only called when exceeds threshold
 * \param value         dependent value for bytecode expansion
 * \param neg_thres     negative threshold for long/short decision
 * \param pos_thres     positive threshold for long/short decision
 */
typedef void (*yasm_bc_add_span_func)
    (void *add_span_data, yasm_bytecode *bc, int id, const yasm_value *value,
     long neg_thres, long pos_thres);

/** Bytecode callback structure.  Any implementation of a specific bytecode
 * must implement these functions and this callback structure.  The bytecode
 * implementation-specific data is stored in #yasm_bytecode.contents.
 */
typedef struct yasm_bytecode_callback {
    /** Destroys the implementation-specific data.
     * Called from yasm_bc_destroy().
     * \param contents  #yasm_bytecode.contents
     */
    void (*destroy) (/*@only@*/ void *contents);

    /** Prints the implementation-specific data (for debugging purposes).
     * Called from yasm_bc_print().
     * \param contents      #yasm_bytecode.contents
     * \param f             file
     * \param indent_level  indentation level
     */
    void (*print) (const void *contents, FILE *f, int indent_level);

    /** Finalizes the bytecode after parsing.  Called from yasm_bc_finalize().
     * A generic fill-in for this is yasm_bc_finalize_common().
     * \param bc            bytecode
     * \param prev_bc       bytecode directly preceding bc
     */
    void (*finalize) (yasm_bytecode *bc, yasm_bytecode *prev_bc);

    /** Return elements size of a data bytecode.
     * This function should return the size of each elements of a data
     * bytecode, for proper dereference of symbols attached to it.
     * \param bc            bytecode
     * \return 0 if element size is unknown.
     */
    int (*elem_size) (yasm_bytecode *bc);

    /** Calculates the minimum size of a bytecode.
     * Called from yasm_bc_calc_len().
     * A generic fill-in for this is yasm_bc_calc_len_common(), but as this
     * function internal errors when called, be very careful when using it!
     * This function should simply add to bc->len and not set it directly
     * (it's initialized by yasm_bc_calc_len() prior to passing control to
     * this function).
     *
     * \param bc            bytecode
     * \param add_span      function to call to add a span
     * \param add_span_data extra data to be passed to add_span function
     * \return 0 if no error occurred, nonzero if there was an error
     *         recognized (and output) during execution.
     * \note May store to bytecode updated expressions.
     */
    int (*calc_len) (yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                     void *add_span_data);

    /** Recalculates the bytecode's length based on an expanded span length.
     * Called from yasm_bc_expand().
     * A generic fill-in for this is yasm_bc_expand_common(), but as this
     * function internal errors when called, if used, ensure that calc_len()
     * never adds a span.
     * This function should simply add to bc->len to increase the length by
     * a delta amount.
     * \param bc            bytecode
     * \param span          span ID (as given to add_span in calc_len)
     * \param old_val       previous span value
     * \param new_val       new span value
     * \param neg_thres     negative threshold for long/short decision
     *                      (returned)
     * \param pos_thres     positive threshold for long/short decision
     *                      (returned)
     * \return 0 if bc no longer dependent on this span's length, negative if
     *         there was an error recognized (and output) during execution,
     *         and positive if bc size may increase for this span further
     *         based on the new negative and positive thresholds returned.
     * \note May store to bytecode updated expressions.
     */
    int (*expand) (yasm_bytecode *bc, int span, long old_val, long new_val,
                   /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres);

    /** Convert a bytecode into its byte representation.
     * Called from yasm_bc_tobytes().
     * A generic fill-in for this is yasm_bc_tobytes_common(), but as this
     * function internal errors when called, be very careful when using it!
     * \param bc            bytecode
     * \param bufp          byte representation destination buffer;
     *                      should be incremented as it's written to,
     *                      so that on return its delta from the
     *                      passed-in buf matches the bytecode length
     *                      (it's okay not to do this if an error
     *                      indication is returned)
     * \param bufstart      For calculating the correct offset parameter for
     *                      the \a output_value calls: *bufp - bufstart.
     * \param d             data to pass to each call to
     *                      output_value/output_reloc
     * \param output_value  function to call to convert values into their byte
     *                      representation
     * \param output_reloc  function to call to output relocation entries
     *                      for a single sym
     * \return Nonzero on error, 0 on success.
     * \note May result in non-reversible changes to the bytecode, but it's
     *       preferable if calling this function twice would result in the
     *       same output.
     */
    int (*tobytes) (yasm_bytecode *bc, unsigned char **bufp,
                    unsigned char *bufstart, void *d,
                    yasm_output_value_func output_value,
                    /*@null@*/ yasm_output_reloc_func output_reloc);

    /** Special bytecode classifications.  Most bytecode types should use
     * #YASM_BC_SPECIAL_NONE.  Others cause special handling to kick in
     * in various parts of yasm.
     */
    enum yasm_bytecode_special_type {
        YASM_BC_SPECIAL_NONE = 0,

        /** Bytecode reserves space instead of outputting data. */
        YASM_BC_SPECIAL_RESERVE,

        /** Adjusts offset instead of calculating len. */
        YASM_BC_SPECIAL_OFFSET,

        /** Instruction bytecode. */
        YASM_BC_SPECIAL_INSN
    } special;
} yasm_bytecode_callback;

/** A bytecode. */
struct yasm_bytecode {
    /** Bytecodes are stored as a singly linked list, with tail insertion.
     * \see section.h (#yasm_section).
     */
    /*@reldef@*/ STAILQ_ENTRY(yasm_bytecode) link;

    /** The bytecode callback structure for this bytecode.  May be NULL
     * during partial initialization.
     */
    /*@null@*/ const yasm_bytecode_callback *callback;

    /** Pointer to section containing bytecode; NULL if not part of a
     * section.
     */
    /*@dependent@*/ /*@null@*/ yasm_section *section;

    /** Number of times bytecode is repeated.
     * NULL=1 (to save space in the common case).
     */
    /*@only@*/ /*@null@*/ yasm_expr *multiple;

    /** Total length of entire bytecode (not including multiple copies). */
    unsigned long len;

    /** Number of copies, integer version. */
    long mult_int;

    /** Line number where bytecode was defined. */
    unsigned long line;

    /** Offset of bytecode from beginning of its section.
     * 0-based, ~0UL (e.g. all 1 bits) if unknown.
     */
    unsigned long offset;

    /** Unique integer index of bytecode.  Used during optimization. */
    unsigned long bc_index;

    /** NULL-terminated array of labels that point to this bytecode (as the
     * bytecode previous to the label).  NULL if no labels point here.
     */
    /*@null@*/ yasm_symrec **symrecs;

    /** Implementation-specific data (type identified by callback). */
    void *contents;
};

/** Create a bytecode of any specified type.
 * \param callback      bytecode callback functions, if NULL, creates empty
 *                      bytecode (may not be resolved or output)
 * \param contents      type-specific data
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode of the specified type.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_common
    (/*@null@*/ const yasm_bytecode_callback *callback,
     /*@only@*/ /*@null@*/ void *contents, unsigned long line);

/** Transform a bytecode of any type into a different type.
 * \param bc            bytecode to transform
 * \param callback      new bytecode callback function
 * \param contents      new type-specific data
 */
YASM_LIB_DECL
void yasm_bc_transform(yasm_bytecode *bc,
                       const yasm_bytecode_callback *callback,
                       void *contents);

/** Common bytecode callback finalize function, for where no finalization
 * is ever required for this type of bytecode.
 */
YASM_LIB_DECL
void yasm_bc_finalize_common(yasm_bytecode *bc, yasm_bytecode *prev_bc);

/** Common bytecode callback calc_len function, for where the bytecode has
 * no calculatable length.  Causes an internal error if called.
 */
YASM_LIB_DECL
int yasm_bc_calc_len_common(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                            void *add_span_data);

/** Common bytecode callback expand function, for where the bytecode is
 * always short (calc_len never calls add_span).  Causes an internal
 * error if called.
 */
YASM_LIB_DECL
int yasm_bc_expand_common
    (yasm_bytecode *bc, int span, long old_val, long new_val,
     /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres);

/** Common bytecode callback tobytes function, for where the bytecode
 * cannot be converted to bytes.  Causes an internal error if called.
 */
YASM_LIB_DECL
int yasm_bc_tobytes_common
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/** Get the next bytecode in a linked list of bytecodes.
 * \param bc    bytecode
 * \return Next bytecode.
 */
#define yasm_bc__next(bc)               STAILQ_NEXT(bc, link)

/** Set multiple field of a bytecode.
 * A bytecode can be repeated a number of times when output.  This function
 * sets that multiple.
 * \param bc    bytecode
 * \param e     multiple (kept, do not free)
 */
YASM_LIB_DECL
void yasm_bc_set_multiple(yasm_bytecode *bc, /*@keep@*/ yasm_expr *e);

/** Create a bytecode containing data value(s).
 * \param datahead      list of data values (kept, do not free)
 * \param size          storage size (in bytes) for each data value
 * \param append_zero   append a single zero byte after each data value
 *                      (if non-zero)
 * \param arch          architecture (optional); if provided, data items
 *                      are directly simplified to bytes if possible
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_data
    (yasm_datavalhead *datahead, unsigned int size, int append_zero,
     /*@null@*/ yasm_arch *arch, unsigned long line);

/** Create a bytecode containing LEB128-encoded data value(s).
 * \param datahead      list of data values (kept, do not free)
 * \param sign          signedness (1=signed, 0=unsigned) of each data value
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_leb128
    (yasm_datavalhead *datahead, int sign, unsigned long line);

/** Create a bytecode reserving space.
 * \param numitems      number of reserve "items" (kept, do not free)
 * \param itemsize      reserved size (in bytes) for each item
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_reserve
    (/*@only@*/ yasm_expr *numitems, unsigned int itemsize,
     unsigned long line);

/** Get the number of items and itemsize for a reserve bytecode.  If bc
 * is not a reserve bytecode, returns NULL.
 * \param bc            bytecode
 * \param itemsize      reserved size (in bytes) for each item (returned)
 * \return NULL if bc is not a reserve bytecode, otherwise an expression
 *         for the number of items to reserve.
 */
YASM_LIB_DECL
/*@null@*/ const yasm_expr *yasm_bc_reserve_numitems
    (yasm_bytecode *bc, /*@out@*/ unsigned int *itemsize);

/** Create a bytecode that includes a binary file verbatim.
 * \param filename      path to binary file (kept, do not free)
 * \param start         starting location in file (in bytes) to read data from
 *                      (kept, do not free); may be NULL to indicate 0
 * \param maxlen        maximum number of bytes to read from the file (kept, do
 *                      do not free); may be NULL to indicate no maximum
 * \param linemap       line mapping repository
 * \param line          virtual line (from yasm_linemap) for the bytecode
 * \return Newly allocated bytecode.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_incbin
    (/*@only@*/ char *filename, /*@only@*/ /*@null@*/ yasm_expr *start,
     /*@only@*/ /*@null@*/ yasm_expr *maxlen, yasm_linemap *linemap,
     unsigned long line);

/** Create a bytecode that aligns the following bytecode to a boundary.
 * \param boundary      byte alignment (must be a power of two)
 * \param fill          fill data (if NULL, code_fill or 0 is used)
 * \param maxskip       maximum number of bytes to skip
 * \param code_fill     code fill data (if NULL, 0 is used)
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 * \note The precedence on generated fill is as follows:
 *       - from fill parameter (if not NULL)
 *       - from code_fill parameter (if not NULL)
 *       - 0
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_align
    (/*@keep@*/ yasm_expr *boundary, /*@keep@*/ /*@null@*/ yasm_expr *fill,
     /*@keep@*/ /*@null@*/ yasm_expr *maxskip,
     /*@null@*/ const unsigned char **code_fill, unsigned long line);

/** Create a bytecode that puts the following bytecode at a fixed section
 * offset.
 * \param start         section offset of following bytecode
 * \param fill          fill value
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 */
YASM_LIB_DECL
/*@only@*/ yasm_bytecode *yasm_bc_create_org
    (unsigned long start, unsigned long fill, unsigned long line);

/** Get the section that contains a particular bytecode.
 * \param bc    bytecode
 * \return Section containing bc (can be NULL if bytecode is not part of a
 *         section).
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ yasm_section *yasm_bc_get_section
    (yasm_bytecode *bc);

/** Add to the list of symrecs that reference a bytecode.  For symrec use
 * only.
 * \param bc    bytecode
 * \param sym   symbol
 */
YASM_LIB_DECL
void yasm_bc__add_symrec(yasm_bytecode *bc, /*@dependent@*/ yasm_symrec *sym);

/** Delete (free allocated memory for) a bytecode.
 * \param bc    bytecode (only pointer to it); may be NULL
 */
YASM_LIB_DECL
void yasm_bc_destroy(/*@only@*/ /*@null@*/ yasm_bytecode *bc);

/** Print a bytecode.  For debugging purposes.
 * \param f             file
 * \param indent_level  indentation level
 * \param bc            bytecode
 */
YASM_LIB_DECL
void yasm_bc_print(const yasm_bytecode *bc, FILE *f, int indent_level);

/** Finalize a bytecode after parsing.
 * \param bc            bytecode
 * \param prev_bc       bytecode directly preceding bc in a list of bytecodes
 */
YASM_LIB_DECL
void yasm_bc_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);

/** Determine the distance between the starting offsets of two bytecodes.
 * \param precbc1       preceding bytecode to the first bytecode
 * \param precbc2       preceding bytecode to the second bytecode
 * \return Distance in bytes between the two bytecodes (bc2-bc1), or NULL if
 *         the distance was indeterminate.
 * \warning Only valid /after/ optimization.
 */
YASM_LIB_DECL
/*@null@*/ /*@only@*/ yasm_intnum *yasm_calc_bc_dist
    (yasm_bytecode *precbc1, yasm_bytecode *precbc2);

/** Get the offset of the next bytecode (the next bytecode doesn't have to
 * actually exist).
 * \param precbc        preceding bytecode
 * \return Offset of the next bytecode in bytes.
 * \warning Only valid /after/ optimization.
 */
YASM_LIB_DECL
unsigned long yasm_bc_next_offset(yasm_bytecode *precbc);

/** Return elemens size of a data bytecode.
 * Returns the size of each elements of a data bytecode, for proper dereference
 * of symbols attached to it.
 * \param bc            bytecode
 * \return 0 if element size is unknown
 */
YASM_LIB_DECL
int yasm_bc_elem_size(yasm_bytecode *bc);

/** Resolve EQUs in a bytecode and calculate its minimum size.
 * Generates dependent bytecode spans for cases where, if the length spanned
 * increases, it could cause the bytecode size to increase.
 * Any bytecode multiple is NOT included in the length or spans generation;
 * this must be handled at a higher level.
 * \param bc            bytecode
 * \param add_span      function to call to add a span
 * \param add_span_data extra data to be passed to add_span function
 * \return 0 if no error occurred, nonzero if there was an error recognized
 *         (and output) during execution.
 * \note May store to bytecode updated expressions and the short length.
 */
YASM_LIB_DECL
int yasm_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                     void *add_span_data);

/** Recalculate a bytecode's length based on an expanded span length.
 * \param bc            bytecode
 * \param span          span ID (as given to yasm_bc_add_span_func in
 *                      yasm_bc_calc_len)
 * \param old_val       previous span value
 * \param new_val       new span value
 * \param neg_thres     negative threshold for long/short decision (returned)
 * \param pos_thres     positive threshold for long/short decision (returned)
 * \return 0 if bc no longer dependent on this span's length, negative if
 *         there was an error recognized (and output) during execution, and
 *         positive if bc size may increase for this span further based on the
 *         new negative and positive thresholds returned.
 * \note May store to bytecode updated expressions and the updated length.
 */
YASM_LIB_DECL
int yasm_bc_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                   /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres);

/** Convert a bytecode into its byte representation.
 * \param bc            bytecode
 * \param buf           byte representation destination buffer
 * \param bufsize       size of buf (in bytes) prior to call; size of the
 *                      generated data after call
 * \param gap           if nonzero, indicates the data does not really need to
 *                      exist in the object file; if nonzero, contents of buf
 *                      are undefined [output]
 * \param d             data to pass to each call to output_value/output_reloc
 * \param output_value  function to call to convert values into their byte
 *                      representation
 * \param output_reloc  function to call to output relocation entries
 *                      for a single sym
 * \return Newly allocated buffer that should be used instead of buf for
 *         reading the byte representation, or NULL if buf was big enough to
 *         hold the entire byte representation.
 * \note Calling twice on the same bytecode may \em not produce the same
 *       results on the second call, as calling this function may result in
 *       non-reversible changes to the bytecode.
 */
YASM_LIB_DECL
/*@null@*/ /*@only@*/ unsigned char *yasm_bc_tobytes
    (yasm_bytecode *bc, unsigned char *buf, unsigned long *bufsize,
     /*@out@*/ int *gap, void *d, yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc)
    /*@sets *buf@*/;

/** Get the bytecode multiple value as an integer.
 * \param bc            bytecode
 * \param multiple      multiple value (output)
 * \param calc_bc_dist  nonzero if distances between bytecodes should be
 *                      calculated, 0 if error should be returned in this case
 * \return 1 on error (set with yasm_error_set), 0 on success.
 */
YASM_LIB_DECL
int yasm_bc_get_multiple(yasm_bytecode *bc, /*@out@*/ long *multiple,
                         int calc_bc_dist);

/** Get the bytecode multiple value as an expression.
 * \param bc            bytecode
 * \return Bytecode multiple, NULL if =1.
 */
YASM_LIB_DECL
const yasm_expr *yasm_bc_get_multiple_expr(const yasm_bytecode *bc);

/** Get a #yasm_insn structure from an instruction bytecode (if possible).
 * \param bc            bytecode
 * \return Instruction details if bytecode is an instruction bytecode,
 *         otherwise NULL.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ yasm_insn *yasm_bc_get_insn(yasm_bytecode *bc);

/** Create a new data value from an expression.
 * \param expn  expression
 * \return Newly allocated data value.
 */
YASM_LIB_DECL
yasm_dataval *yasm_dv_create_expr(/*@keep@*/ yasm_expr *expn);

/** Create a new data value from a string.
 * \param contents      string (may contain NULs)
 * \param len           length of string
 * \return Newly allocated data value.
 */
yasm_dataval *yasm_dv_create_string(/*@keep@*/ char *contents, size_t len);

/** Create a new data value from raw bytes data.
 * \param contents      raw data (may contain NULs)
 * \param len           length
 * \return Newly allocated data value.
 */
YASM_LIB_DECL
yasm_dataval *yasm_dv_create_raw(/*@keep@*/ unsigned char *contents,
                                 unsigned long len);

/** Create a new uninitialized data value.
 * \return Newly allocated data value.
 */
yasm_dataval *yasm_dv_create_reserve(void);

#ifndef YASM_DOXYGEN
#define yasm_dv_create_string(s, l) yasm_dv_create_raw((unsigned char *)(s), \
                                                       (unsigned long)(l))
#endif

/** Get the underlying value of a data value.
 * \param dv    data value
 * \return Value, or null if non-value (e.g. string or raw).
 */
yasm_value *yasm_dv_get_value(yasm_dataval *dv);

/** Set multiple field of a data value.
 * A data value can be repeated a number of times when output.  This function
 * sets that multiple.
 * \param dv    data value
 * \param e     multiple (kept, do not free)
 */
void yasm_dv_set_multiple(yasm_dataval *dv, /*@keep@*/ yasm_expr *e);

/** Get the data value multiple value as an unsigned long integer.
 * \param dv            data value
 * \param multiple      multiple value (output)
 * \return 1 on error (set with yasm_error_set), 0 on success.
 */
int yasm_dv_get_multiple(yasm_dataval *dv, /*@out@*/ unsigned long *multiple);

/** Initialize a list of data values.
 * \param headp list of data values
 */
void yasm_dvs_initialize(yasm_datavalhead *headp);
#ifndef YASM_DOXYGEN
#define yasm_dvs_initialize(headp)      STAILQ_INIT(headp)
#endif

/** Delete (free allocated memory for) a list of data values.
 * \param headp list of data values
 */
YASM_LIB_DECL
void yasm_dvs_delete(yasm_datavalhead *headp);

/** Add data value to the end of a list of data values.
 * \note Does not make a copy of the data value; so don't pass this function
 *       static or local variables, and discard the dv pointer after calling
 *       this function.
 * \param headp         data value list
 * \param dv            data value (may be NULL)
 * \return If data value was actually appended (it wasn't NULL), the data
 *         value; otherwise NULL.
 */
YASM_LIB_DECL
/*@null@*/ yasm_dataval *yasm_dvs_append
    (yasm_datavalhead *headp, /*@returned@*/ /*@null@*/ yasm_dataval *dv);

/** Print a data value list.  For debugging purposes.
 * \param f             file
 * \param indent_level  indentation level
 * \param headp         data value list
 */
YASM_LIB_DECL
void yasm_dvs_print(const yasm_datavalhead *headp, FILE *f, int indent_level);

#endif
