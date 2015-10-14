/**
 * \file libyasm/arch.h
 * \brief YASM architecture interface.
 *
 * \license
 *  Copyright (C) 2002-2007  Peter Johnson
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
#ifndef YASM_ARCH_H
#define YASM_ARCH_H

/** Errors that may be returned by yasm_arch_module::create(). */
typedef enum yasm_arch_create_error {
    YASM_ARCH_CREATE_OK = 0,            /**< No error. */
    YASM_ARCH_CREATE_BAD_MACHINE,       /**< Unrecognized machine name. */
    YASM_ARCH_CREATE_BAD_PARSER         /**< Unrecognized parser name. */
} yasm_arch_create_error;

/** Return values for yasm_arch_module::parse_check_insnprefix(). */
typedef enum yasm_arch_insnprefix {
    YASM_ARCH_NOTINSNPREFIX = 0,        /**< Unrecognized */
    YASM_ARCH_INSN,                     /**< An instruction */
    YASM_ARCH_PREFIX                    /**< An instruction prefix */
} yasm_arch_insnprefix;

/** Types of registers / target modifiers that may be returned by
 * yasm_arch_module::parse_check_regtmod().
 */
typedef enum yasm_arch_regtmod {
    YASM_ARCH_NOTREGTMOD = 0,           /**< Unrecognized */
    YASM_ARCH_REG,                      /**< A "normal" register */
    YASM_ARCH_REGGROUP,                 /**< A group of indexable registers */
    YASM_ARCH_SEGREG,                   /**< A segment register */
    YASM_ARCH_TARGETMOD                 /**< A target modifier (for jumps) */
} yasm_arch_regtmod;

#ifndef YASM_DOXYGEN
/** Base #yasm_arch structure.  Must be present as the first element in any
 * #yasm_arch implementation.
 */
typedef struct yasm_arch_base {
    /** #yasm_arch_module implementation for this architecture. */
    const struct yasm_arch_module *module;
} yasm_arch_base;
#endif

/** YASM machine subtype.  A number of different machine types may be
 * associated with a single architecture.  These may be specific CPU's, but
 * the ABI used to interface with the architecture should be the primary
 * differentiator between machines.  Some object formats (ELF) use the machine
 * to determine parameters within the generated output.
 */
typedef struct yasm_arch_machine {
    /** One-line description of the machine. */
    const char *name;

    /** Keyword used to select machine. */
    const char *keyword;
} yasm_arch_machine;

/** YASM architecture module interface.
 * \note All "data" in parser-related functions (yasm_arch_parse_*) needs to
 *       start the parse initialized to 0 to make it okay for a parser-related
 *       function to use/check previously stored data to see if it's been
 *       called before on the same piece of data.
 */
typedef struct yasm_arch_module {
    /** One-line description of the architecture.
     * Call yasm_arch_name() to get the name of a particular #yasm_arch.
     */
    const char *name;

    /** Keyword used to select architecture.
     * Call yasm_arch_keyword() to get the keyword of a particular #yasm_arch.
     */
    const char *keyword;

    /** NULL-terminated list of directives.  NULL if none. */
    /*@null@*/ const yasm_directive *directives;

    /** Create architecture.
     * Module-level implementation of yasm_arch_create().
     * Call yasm_arch_create() instead of calling this function.
     */
    /*@only@*/ yasm_arch * (*create) (const char *machine, const char *parser,
                                      /*@out@*/ yasm_arch_create_error *error);

    /** Module-level implementation of yasm_arch_destroy().
     * Call yasm_arch_destroy() instead of calling this function.
     */
    void (*destroy) (/*@only@*/ yasm_arch *arch);

    /** Module-level implementation of yasm_arch_get_machine().
     * Call yasm_arch_get_machine() instead of calling this function.
     */
    const char * (*get_machine) (const yasm_arch *arch);

    /** Module-level implementation of yasm_arch_get_address_size().
     * Call yasm_arch_get_address_size() instead of calling this function.
     */
    unsigned int (*get_address_size) (const yasm_arch *arch);

    /** Module-level implementation of yasm_arch_set_var().
     * Call yasm_arch_set_var() instead of calling this function.
     */
    int (*set_var) (yasm_arch *arch, const char *var, unsigned long val);

    /** Module-level implementation of yasm_arch_parse_check_insnprefix().
     * Call yasm_arch_parse_check_insnprefix() instead of calling this function.
     */
    yasm_arch_insnprefix (*parse_check_insnprefix)
        (yasm_arch *arch, const char *id, size_t id_len, unsigned long line,
         /*@out@*/ /*@only@*/ yasm_bytecode **bc, /*@out@*/ uintptr_t *prefix);

    /** Module-level implementation of yasm_arch_parse_check_regtmod().
     * Call yasm_arch_parse_check_regtmod() instead of calling this function.
     */
    yasm_arch_regtmod (*parse_check_regtmod)
        (yasm_arch *arch, const char *id, size_t id_len,
         /*@out@*/ uintptr_t *data);

    /** Module-level implementation of yasm_arch_get_fill().
     * Call yasm_arch_get_fill() instead of calling this function.
     */
    const unsigned char ** (*get_fill) (const yasm_arch *arch);

    /** Module-level implementation of yasm_arch_floatnum_tobytes().
     * Call yasm_arch_floatnum_tobytes() instead of calling this function.
     */
    int (*floatnum_tobytes) (yasm_arch *arch, const yasm_floatnum *flt,
                             unsigned char *buf, size_t destsize,
                             size_t valsize, size_t shift, int warn);

    /** Module-level implementation of yasm_arch_intnum_tobytes().
     * Call yasm_arch_intnum_tobytes() instead of calling this function.
     */
    int (*intnum_tobytes) (yasm_arch *arch, const yasm_intnum *intn,
                           unsigned char *buf, size_t destsize, size_t valsize,
                           int shift, const yasm_bytecode *bc,
                           int warn);

    /** Module-level implementation of yasm_arch_get_reg_size().
     * Call yasm_arch_get_reg_size() instead of calling this function.
     */
    unsigned int (*get_reg_size) (yasm_arch *arch, uintptr_t reg);

    /** Module-level implementation of yasm_arch_reggroup_get_reg().
     * Call yasm_arch_reggroup_get_reg() instead of calling this function.
     */
    uintptr_t (*reggroup_get_reg) (yasm_arch *arch, uintptr_t reggroup,
                                   unsigned long regindex);

    /** Module-level implementation of yasm_arch_reg_print().
     * Call yasm_arch_reg_print() instead of calling this function.
     */
    void (*reg_print) (yasm_arch *arch, uintptr_t reg, FILE *f);

    /** Module-level implementation of yasm_arch_segreg_print().
     * Call yasm_arch_segreg_print() instead of calling this function.
     */
    void (*segreg_print) (yasm_arch *arch, uintptr_t segreg, FILE *f);

    /** Module-level implementation of yasm_arch_ea_create().
     * Call yasm_arch_ea_create() instead of calling this function.
     */
    yasm_effaddr * (*ea_create) (yasm_arch *arch, /*@keep@*/ yasm_expr *e);

    /** Module-level implementation of yasm_arch_ea_destroy().
     * Call yasm_arch_ea_destroy() instead of calling this function.
     */
    void (*ea_destroy) (/*@only@*/ yasm_effaddr *ea);

    /** Module-level implementation of yasm_arch_ea_print().
     * Call yasm_arch_ea_print() instead of calling this function.
     */
    void (*ea_print) (const yasm_effaddr *ea, FILE *f, int indent_level);

    /** Module-level implementation of yasm_arch_create_empty_insn().
     * Call yasm_arch_create_empty_insn() instead of calling this function.
     */
    /*@only@*/ yasm_bytecode * (*create_empty_insn) (yasm_arch *arch,
                                                     unsigned long line);

    /** NULL-terminated list of machines for this architecture.
     * Call yasm_arch_get_machine() to get the active machine of a particular
     * #yasm_arch.
     */
    const yasm_arch_machine *machines;

    /** Default machine keyword.
     * Call yasm_arch_get_machine() to get the active machine of a particular
     * #yasm_arch.
     */
    const char *default_machine_keyword;

    /** Canonical "word" size in bits.
     * Call yasm_arch_wordsize() to get the word size of a particular
     * #yasm_arch.
     */
    unsigned int wordsize;

    /** Worst case minimum instruction length in bytes.
     * Call yasm_arch_min_insn_len() to get the minimum instruction length of
     * a particular #yasm_arch.
     */
    unsigned int min_insn_len;
} yasm_arch_module;

/** Get the one-line description of an architecture.
 * \param arch      architecture
 * \return One-line description of architecture.
 */
const char *yasm_arch_name(const yasm_arch *arch);

/** Get the keyword used to select an architecture.
 * \param arch      architecture
 * \return Architecture keyword.
 */
const char *yasm_arch_keyword(const yasm_arch *arch);

/** Get the word size of an architecture.
 * \param arch      architecture
 * \return Word size (in bits).
 */
unsigned int yasm_arch_wordsize(const yasm_arch *arch);

/** Get the minimum instruction length of an architecture.
 * \param arch      architecture
 * \return Minimum instruction length (in bytes).
 */
unsigned int yasm_arch_min_insn_len(const yasm_arch *arch);

/** Create architecture.
 * \param module        architecture module
 * \param machine       keyword of machine in use (must be one listed in
 *                      #yasm_arch_module.machines)
 * \param parser        keyword of parser in use
 * \param error         error return value
 * \return NULL on error (error returned in error parameter), otherwise new
 *         architecture.
 */
/*@only@*/ yasm_arch *yasm_arch_create(const yasm_arch_module *module,
                                       const char *machine, const char *parser,
                                       /*@out@*/ yasm_arch_create_error *error);

/** Clean up, free any architecture-allocated memory.
 * \param arch  architecture
 */
void yasm_arch_destroy(/*@only@*/ yasm_arch *arch);

/** Get architecture's active machine name.
 * \param arch  architecture
 * \return Active machine name.
 */
const char *yasm_arch_get_machine(const yasm_arch *arch);

/** Get architecture's active address size, in bits.
 * \param arch  architecture
 * \return Active address size (in bits).
 */
unsigned int yasm_arch_get_address_size(const yasm_arch *arch);

/** Set any arch-specific variables.  For example, "mode_bits" in x86.
 * \param arch  architecture
 * \param var   variable name
 * \param val   value to set
 * \return Zero on success, non-zero on failure (variable does not exist).
 */
int yasm_arch_set_var(yasm_arch *arch, const char *var, unsigned long val);

/** Check an generic identifier to see if it matches architecture specific
 * names for instructions or instruction prefixes.  Unrecognized identifiers
 * should return #YASM_ARCH_NOTINSNPREFIX so they can be treated as normal
 * symbols.  Any additional data beyond just the type (almost always necessary)
 * should be returned into the space provided by the data parameter.
 * \param arch          architecture
 * \param id            identifier as in the input file
 * \param id_len        length of id string
 * \param line          virtual line
 * \param bc            for instructions, yasm_insn-based bytecode is returned
 *                      (and NULL otherwise)
 * \param prefix        for prefixes, yasm_arch-specific value is returned
 *                      (and 0 otherwise)
 * \return Identifier type (#YASM_ARCH_NOTINSNPREFIX if unrecognized)
 */
yasm_arch_insnprefix yasm_arch_parse_check_insnprefix
    (yasm_arch *arch, const char *id, size_t id_len, unsigned long line,
     /*@out@*/ /*@only@*/ yasm_bytecode **bc, /*@out@*/ uintptr_t *prefix);

/** Check an generic identifier to see if it matches architecture specific
 * names for registers or target modifiers.  Unrecognized identifiers should
 * return #YASM_ARCH_NOTREGTMOD.  Any additional data beyond just the type
 * (almost always necessary) should be returned into the space provided by the
 * data parameter.
 * \param arch          architecture
 * \param id            identifier as in the input file
 * \param id_len        length of id string
 * \param data          extra identification information (yasm_arch-specific)
 *                      [output]
 * \return Identifier type (#YASM_ARCH_NOTREGTMOD if unrecognized)
 */
yasm_arch_regtmod yasm_arch_parse_check_regtmod
    (yasm_arch *arch, const char *id, size_t id_len,
     /*@out@*/ uintptr_t *data);

/** Get NOP fill patterns for 1-15 bytes of fill.
 * \param arch          architecture
 * \return 16-entry array of arrays; [0] is unused, [1] - [15] point to arrays
 * of 1-15 bytes (respectively) in length.
 */
const unsigned char **yasm_arch_get_fill(const yasm_arch *arch);

/** Output #yasm_floatnum to buffer.  Puts the value into the least
 * significant bits of the destination, or may be shifted into more
 * significant bits by the shift parameter.  The destination bits are
 * cleared before being set.
 * Architecture-specific because of endianness.
 * \param arch          architecture
 * \param flt           floating point value
 * \param buf           buffer to write into
 * \param destsize      destination size (in bytes)
 * \param valsize       size (in bits)
 * \param shift         left shift (in bits)
 * \param warn          enables standard overflow/underflow warnings
 * \return Nonzero on error.
 */
int yasm_arch_floatnum_tobytes(yasm_arch *arch, const yasm_floatnum *flt,
                               unsigned char *buf, size_t destsize,
                               size_t valsize, size_t shift, int warn);

/** Output #yasm_intnum to buffer.  Puts the value into the least
 * significant bits of the destination, or may be shifted into more
 * significant bits by the shift parameter.  The destination bits are
 * cleared before being set.
 * \param arch          architecture
 * \param intn          integer value
 * \param buf           buffer to write into
 * \param destsize      destination size (in bytes)
 * \param valsize       size (in bits)
 * \param shift         left shift (in bits); may be negative to specify right
 *                      shift (standard warnings include truncation to boundary)
 * \param bc            bytecode being output ("parent" of value)
 * \param warn          enables standard warnings (value doesn't fit into
 *                      valsize bits)
 * \return Nonzero on error.
 */
int yasm_arch_intnum_tobytes(yasm_arch *arch, const yasm_intnum *intn,
                             unsigned char *buf, size_t destsize,
                             size_t valsize, int shift,
                             const yasm_bytecode *bc, int warn);

/** Get the equivalent size of a register in bits.
 * \param arch  architecture
 * \param reg   register
 * \return 0 if there is no suitable equivalent size, otherwise the size.
 */
unsigned int yasm_arch_get_reg_size(yasm_arch *arch, uintptr_t reg);

/** Get a specific register of a register group, based on the register
 * group and the index within the group.
 * \param arch          architecture
 * \param reggroup      register group
 * \param regindex      register index
 * \return 0 if regindex is not valid for that register group, otherwise the
 *         specific register value.
 */
uintptr_t yasm_arch_reggroup_get_reg(yasm_arch *arch, uintptr_t reggroup,
                                     unsigned long regindex);

/** Print a register.  For debugging purposes.
 * \param arch          architecture
 * \param reg           register
 * \param f             file
 */
void yasm_arch_reg_print(yasm_arch *arch, uintptr_t reg, FILE *f);

/** Print a segment register.  For debugging purposes.
 * \param arch          architecture
 * \param segreg        segment register
 * \param f             file
 */
void yasm_arch_segreg_print(yasm_arch *arch, uintptr_t segreg, FILE *f);

/** Create an effective address from an expression.
 * \param arch  architecture
 * \param e     expression (kept, do not delete)
 * \return Newly allocated effective address.
 */
yasm_effaddr *yasm_arch_ea_create(yasm_arch *arch, /*@keep@*/ yasm_expr *e);

/** Delete (free allocated memory for) an effective address.
 * \param arch  architecture
 * \param ea    effective address (only pointer to it).
 */
void yasm_arch_ea_destroy(yasm_arch *arch, /*@only@*/ yasm_effaddr *ea);

/** Print an effective address.  For debugging purposes.
 * \param arch          architecture
 * \param ea            effective address
 * \param f             file
 * \param indent_level  indentation level
 */
void yasm_arch_ea_print(const yasm_arch *arch, const yasm_effaddr *ea,
                        FILE *f, int indent_level);

/** Create a bytecode that represents a single empty (0 length) instruction.
 * This is used for handling solitary prefixes.
 * \param arch          architecture
 * \param line          virtual line (from yasm_linemap)
 * \return Newly allocated bytecode.
 */
/*@only@*/ yasm_bytecode *yasm_arch_create_empty_insn(yasm_arch *arch,
                                                      unsigned long line);

#ifndef YASM_DOXYGEN

/* Inline macro implementations for arch functions */

#define yasm_arch_name(arch) \
    (((yasm_arch_base *)arch)->module->name)
#define yasm_arch_keyword(arch) \
    (((yasm_arch_base *)arch)->module->keyword)
#define yasm_arch_wordsize(arch) \
    (((yasm_arch_base *)arch)->module->wordsize)
#define yasm_arch_min_insn_len(arch) \
    (((yasm_arch_base *)arch)->module->min_insn_len)

#define yasm_arch_create(module, machine, parser, error) \
    module->create(machine, parser, error)

#define yasm_arch_destroy(arch) \
    ((yasm_arch_base *)arch)->module->destroy(arch)
#define yasm_arch_get_machine(arch) \
    ((yasm_arch_base *)arch)->module->get_machine(arch)
#define yasm_arch_get_address_size(arch) \
    ((yasm_arch_base *)arch)->module->get_address_size(arch)
#define yasm_arch_set_var(arch, var, val) \
    ((yasm_arch_base *)arch)->module->set_var(arch, var, val)
#define yasm_arch_parse_check_insnprefix(arch, id, id_len, line, bc, prefix) \
    ((yasm_arch_base *)arch)->module->parse_check_insnprefix \
        (arch, id, id_len, line, bc, prefix)
#define yasm_arch_parse_check_regtmod(arch, id, id_len, data) \
    ((yasm_arch_base *)arch)->module->parse_check_regtmod \
        (arch, id, id_len, data)
#define yasm_arch_get_fill(arch) \
    ((yasm_arch_base *)arch)->module->get_fill(arch)
#define yasm_arch_floatnum_tobytes(arch, flt, buf, destsize, valsize, shift, \
                                   warn) \
    ((yasm_arch_base *)arch)->module->floatnum_tobytes \
        (arch, flt, buf, destsize, valsize, shift, warn)
#define yasm_arch_intnum_tobytes(arch, intn, buf, destsize, valsize, shift, \
                                 bc, warn) \
    ((yasm_arch_base *)arch)->module->intnum_tobytes \
        (arch, intn, buf, destsize, valsize, shift, bc, warn)
#define yasm_arch_get_reg_size(arch, reg) \
    ((yasm_arch_base *)arch)->module->get_reg_size(arch, reg)
#define yasm_arch_reggroup_get_reg(arch, regg, regi) \
    ((yasm_arch_base *)arch)->module->reggroup_get_reg(arch, regg, regi)
#define yasm_arch_reg_print(arch, reg, f) \
    ((yasm_arch_base *)arch)->module->reg_print(arch, reg, f)
#define yasm_arch_segreg_print(arch, segreg, f) \
    ((yasm_arch_base *)arch)->module->segreg_print(arch, segreg, f)
#define yasm_arch_ea_create(arch, e) \
    ((yasm_arch_base *)arch)->module->ea_create(arch, e)
#define yasm_arch_ea_destroy(arch, ea) \
    ((yasm_arch_base *)arch)->module->ea_destroy(ea)
#define yasm_arch_ea_print(arch, ea, f, i) \
    ((yasm_arch_base *)arch)->module->ea_print(ea, f, i)
#define yasm_arch_create_empty_insn(arch, line) \
    ((yasm_arch_base *)arch)->module->create_empty_insn(arch, line)

#endif

#endif
