/**
 * \file libyasm/insn.h
 * \brief YASM mnenomic instruction.
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
#ifndef YASM_INSN_H
#define YASM_INSN_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Base structure for an effective address.  As with all base
 * structures, must be present as the first element in any
 * #yasm_arch implementation of an effective address.
 */
struct yasm_effaddr {
    yasm_value disp;            /**< address displacement */

    /** Segment register override (0 if none). */
    uintptr_t segreg;

    /** 1 if length of disp must be >0. */
    unsigned int need_nonzero_len:1;

    /** 1 if a displacement should be present in the output. */
    unsigned int need_disp:1;

    /** 1 if reg*2 should not be split into reg+reg. (0 if not).
     * This flag indicates (for architectures that support complex effective
     * addresses such as x86) if various types of complex effective addresses
     * can be split into different forms in order to minimize instruction
     * length.
     */
    unsigned int nosplit:1;

    /** 1 if effective address is /definitely/ an effective address.
     * This is used in e.g. the GAS parser to differentiate
     * between "expr" (which might or might not be an effective address) and
     * "expr(,1)" (which is definitely an effective address).
     */
    unsigned int strong:1;

    /** 1 if effective address is forced PC-relative. */
    unsigned int pc_rel:1;

    /** 1 if effective address is forced non-PC-relative. */
    unsigned int not_pc_rel:1;

    /** length of pointed data (in bytes), 0 if unknown. */
    unsigned int data_len;
};

/** An instruction operand (opaque type). */
typedef struct yasm_insn_operand yasm_insn_operand;

/** The type of an instruction operand. */
typedef enum yasm_insn_operand_type {
    YASM_INSN__OPERAND_REG = 1,     /**< A register. */
    YASM_INSN__OPERAND_SEGREG,      /**< A segment register. */
    YASM_INSN__OPERAND_MEMORY,      /**< An effective address
                                     *   (memory reference). */
    YASM_INSN__OPERAND_IMM          /**< An immediate or jump target. */
} yasm_insn_operand_type;

/** An instruction operand. */
struct yasm_insn_operand {
    /** Link for building linked list of operands.  \internal */
    /*@reldef@*/ STAILQ_ENTRY(yasm_insn_operand) link;

    /** Operand data. */
    union {
        uintptr_t reg;      /**< Arch data for reg/segreg. */
        yasm_effaddr *ea;   /**< Effective address for memory references. */
        yasm_expr *val;     /**< Value of immediate or jump target. */
    } data;

    yasm_expr *seg;         /**< Segment expression */

    uintptr_t targetmod;        /**< Arch target modifier, 0 if none. */

    /** Specified size of the operand, in bits.  0 if not user-specified. */
    unsigned int size:16;

    /** Nonzero if dereference.  Used for "*foo" in GAS.
     * The reason for this is that by default in GAS, an unprefixed value
     * is a memory address, except for jumps/calls, in which case it needs a
     * "*" prefix to become a memory address (otherwise it's an immediate).
     * This isn't knowable in the parser stage, so the parser sets this flag
     * to indicate the "*" prefix has been used, and the arch needs to adjust
     * the operand type appropriately depending on the instruction type.
     */
    unsigned int deref:1;

    /** Nonzero if strict.  Used for "strict foo" in NASM.
     * This is used to inhibit optimization on otherwise "sized" values.
     * For example, the user may just want to be explicit with the size on
     * "push dword 4", but not actually want to force the immediate size to
     * 4 bytes (rather wanting the optimizer to optimize it down to 1 byte as
     * though "dword" was not specified).  To indicate the immediate should
     * actually be forced to 4 bytes, the user needs to write
     * "push strict dword 4", which sets this flag.
     */
    unsigned int strict:1;

    /** Operand type. */
    unsigned int type:4;
};

/** Base structure for "instruction" bytecodes.  These are the mnenomic
 * (rather than raw) representation of instructions.  As with all base
 * structures, must be present as the first element in any
 * #yasm_arch implementation of mnenomic instruction bytecodes.
 */
struct yasm_insn {
    /** Linked list of operands. */
    /*@reldef@*/ STAILQ_HEAD(yasm_insn_operands, yasm_insn_operand) operands;

    /** Array of prefixes. */
    /*@null@*/ uintptr_t *prefixes;

    /** Array of segment prefixes. */
    /*@null@*/ uintptr_t *segregs;

    unsigned int num_operands;       /**< Number of operands. */
    unsigned int num_prefixes;       /**< Number of prefixes. */
    unsigned int num_segregs;        /**< Number of segment prefixes. */
};

/** Set segment override for an effective address.
 * Some architectures (such as x86) support segment overrides on effective
 * addresses.  A override of an override will result in a warning.
 * \param ea            effective address
 * \param segreg        segment register (0 if none)
 */
YASM_LIB_DECL
void yasm_ea_set_segreg(yasm_effaddr *ea, uintptr_t segreg);

/** Create an instruction operand from a register.
 * \param reg   register
 * \return Newly allocated operand.
 */
YASM_LIB_DECL
yasm_insn_operand *yasm_operand_create_reg(uintptr_t reg);

/** Create an instruction operand from a segment register.
 * \param segreg        segment register
 * \return Newly allocated operand.
 */
YASM_LIB_DECL
yasm_insn_operand *yasm_operand_create_segreg(uintptr_t segreg);

/** Create an instruction operand from an effective address.
 * \param ea    effective address
 * \return Newly allocated operand.
 */
YASM_LIB_DECL
yasm_insn_operand *yasm_operand_create_mem(/*@only@*/ yasm_effaddr *ea);

/** Create an instruction operand from an immediate expression.
 * Looks for cases of a single register and creates a register variant of
 * #yasm_insn_operand.
 * \param val   immediate expression
 * \return Newly allocated operand.
 */
YASM_LIB_DECL
yasm_insn_operand *yasm_operand_create_imm(/*@only@*/ yasm_expr *val);

/** Get the first operand in an instruction.
 * \param insn          instruction
 * \return First operand (NULL if no operands).
 */
yasm_insn_operand *yasm_insn_ops_first(yasm_insn *insn);
#define yasm_insn_ops_first(insn)   STAILQ_FIRST(&((insn)->operands))

/** Get the next operand in an instruction.
 * \param op            previous operand
 * \return Next operand (NULL if op was the last operand).
 */
yasm_insn_operand *yasm_insn_op_next(yasm_insn_operand *op);
#define yasm_insn_op_next(cur)      STAILQ_NEXT(cur, link)

/** Add operand to the end of an instruction.
 * \note Does not make a copy of the operand; so don't pass this function
 *       static or local variables, and discard the op pointer after calling
 *       this function.
 * \param insn          instruction
 * \param op            operand (may be NULL)
 * \return If operand was actually appended (it wasn't NULL), the operand;
 *         otherwise NULL.
 */
YASM_LIB_DECL
/*@null@*/ yasm_insn_operand *yasm_insn_ops_append
    (yasm_insn *insn,
     /*@returned@*/ /*@null@*/ yasm_insn_operand *op);

/** Associate a prefix with an instruction.
 * \param insn          instruction
 * \param prefix        data that identifies the prefix
 */
YASM_LIB_DECL
void yasm_insn_add_prefix(yasm_insn *insn, uintptr_t prefix);

/** Associate a segment prefix with an instruction.
 * \param insn          instruction
 * \param segreg        data that identifies the segment register
 */
YASM_LIB_DECL
void yasm_insn_add_seg_prefix(yasm_insn *insn, uintptr_t segreg);

/** Initialize the common parts of an instruction.
 * \internal For use by yasm_arch implementations only.
 * \param insn          instruction
 */
YASM_LIB_DECL
void yasm_insn_initialize(/*@out@*/ yasm_insn *insn);

/** Delete the common parts of an instruction.
 * \internal For use by yasm_arch implementations only.
 * \param insn          instruction
 * \param content       if nonzero, deletes content of each operand
 * \param arch          architecture
 */
YASM_LIB_DECL
void yasm_insn_delete(yasm_insn *insn,
                      void (*ea_destroy) (/*@only@*/ yasm_effaddr *));

/** Print a list of instruction operands.  For debugging purposes.
 * \internal For use by yasm_arch implementations only.
 * \param insn          instruction
 * \param f             file
 * \param indent_level  indentation level
 * \param arch          architecture
 */
YASM_LIB_DECL
void yasm_insn_print(const yasm_insn *insn, FILE *f, int indent_level);

/** Finalize the common parts of an instruction.
 * \internal For use by yasm_arch implementations only.
 * \param insn          instruction
 */
YASM_LIB_DECL
void yasm_insn_finalize(yasm_insn *insn);

#endif
