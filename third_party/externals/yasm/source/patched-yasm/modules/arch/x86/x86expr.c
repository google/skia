/*
 * x86 expression handling
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
#include <util.h>

#include <libyasm.h>

#include "x86arch.h"


typedef struct x86_checkea_reg3264_data {
    int *regs;          /* total multiplier for each reg */
    unsigned char vsib_mode;
    unsigned char bits;
    unsigned char addrsize;
} x86_checkea_reg3264_data;

/* Only works if ei->type == EXPR_REG (doesn't check).
 * Overwrites ei with intnum of 0 (to eliminate regs from the final expr).
 */
static /*@null@*/ /*@dependent@*/ int *
x86_expr_checkea_get_reg3264(yasm_expr__item *ei, int *regnum,
                             /*returned*/ void *d)
{
    x86_checkea_reg3264_data *data = d;

    switch ((x86_expritem_reg_size)(ei->data.reg & ~0xFUL)) {
        case X86_REG32:
            if (data->addrsize != 32)
                return 0;
            *regnum = (unsigned int)(ei->data.reg & 0xF);
            break;
        case X86_REG64:
            if (data->addrsize != 64)
                return 0;
            *regnum = (unsigned int)(ei->data.reg & 0xF);
            break;
        case X86_XMMREG:
            if (data->vsib_mode != 1)
                return 0;
            if (data->bits != 64 && (ei->data.reg & 0x8) == 0x8)
                return 0;
            *regnum = 17+(unsigned int)(ei->data.reg & 0xF);
            break;
        case X86_YMMREG:
            if (data->vsib_mode != 2)
                return 0;
            if (data->bits != 64 && (ei->data.reg & 0x8) == 0x8)
                return 0;
            *regnum = 17+(unsigned int)(ei->data.reg & 0xF);
            break;
        case X86_RIP:
            if (data->bits != 64)
                return 0;
            *regnum = 16;
            break;
        default:
            return 0;
    }

    /* overwrite with 0 to eliminate register from displacement expr */
    ei->type = YASM_EXPR_INT;
    ei->data.intn = yasm_intnum_create_uint(0);

    /* we're okay */
    return &data->regs[*regnum];
}

typedef struct x86_checkea_reg16_data {
    int bx, si, di, bp;         /* total multiplier for each reg */
} x86_checkea_reg16_data;

/* Only works if ei->type == EXPR_REG (doesn't check).
 * Overwrites ei with intnum of 0 (to eliminate regs from the final expr).
 */
static /*@null@*/ int *
x86_expr_checkea_get_reg16(yasm_expr__item *ei, int *regnum, void *d)
{
    x86_checkea_reg16_data *data = d;
    /* in order: ax,cx,dx,bx,sp,bp,si,di */
    /*@-nullassign@*/
    static int *reg16[8] = {0,0,0,0,0,0,0,0};
    /*@=nullassign@*/

    reg16[3] = &data->bx;
    reg16[5] = &data->bp;
    reg16[6] = &data->si;
    reg16[7] = &data->di;

    /* don't allow 32-bit registers */
    if ((ei->data.reg & ~0xFUL) != X86_REG16)
        return 0;

    /* & 7 for sanity check */
    *regnum = (unsigned int)(ei->data.reg & 0x7);

    /* only allow BX, SI, DI, BP */
    if (!reg16[*regnum])
        return 0;

    /* overwrite with 0 to eliminate register from displacement expr */
    ei->type = YASM_EXPR_INT;
    ei->data.intn = yasm_intnum_create_uint(0);

    /* we're okay */
    return reg16[*regnum];
}

/* Distribute over registers to help bring them to the topmost level of e.
 * Also check for illegal operations against registers.
 * Returns 0 if something was illegal, 1 if legal and nothing in e changed,
 * and 2 if legal and e needs to be simplified.
 *
 * Only half joking: Someday make this/checkea able to accept crazy things
 *  like: (bx+di)*(bx+di)-bx*bx-2*bx*di-di*di+di?  Probably not: NASM never
 *  accepted such things, and it's doubtful such an expn is valid anyway
 *  (even though the above one is).  But even macros would be hard-pressed
 *  to generate something like this.
 *
 * e must already have been simplified for this function to work properly
 * (as it doesn't think things like SUB are valid).
 *
 * IMPLEMENTATION NOTE: About the only thing this function really needs to
 * "distribute" is: (non-float-expn or intnum) * (sum expn of registers).
 *
 * TODO: Clean up this code, make it easier to understand.
 */
static int
x86_expr_checkea_distcheck_reg(yasm_expr **ep, unsigned int bits)
{
    yasm_expr *e = *ep;
    int i;
    int havereg = -1, havereg_expr = -1;
    int retval = 1;     /* default to legal, no changes */

    for (i=0; i<e->numterms; i++) {
        switch (e->terms[i].type) {
            case YASM_EXPR_REG:
                /* Check op to make sure it's valid to use w/register. */
                switch (e->op) {
                    case YASM_EXPR_MUL:
                        /* Check for reg*reg */
                        if (havereg != -1)
                            return 0;
                        break;
                    case YASM_EXPR_ADD:
                    case YASM_EXPR_IDENT:
                        break;
                    default:
                        return 0;
                }
                havereg = i;
                break;
            case YASM_EXPR_FLOAT:
                /* Floats not allowed. */
                return 0;
            case YASM_EXPR_EXPR:
                if (yasm_expr__contains(e->terms[i].data.expn,
                                        YASM_EXPR_REG)) {
                    int ret2;

                    /* Check op to make sure it's valid to use w/register. */
                    if (e->op != YASM_EXPR_ADD && e->op != YASM_EXPR_MUL)
                        return 0;
                    /* Check for reg*reg */
                    if (e->op == YASM_EXPR_MUL && havereg != -1)
                        return 0;
                    havereg = i;
                    havereg_expr = i;
                    /* Recurse to check lower levels */
                    ret2 =
                        x86_expr_checkea_distcheck_reg(&e->terms[i].data.expn,
                                                       bits);
                    if (ret2 == 0)
                        return 0;
                    if (ret2 == 2)
                        retval = 2;
                } else if (yasm_expr__contains(e->terms[i].data.expn,
                                               YASM_EXPR_FLOAT))
                    return 0;   /* Disallow floats */
                break;
            default:
                break;
        }
    }

    /* just exit if no registers were used */
    if (havereg == -1)
        return retval;

    /* Distribute */
    if (e->op == YASM_EXPR_MUL && havereg_expr != -1) {
        yasm_expr *ne;

        retval = 2;     /* we're going to change it */

        /* The reg expn *must* be EXPR_ADD at this point.  Sanity check. */
        if (e->terms[havereg_expr].type != YASM_EXPR_EXPR ||
            e->terms[havereg_expr].data.expn->op != YASM_EXPR_ADD)
            yasm_internal_error(N_("Register expression not ADD or EXPN"));

        /* Iterate over each term in reg expn */
        for (i=0; i<e->terms[havereg_expr].data.expn->numterms; i++) {
            /* Copy everything EXCEPT havereg_expr term into new expression */
            ne = yasm_expr__copy_except(e, havereg_expr);
            assert(ne != NULL);
            /* Copy reg expr term into uncopied (empty) term in new expn */
            ne->terms[havereg_expr] =
                e->terms[havereg_expr].data.expn->terms[i]; /* struct copy */
            /* Overwrite old reg expr term with new expn */
            e->terms[havereg_expr].data.expn->terms[i].type = YASM_EXPR_EXPR;
            e->terms[havereg_expr].data.expn->terms[i].data.expn = ne;
        }

        /* Replace e with expanded reg expn */
        ne = e->terms[havereg_expr].data.expn;
        e->terms[havereg_expr].type = YASM_EXPR_NONE;   /* don't delete it! */
        yasm_expr_destroy(e);                       /* but everything else */
        e = ne;
        /*@-onlytrans@*/
        *ep = ne;
        /*@=onlytrans@*/
    }

    return retval;
}

/* Simplify and determine if expression is superficially valid:
 * Valid expr should be [(int-equiv expn)]+[reg*(int-equiv expn)+...]
 * where the [...] parts are optional.
 *
 * Don't simplify out constant identities if we're looking for an indexreg: we
 * may need the multiplier for determining what the indexreg is!
 *
 * Returns 1 if invalid register usage, 2 if unable to determine all values,
 * and 0 if all values successfully determined and saved in data.
 */
static int
x86_expr_checkea_getregusage(yasm_expr **ep, /*@null@*/ int *indexreg,
    int *pcrel, unsigned int bits, void *data,
    int *(*get_reg)(yasm_expr__item *ei, int *regnum, void *d))
{
    int i;
    int *reg;
    int regnum;
    int indexval = 0;
    int indexmult = 0;
    yasm_expr *e, *wrt;

    /*@-unqualifiedtrans@*/
    *ep = yasm_expr__level_tree(*ep, 1, 1, indexreg == 0, 0, NULL, NULL);

    /* Check for WRT rip first */
    wrt = yasm_expr_extract_wrt(ep);
    if (wrt && wrt->op == YASM_EXPR_IDENT &&
        wrt->terms[0].type == YASM_EXPR_REG) {
        if (bits != 64) {   /* only valid in 64-bit mode */
            yasm_expr_destroy(wrt);
            return 1;
        }
        reg = get_reg(&wrt->terms[0], &regnum, data);
        if (!reg || regnum != 16) { /* only accept rip */
            yasm_expr_destroy(wrt);
            return 1;
        }
        (*reg)++;

        /* Delete WRT.  Set pcrel to 1 to indicate to x86
         * bytecode code to do PC-relative displacement transform.
         */
        *pcrel = 1;
        yasm_expr_destroy(wrt);
    } else if (wrt) {
        yasm_expr_destroy(wrt);
        return 1;
    }

    /*@=unqualifiedtrans@*/
    assert(*ep != NULL);
    e = *ep;
    switch (x86_expr_checkea_distcheck_reg(ep, bits)) {
        case 0:
            return 1;
        case 2:
            /* Need to simplify again */
            *ep = yasm_expr__level_tree(*ep, 1, 1, indexreg == 0, 0, NULL,
                                        NULL);
            e = *ep;
            break;
        default:
            break;
    }

    switch (e->op) {
        case YASM_EXPR_ADD:
            /* Prescan for non-int multipliers against a reg.
             * This is invalid due to the optimizer structure.
             */
            for (i=0; i<e->numterms; i++)
                if (e->terms[i].type == YASM_EXPR_EXPR) {
                    yasm_expr__order_terms(e->terms[i].data.expn);
                    if (e->terms[i].data.expn->terms[0].type ==
                        YASM_EXPR_REG) {
                        if (e->terms[i].data.expn->numterms > 2)
                            return 1;
                        if (e->terms[i].data.expn->terms[1].type !=
                            YASM_EXPR_INT)
                            return 1;
                    }
                }

            /*@fallthrough@*/
        case YASM_EXPR_IDENT:
            /* Check each term for register (and possible multiplier). */
            for (i=0; i<e->numterms; i++) {
                if (e->terms[i].type == YASM_EXPR_REG) {
                    reg = get_reg(&e->terms[i], &regnum, data);
                    if (!reg)
                        return 1;
                    (*reg)++;
                    /* Let last, largest multipler win indexreg */
                    if (indexreg && *reg > 0 && indexval <= *reg &&
                        !indexmult) {
                        *indexreg = regnum;
                        indexval = *reg;
                    }
                } else if (e->terms[i].type == YASM_EXPR_EXPR) {
                    /* Already ordered from ADD above, just grab the value.
                     * Sanity check for EXPR_INT.
                     */
                    if (e->terms[i].data.expn->terms[0].type ==
                        YASM_EXPR_REG) {
                        long delta;
                        if (e->terms[i].data.expn->terms[1].type !=
                            YASM_EXPR_INT)
                            yasm_internal_error(
                                N_("Non-integer value in reg expn"));
                        reg = get_reg(&e->terms[i].data.expn->terms[0],
                                      &regnum, data);
                        if (!reg)
                            return 1;
                        delta = yasm_intnum_get_int(
                            e->terms[i].data.expn->terms[1].data.intn);
                        (*reg) += delta;
                        /* Let last, largest multipler win indexreg */
                        if (indexreg && delta > 0 && indexval <= *reg) {
                            *indexreg = regnum;
                            indexval = *reg;
                            indexmult = 1;
                        } else if (indexreg && *indexreg == regnum &&
                                   delta < 0 && *reg <= 1) {
                            *indexreg = -1;
                            indexval = 0;
                            indexmult = 0;
                        }
                    }
                }
            }
            break;
        case YASM_EXPR_MUL:
            /* Here, too, check for non-int multipliers against a reg. */
            yasm_expr__order_terms(e);
            if (e->terms[0].type == YASM_EXPR_REG) {
                long delta;
                if (e->numterms > 2)
                    return 1;
                if (e->terms[1].type != YASM_EXPR_INT)
                    return 1;
                reg = get_reg(&e->terms[0], &regnum, data);
                if (!reg)
                    return 1;
                delta = yasm_intnum_get_int(e->terms[1].data.intn);
                (*reg) += delta;
                if (indexreg)
                {
                    if (delta < 0 && *reg <= 1)
                    {
                        *indexreg = -1;
                        indexval = 0;
                        indexmult = 0;
                    }
                    else
                        *indexreg = regnum;
                }
            }
            break;
        case YASM_EXPR_SEGOFF:
            /* No registers are allowed on either side. */
            if (yasm_expr__contains(e, YASM_EXPR_REG))
                return 1;
            break;
        default:
            /* Should never get here! */
            yasm_internal_error(N_("unexpected expr op"));
    }

    /* Simplify expr, which is now really just the displacement. This
     * should get rid of the 0's we put in for registers in the callback.
     */
    *ep = yasm_expr_simplify(*ep, 0);
    /* e = *ep; */

    return 0;
}

/* Calculate the displacement length, if possible.
 * Takes several extra inputs so it can be used by both 32-bit and 16-bit
 * expressions:
 *  wordsize=16 for 16-bit, =32 for 32-bit.
 *  noreg=1 if the *ModRM byte* has no registers used.
 *  dispreq=1 if a displacement value is *required* (even if =0).
 * Returns 0 if successfully calculated, 1 if not.
 */
/*@-nullstate@*/
static int
x86_checkea_calc_displen(x86_effaddr *x86_ea, unsigned int wordsize, int noreg,
                         int dispreq)
{
    /*@null@*/ /*@only@*/ yasm_intnum *num;

    x86_ea->valid_modrm = 0;    /* default to not yet valid */

    switch (x86_ea->ea.disp.size) {
        case 0:
            break;
        /* If not 0, the displacement length was forced; set the Mod bits
         * appropriately and we're done with the ModRM byte.
         */
        case 8:
            /* Byte is only a valid override if there are registers in the
             * EA.  With no registers, we must have a 16/32 value.
             */
            if (noreg) {
                yasm_warn_set(YASM_WARN_IMPLICIT_SIZE_OVERRIDE,
                              N_("invalid displacement size; fixed"));
                x86_ea->ea.disp.size = wordsize;
            } else
                x86_ea->modrm |= 0100;
            x86_ea->valid_modrm = 1;
            return 0;
        case 16:
        case 32:
            /* Don't allow changing displacement different from BITS setting
             * directly; require an address-size override to change it.
             */
            if (wordsize != x86_ea->ea.disp.size) {
                yasm_error_set(YASM_ERROR_VALUE,
                    N_("invalid effective address (displacement size)"));
                return 1;
            }
            if (!noreg)
                x86_ea->modrm |= 0200;
            x86_ea->valid_modrm = 1;
            return 0;
        default:
            /* we shouldn't ever get any other size! */
            yasm_internal_error(N_("strange EA displacement size"));
    }

    /* The displacement length hasn't been forced (or the forcing wasn't
     * valid), try to determine what it is.
     */
    if (noreg) {
        /* No register in ModRM expression, so it must be disp16/32,
         * and as the Mod bits are set to 0 by the caller, we're done
         * with the ModRM byte.
         */
        x86_ea->ea.disp.size = wordsize;
        x86_ea->valid_modrm = 1;
        return 0;
    }

    if (dispreq) {
        /* for BP/EBP, there *must* be a displacement value, but we
         * may not know the size (8 or 16/32) for sure right now.
         */
        x86_ea->ea.need_nonzero_len = 1;
    }

    if (x86_ea->ea.disp.rel) {
        /* Relative displacement; basically all object formats need non-byte
         * for relocation here, so just do that. (TODO: handle this
         * differently?)
         */
        x86_ea->ea.disp.size = wordsize;
        x86_ea->modrm |= 0200;
        x86_ea->valid_modrm = 1;
        return 0;
    }

    /* At this point there's 3 possibilities for the displacement:
     *  - None (if =0)
     *  - signed 8 bit (if in -128 to 127 range)
     *  - 16/32 bit (word size)
     * For now, check intnum value right now; if it's not 0,
     * assume 8 bit and set up for allowing 16 bit later.
     * FIXME: The complex expression equaling zero is probably a rare case,
     * so we ignore it for now.
     */
    num = yasm_value_get_intnum(&x86_ea->ea.disp, NULL, 0);
    if (!num) {
        /* Still has unknown values. */
        x86_ea->ea.need_nonzero_len = 1;
        x86_ea->modrm |= 0100;
        x86_ea->valid_modrm = 1;
        return 0;
    }

    /* Figure out what size displacement we will have. */
    if (yasm_intnum_is_zero(num) && !x86_ea->ea.need_nonzero_len) {
        /* If we know that the displacement is 0 right now,
         * go ahead and delete the expr and make it so no
         * displacement value is included in the output.
         * The Mod bits of ModRM are set to 0 above, and
         * we're done with the ModRM byte!
         */
        yasm_value_delete(&x86_ea->ea.disp);
        x86_ea->ea.need_disp = 0;
    } else if (yasm_intnum_in_range(num, -128, 127)) {
        /* It fits into a signed byte */
        x86_ea->ea.disp.size = 8;
        x86_ea->modrm |= 0100;
    } else {
        /* It's a 16/32-bit displacement */
        x86_ea->ea.disp.size = wordsize;
        x86_ea->modrm |= 0200;
    }
    x86_ea->valid_modrm = 1;    /* We're done with ModRM */

    yasm_intnum_destroy(num);
    return 0;
}
/*@=nullstate@*/

static int
x86_expr_checkea_getregsize_callback(yasm_expr__item *ei, void *d)
{
    unsigned char *addrsize = (unsigned char *)d;

    if (ei->type == YASM_EXPR_REG) {
        switch ((x86_expritem_reg_size)(ei->data.reg & ~0xFUL)) {
            case X86_REG16:
                *addrsize = 16;
                break;
            case X86_REG32:
                *addrsize = 32;
                break;
            case X86_REG64:
            case X86_RIP:
                *addrsize = 64;
                break;
            default:
                return 0;
        }
        return 1;
    } else
        return 0;
}

int
yasm_x86__expr_checkea(x86_effaddr *x86_ea, unsigned char *addrsize,
                       unsigned int bits, int address16_op, unsigned char *rex,
                       yasm_bytecode *bc)
{
    int retval;

    if (*addrsize == 0) {
        /* we need to figure out the address size from what we know about:
         * - the displacement length
         * - what registers are used in the expression
         * - the bits setting
         */
        switch (x86_ea->ea.disp.size) {
            case 16:
                /* must be 16-bit */
                *addrsize = 16;
                break;
            case 64:
                /* We have to support this for the MemOffs case, but it's
                 * otherwise illegal.  It's also illegal in non-64-bit mode.
                 */
                if (x86_ea->need_modrm || x86_ea->need_sib) {
                    yasm_error_set(YASM_ERROR_VALUE,
                        N_("invalid effective address (displacement size)"));
                    return 1;
                }
                *addrsize = 64;
                break;
            case 32:
                /* Must be 32-bit in 16-bit or 32-bit modes.  In 64-bit mode,
                 * we don't know unless we look at the registers, except in the
                 * MemOffs case (see the end of this function).
                 */
                if (bits != 64 || (!x86_ea->need_modrm && !x86_ea->need_sib)) {
                    *addrsize = 32;
                    break;
                }
                /*@fallthrough@*/
            default:
                /* If SIB is required, but we're in 16-bit mode, set to 32. */
                if (bits == 16 && x86_ea->need_sib == 1) {
                    *addrsize = 32;
                    break;
                }
                /* check for use of 16 or 32-bit registers; if none are used
                 * default to bits setting.
                 */
                if (!x86_ea->ea.disp.abs ||
                    !yasm_expr__traverse_leaves_in(x86_ea->ea.disp.abs,
                        addrsize, x86_expr_checkea_getregsize_callback))
                    *addrsize = bits;
                /* TODO: Add optional warning here if switched address size
                 * from bits setting just by register use.. eg [ax] in
                 * 32-bit mode would generate a warning.
                 */
        }
    }

    if ((*addrsize == 32 || *addrsize == 64) &&
        ((x86_ea->need_modrm && !x86_ea->valid_modrm) ||
         (x86_ea->need_sib && !x86_ea->valid_sib))) {
        int i;
        unsigned char low3;
        typedef enum {
            REG3264_NONE = -1,
            REG3264_EAX = 0,
            REG3264_ECX,
            REG3264_EDX,
            REG3264_EBX,
            REG3264_ESP,
            REG3264_EBP,
            REG3264_ESI,
            REG3264_EDI,
            REG64_R8,
            REG64_R9,
            REG64_R10,
            REG64_R11,
            REG64_R12,
            REG64_R13,
            REG64_R14,
            REG64_R15,
            REG64_RIP,
            SIMDREGS
        } reg3264type;
        int reg3264mult[33] =
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        x86_checkea_reg3264_data reg3264_data;
        int basereg = REG3264_NONE;     /* "base" register (for SIB) */
        int indexreg = REG3264_NONE;    /* "index" register (for SIB) */
        int regcount = 17;              /* normally don't check SIMD regs */

        if (x86_ea->vsib_mode != 0)
            regcount = 33;

        /* We can only do 64-bit addresses in 64-bit mode. */
        if (*addrsize == 64 && bits != 64) {
            yasm_error_set(YASM_ERROR_TYPE,
                N_("invalid effective address (64-bit in non-64-bit mode)"));
            return 1;
        }

        if (x86_ea->ea.pc_rel && bits != 64) {
            yasm_warn_set(YASM_WARN_GENERAL,
                N_("RIP-relative directive ignored in non-64-bit mode"));
            x86_ea->ea.pc_rel = 0;
        }

        reg3264_data.regs = reg3264mult;
        reg3264_data.vsib_mode = x86_ea->vsib_mode;
        reg3264_data.bits = bits;
        reg3264_data.addrsize = *addrsize;
        if (x86_ea->ea.disp.abs) {
            int pcrel = 0;
            switch (x86_expr_checkea_getregusage
                    (&x86_ea->ea.disp.abs, &indexreg, &pcrel, bits,
                     &reg3264_data, x86_expr_checkea_get_reg3264)) {
                case 1:
                    yasm_error_set(YASM_ERROR_VALUE,
                                   N_("invalid effective address"));
                    return 1;
                case 2:
                    if (pcrel)
                        yasm_value_set_curpos_rel(&x86_ea->ea.disp, bc, 1);
                    return 2;
                default:
                    if (pcrel)
                        yasm_value_set_curpos_rel(&x86_ea->ea.disp, bc, 1);
                    break;
            }
        }

        /* If indexreg mult is 0, discard it.
         * This is possible because of the way indexreg is found in
         * expr_checkea_getregusage().
         */
        if (indexreg != REG3264_NONE && reg3264mult[indexreg] == 0)
            indexreg = REG3264_NONE;

        /* Find a basereg (*1, but not indexreg), if there is one.
         * Also, if an indexreg hasn't been assigned, try to find one.
         * Meanwhile, check to make sure there's no negative register mults.
         */
        for (i=0; i<regcount; i++) {
            if (reg3264mult[i] < 0) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("invalid effective address"));
                return 1;
            }
            if (i != indexreg && reg3264mult[i] == 1 &&
                basereg == REG3264_NONE)
                basereg = i;
            else if (indexreg == REG3264_NONE && reg3264mult[i] > 0)
                indexreg = i;
        }

        if (x86_ea->vsib_mode != 0) {
            /* For VSIB, the SIMD register needs to go into the indexreg.
             * Also check basereg (must be a GPR if present) and indexreg
             * (must be a SIMD register).
             */
            if (basereg >= SIMDREGS &&
                (indexreg == REG3264_NONE || reg3264mult[indexreg] == 1)) {
                int temp = basereg;
                basereg = indexreg;
                indexreg = temp;
            }
            if (basereg >= REG64_RIP || indexreg < SIMDREGS) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("invalid effective address"));
                return 1;
            }
        } else if (indexreg != REG3264_NONE && basereg == REG3264_NONE)
            /* Handle certain special cases of indexreg mults when basereg is
             * empty.
             */
            switch (reg3264mult[indexreg]) {
                case 1:
                    /* Only optimize this way if nosplit wasn't specified */
                    if (!x86_ea->ea.nosplit) {
                        basereg = indexreg;
                        indexreg = -1;
                    }
                    break;
                case 2:
                    /* Only split if nosplit wasn't specified */
                    if (!x86_ea->ea.nosplit) {
                        basereg = indexreg;
                        reg3264mult[indexreg] = 1;
                    }
                    break;
                case 3:
                case 5:
                case 9:
                    basereg = indexreg;
                    reg3264mult[indexreg]--;
                    break;
            }

        /* Make sure there's no other registers than the basereg and indexreg
         * we just found.
         */
        for (i=0; i<regcount; i++)
            if (i != basereg && i != indexreg && reg3264mult[i] != 0) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("invalid effective address"));
                return 1;
            }

        /* Check the index multiplier value for validity if present. */
        if (indexreg != REG3264_NONE && reg3264mult[indexreg] != 1 &&
            reg3264mult[indexreg] != 2 && reg3264mult[indexreg] != 4 &&
            reg3264mult[indexreg] != 8) {
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid effective address"));
            return 1;
        }

        /* ESP is not a legal indexreg. */
        if (indexreg == REG3264_ESP) {
            /* If mult>1 or basereg is ESP also, there's no way to make it
             * legal.
             */
            if (reg3264mult[REG3264_ESP] > 1 || basereg == REG3264_ESP) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("invalid effective address"));
                return 1;
            }
            /* If mult==1 and basereg is not ESP, swap indexreg w/basereg. */
            indexreg = basereg;
            basereg = REG3264_ESP;
        }

        /* RIP is only legal if it's the ONLY register used. */
        if (indexreg == REG64_RIP ||
            (basereg == REG64_RIP && indexreg != REG3264_NONE)) {
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid effective address"));
            return 1;
        }

        /* At this point, we know the base and index registers and that the
         * memory expression is (essentially) valid.  Now build the ModRM and
         * (optional) SIB bytes.
         */

        /* If we're supposed to be RIP-relative and there's no register
         * usage, change to RIP-relative.
         */
        if (basereg == REG3264_NONE && indexreg == REG3264_NONE &&
            x86_ea->ea.pc_rel) {
            basereg = REG64_RIP;
            yasm_value_set_curpos_rel(&x86_ea->ea.disp, bc, 1);
        }

        /* First determine R/M (Mod is later determined from disp size) */
        x86_ea->need_modrm = 1; /* we always need ModRM */
        if (basereg == REG3264_NONE && indexreg == REG3264_NONE) {
            /* Just a disp32: in 64-bit mode the RM encoding is used for RIP
             * offset addressing, so we need to use the SIB form instead.
             */
            if (bits == 64) {
                x86_ea->modrm |= 4;
                x86_ea->need_sib = 1;
            } else {
                x86_ea->modrm |= 5;
                x86_ea->sib = 0;
                x86_ea->valid_sib = 0;
                x86_ea->need_sib = 0;
            }
        } else if (basereg == REG64_RIP) {
            x86_ea->modrm |= 5;
            x86_ea->sib = 0;
            x86_ea->valid_sib = 0;
            x86_ea->need_sib = 0;
            /* RIP always requires a 32-bit displacement */
            x86_ea->valid_modrm = 1;
            x86_ea->ea.disp.size = 32;
            return 0;
        } else if (indexreg == REG3264_NONE) {
            /* basereg only */
            /* Don't need to go to the full effort of determining what type
             * of register basereg is, as x86_set_rex_from_reg doesn't pay
             * much attention.
             */
            if (yasm_x86__set_rex_from_reg(rex, &low3,
                                           (unsigned int)(X86_REG64 | basereg),
                                           bits, X86_REX_B))
                return 1;
            x86_ea->modrm |= low3;
            /* we don't need an SIB *unless* basereg is ESP or R12 */
            if (basereg == REG3264_ESP || basereg == REG64_R12)
                x86_ea->need_sib = 1;
            else {
                x86_ea->sib = 0;
                x86_ea->valid_sib = 0;
                x86_ea->need_sib = 0;
            }
        } else {
            /* index or both base and index */
            x86_ea->modrm |= 4;
            x86_ea->need_sib = 1;
        }

        /* Determine SIB if needed */
        if (x86_ea->need_sib == 1) {
            x86_ea->sib = 0;    /* start with 0 */

            /* Special case: no basereg */
            if (basereg == REG3264_NONE)
                x86_ea->sib |= 5;
            else {
                if (yasm_x86__set_rex_from_reg(rex, &low3, (unsigned int)
                                               (X86_REG64 | basereg), bits,
                                               X86_REX_B))
                    return 1;
                x86_ea->sib |= low3;
            }
            
            /* Put in indexreg, checking for none case */
            if (indexreg == REG3264_NONE)
                x86_ea->sib |= 040;
                /* Any scale field is valid, just leave at 0. */
            else {
                if (indexreg >= SIMDREGS) {
                    if (yasm_x86__set_rex_from_reg(rex, &low3,
                            (unsigned int)(X86_XMMREG | (indexreg-SIMDREGS)),
                            bits, X86_REX_X))
                        return 1;
                } else {
                    if (yasm_x86__set_rex_from_reg(rex, &low3,
                            (unsigned int)(X86_REG64 | indexreg),
                            bits, X86_REX_X))
                        return 1;
                }
                x86_ea->sib |= low3 << 3;
                /* Set scale field, 1 case -> 0, so don't bother. */
                switch (reg3264mult[indexreg]) {
                    case 2:
                        x86_ea->sib |= 0100;
                        break;
                    case 4:
                        x86_ea->sib |= 0200;
                        break;
                    case 8:
                        x86_ea->sib |= 0300;
                        break;
                }
            }

            x86_ea->valid_sib = 1;      /* Done with SIB */
        }

        /* Calculate displacement length (if possible) */
        retval = x86_checkea_calc_displen
            (x86_ea, 32, basereg == REG3264_NONE,
             basereg == REG3264_EBP || basereg == REG64_R13);
        return retval;
    } else if (*addrsize == 16 && x86_ea->need_modrm && !x86_ea->valid_modrm) {
        static const unsigned char modrm16[16] = {
            0006 /* disp16  */, 0007 /* [BX]    */, 0004 /* [SI]    */,
            0000 /* [BX+SI] */, 0005 /* [DI]    */, 0001 /* [BX+DI] */,
            0377 /* invalid */, 0377 /* invalid */, 0006 /* [BP]+d  */,
            0377 /* invalid */, 0002 /* [BP+SI] */, 0377 /* invalid */,
            0003 /* [BP+DI] */, 0377 /* invalid */, 0377 /* invalid */,
            0377 /* invalid */
        };
        x86_checkea_reg16_data reg16mult = {0, 0, 0, 0};
        enum {
            HAVE_NONE = 0,
            HAVE_BX = 1<<0,
            HAVE_SI = 1<<1,
            HAVE_DI = 1<<2,
            HAVE_BP = 1<<3
        } havereg = HAVE_NONE;

        /* 64-bit mode does not allow 16-bit addresses */
        if (bits == 64 && !address16_op) {
            yasm_error_set(YASM_ERROR_TYPE,
                N_("16-bit addresses not supported in 64-bit mode"));
            return 1;
        }

        /* 16-bit cannot have SIB */
        x86_ea->sib = 0;
        x86_ea->valid_sib = 0;
        x86_ea->need_sib = 0;

        if (x86_ea->ea.disp.abs) {
            int pcrel = 0;
            switch (x86_expr_checkea_getregusage
                    (&x86_ea->ea.disp.abs, (int *)NULL, &pcrel, bits,
                     &reg16mult, x86_expr_checkea_get_reg16)) {
                case 1:
                    yasm_error_set(YASM_ERROR_VALUE,
                                   N_("invalid effective address"));
                    return 1;
                case 2:
                    if (pcrel)
                        yasm_value_set_curpos_rel(&x86_ea->ea.disp, bc, 1);
                    return 2;
                default:
                    if (pcrel)
                        yasm_value_set_curpos_rel(&x86_ea->ea.disp, bc, 1);
                    break;
            }
        }

        /* reg multipliers not 0 or 1 are illegal. */
        if (reg16mult.bx & ~1 || reg16mult.si & ~1 || reg16mult.di & ~1 ||
            reg16mult.bp & ~1) {
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid effective address"));
            return 1;
        }

        /* Set havereg appropriately */
        if (reg16mult.bx > 0)
            havereg |= HAVE_BX;
        if (reg16mult.si > 0)
            havereg |= HAVE_SI;
        if (reg16mult.di > 0)
            havereg |= HAVE_DI;
        if (reg16mult.bp > 0)
            havereg |= HAVE_BP;

        /* Check the modrm value for invalid combinations. */
        if (modrm16[havereg] & 0070) {
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid effective address"));
            return 1;
        }

        /* Set ModRM byte for registers */
        x86_ea->modrm |= modrm16[havereg];

        /* Calculate displacement length (if possible) */
        retval = x86_checkea_calc_displen
            (x86_ea, 16, havereg == HAVE_NONE, havereg == HAVE_BP);
        return retval;
    } else if (!x86_ea->need_modrm && !x86_ea->need_sib) {
        /* Special case for MOV MemOffs opcode: displacement but no modrm. */
        switch (*addrsize) {
            case 64:
                if (bits != 64) {
                    yasm_error_set(YASM_ERROR_TYPE,
                        N_("invalid effective address (64-bit in non-64-bit mode)"));
                    return 1;
                }
                x86_ea->ea.disp.size = 64;
                break;
            case 32:
                x86_ea->ea.disp.size = 32;
                break;
            case 16:
                /* 64-bit mode does not allow 16-bit addresses */
                if (bits == 64 && !address16_op) {
                    yasm_error_set(YASM_ERROR_TYPE,
                        N_("16-bit addresses not supported in 64-bit mode"));
                    return 1;
                }
                x86_ea->ea.disp.size = 16;
                break;
        }
    }
    return 0;
}

int
yasm_x86__floatnum_tobytes(yasm_arch *arch, const yasm_floatnum *flt,
                           unsigned char *buf, size_t destsize, size_t valsize,
                           size_t shift, int warn)
{
    if (!yasm_floatnum_check_size(flt, valsize)) {
        yasm_error_set(YASM_ERROR_FLOATING_POINT,
                       N_("invalid floating point constant size"));
        return 1;
    }

    yasm_floatnum_get_sized(flt, buf, destsize, valsize, shift, 0, warn);
    return 0;
}
