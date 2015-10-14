/*
 * Integer number functions.
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
#include "util.h"

#include <ctype.h>
#include <limits.h>

#include "coretype.h"
#include "bitvect.h"
#include "file.h"

#include "errwarn.h"
#include "intnum.h"


/* "Native" "word" size for intnum calculations. */
#define BITVECT_NATIVE_SIZE     256

struct yasm_intnum {
    union val {
        long l;                 /* integer value (for integers <32 bits) */
        wordptr bv;             /* bit vector (for integers >=32 bits) */
    } val;
    enum { INTNUM_L, INTNUM_BV } type;
};

/* static bitvect used for conversions */
static /*@only@*/ wordptr conv_bv;

/* static bitvects used for computation */
static /*@only@*/ wordptr result, spare, op1static, op2static;

static /*@only@*/ BitVector_from_Dec_static_data *from_dec_data;


void
yasm_intnum_initialize(void)
{
    conv_bv = BitVector_Create(BITVECT_NATIVE_SIZE, FALSE);
    result = BitVector_Create(BITVECT_NATIVE_SIZE, FALSE);
    spare = BitVector_Create(BITVECT_NATIVE_SIZE, FALSE);
    op1static = BitVector_Create(BITVECT_NATIVE_SIZE, FALSE);
    op2static = BitVector_Create(BITVECT_NATIVE_SIZE, FALSE);
    from_dec_data = BitVector_from_Dec_static_Boot(BITVECT_NATIVE_SIZE);
}

void
yasm_intnum_cleanup(void)
{
    BitVector_from_Dec_static_Shutdown(from_dec_data);
    BitVector_Destroy(op2static);
    BitVector_Destroy(op1static);
    BitVector_Destroy(spare);
    BitVector_Destroy(result);
    BitVector_Destroy(conv_bv);
}

/* Compress a bitvector into intnum storage.
 * If saved as a bitvector, clones the passed bitvector.
 * Can modify the passed bitvector.
 */
static void
intnum_frombv(/*@out@*/ yasm_intnum *intn, wordptr bv)
{
    if (Set_Max(bv) < 31) {
        intn->type = INTNUM_L;
        intn->val.l = (long)BitVector_Chunk_Read(bv, 31, 0);
    } else if (BitVector_msb_(bv)) {
        /* Negative, negate and see if we'll fit into a long. */
        unsigned long ul;
        BitVector_Negate(bv, bv);
        if (Set_Max(bv) >= 32 ||
            ((ul = BitVector_Chunk_Read(bv, 32, 0)) & 0x80000000)) {
            /* too negative */
            BitVector_Negate(bv, bv);
            intn->type = INTNUM_BV;
            intn->val.bv = BitVector_Clone(bv);
        } else {
            intn->type = INTNUM_L;
            intn->val.l = -((long)ul);
        }
    } else {
        intn->type = INTNUM_BV;
        intn->val.bv = BitVector_Clone(bv);
    }
}

/* If intnum is a BV, returns its bitvector directly.
 * If not, converts into passed bv and returns that instead.
 */
static wordptr
intnum_tobv(/*@returned@*/ wordptr bv, const yasm_intnum *intn)
{
    if (intn->type == INTNUM_BV)
        return intn->val.bv;

    BitVector_Empty(bv);
    if (intn->val.l >= 0)
        BitVector_Chunk_Store(bv, 32, 0, (unsigned long)intn->val.l);
    else {
        BitVector_Chunk_Store(bv, 32, 0, (unsigned long)-intn->val.l);
        BitVector_Negate(bv, bv);
    }
    return bv;
}

yasm_intnum *
yasm_intnum_create_dec(char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    switch (BitVector_from_Dec_static(from_dec_data, conv_bv,
                                      (unsigned char *)str)) {
        case ErrCode_Pars:
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid decimal literal"));
            break;
        case ErrCode_Ovfl:
            yasm_error_set(YASM_ERROR_OVERFLOW,
                N_("Numeric constant too large for internal format"));
            break;
        default:
            break;
    }
    intnum_frombv(intn, conv_bv);
    return intn;
}

yasm_intnum *
yasm_intnum_create_bin(char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    switch (BitVector_from_Bin(conv_bv, (unsigned char *)str)) {
        case ErrCode_Pars:
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid binary literal"));
            break;
        case ErrCode_Ovfl:
            yasm_error_set(YASM_ERROR_OVERFLOW,
                N_("Numeric constant too large for internal format"));
            break;
        default:
            break;
    }
    intnum_frombv(intn, conv_bv);
    return intn;
}

yasm_intnum *
yasm_intnum_create_oct(char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    switch (BitVector_from_Oct(conv_bv, (unsigned char *)str)) {
        case ErrCode_Pars:
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid octal literal"));
            break;
        case ErrCode_Ovfl:
            yasm_error_set(YASM_ERROR_OVERFLOW,
                N_("Numeric constant too large for internal format"));
            break;
        default:
            break;
    }
    intnum_frombv(intn, conv_bv);
    return intn;
}

yasm_intnum *
yasm_intnum_create_hex(char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    switch (BitVector_from_Hex(conv_bv, (unsigned char *)str)) {
        case ErrCode_Pars:
            yasm_error_set(YASM_ERROR_VALUE, N_("invalid hex literal"));
            break;
        case ErrCode_Ovfl:
            yasm_error_set(YASM_ERROR_OVERFLOW,
                           N_("Numeric constant too large for internal format"));
            break;
        default:
            break;
    }
    intnum_frombv(intn, conv_bv);
    return intn;
}

/*@-usedef -compdef -uniondef@*/
yasm_intnum *
yasm_intnum_create_charconst_nasm(const char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));
    size_t len = strlen(str);

    if(len*8 > BITVECT_NATIVE_SIZE)
        yasm_error_set(YASM_ERROR_OVERFLOW,
                       N_("Character constant too large for internal format"));

    /* be conservative in choosing bitvect in case MSB is set */
    if (len > 3) {
        BitVector_Empty(conv_bv);
        intn->type = INTNUM_BV;
    } else {
        intn->val.l = 0;
        intn->type = INTNUM_L;
    }

    switch (len) {
        case 3:
            intn->val.l |= ((unsigned long)str[2]) & 0xff;
            intn->val.l <<= 8;
            /*@fallthrough@*/
        case 2:
            intn->val.l |= ((unsigned long)str[1]) & 0xff;
            intn->val.l <<= 8;
            /*@fallthrough@*/
        case 1:
            intn->val.l |= ((unsigned long)str[0]) & 0xff;
        case 0:
            break;
        default:
            /* >=32 bit conversion */
            while (len) {
                BitVector_Move_Left(conv_bv, 8);
                BitVector_Chunk_Store(conv_bv, 8, 0,
                                      ((unsigned long)str[--len]) & 0xff);
            }
            intn->val.bv = BitVector_Clone(conv_bv);
    }

    return intn;
}

yasm_intnum *
yasm_intnum_create_charconst_tasm(const char *str)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));
    size_t len = strlen(str);
    size_t i;

    if(len*8 > BITVECT_NATIVE_SIZE)
        yasm_error_set(YASM_ERROR_OVERFLOW,
                       N_("Character constant too large for internal format"));

    /* be conservative in choosing bitvect in case MSB is set */
    if (len > 3) {
        BitVector_Empty(conv_bv);
        intn->type = INTNUM_BV;
    } else {
        intn->val.l = 0;
        intn->type = INTNUM_L;
    }

    /* tasm uses big endian notation */
    i = 0;
    switch (len) {
        case 3:
            intn->val.l |= ((unsigned long)str[i++]) & 0xff;
            intn->val.l <<= 8;
            /*@fallthrough@*/
        case 2:
            intn->val.l |= ((unsigned long)str[i++]) & 0xff;
            intn->val.l <<= 8;
            /*@fallthrough@*/
        case 1:
            intn->val.l |= ((unsigned long)str[i++]) & 0xff;
        case 0:
            break;
        default:
            /* >=32 bit conversion */
            while (i < len) {
                BitVector_Chunk_Store(conv_bv, 8, (len-i-1)*8,
                                      ((unsigned long)str[i]) & 0xff);
                i++;
            }
            intn->val.bv = BitVector_Clone(conv_bv);
    }

    return intn;
}
/*@=usedef =compdef =uniondef@*/

yasm_intnum *
yasm_intnum_create_uint(unsigned long i)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    if (i > LONG_MAX) {
        /* Too big, store as bitvector */
        intn->val.bv = BitVector_Create(BITVECT_NATIVE_SIZE, TRUE);
        intn->type = INTNUM_BV;
        BitVector_Chunk_Store(intn->val.bv, 32, 0, i);
    } else {
        intn->val.l = (long)i;
        intn->type = INTNUM_L;
    }

    return intn;
}

yasm_intnum *
yasm_intnum_create_int(long i)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));

    intn->val.l = i;
    intn->type = INTNUM_L;

    return intn;
}

yasm_intnum *
yasm_intnum_create_leb128(const unsigned char *ptr, int sign,
                          unsigned long *size)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));
    const unsigned char *ptr_orig = ptr;
    unsigned long i = 0;

    BitVector_Empty(conv_bv);
    for (;;) {
        BitVector_Chunk_Store(conv_bv, 7, i, *ptr);
        i += 7;
        if ((*ptr & 0x80) != 0x80)
            break;
        ptr++;
    }

    *size = (unsigned long)(ptr-ptr_orig)+1;

    if(i > BITVECT_NATIVE_SIZE)
        yasm_error_set(YASM_ERROR_OVERFLOW,
                       N_("Numeric constant too large for internal format"));
    else if (sign && (*ptr & 0x40) == 0x40)
        BitVector_Interval_Fill(conv_bv, i, BITVECT_NATIVE_SIZE-1);

    intnum_frombv(intn, conv_bv);
    return intn;
}

yasm_intnum *
yasm_intnum_create_sized(unsigned char *ptr, int sign, size_t srcsize,
                         int bigendian)
{
    yasm_intnum *intn = yasm_xmalloc(sizeof(yasm_intnum));
    unsigned long i = 0;

    if (srcsize*8 > BITVECT_NATIVE_SIZE)
        yasm_error_set(YASM_ERROR_OVERFLOW,
                       N_("Numeric constant too large for internal format"));

    /* Read the buffer into a bitvect */
    BitVector_Empty(conv_bv);
    if (bigendian) {
        /* TODO */
        yasm_internal_error(N_("big endian not implemented"));
    } else {
        for (i = 0; i < srcsize; i++)
            BitVector_Chunk_Store(conv_bv, 8, i*8, ptr[i]);
    }

    /* Sign extend if needed */
    if (srcsize*8 < BITVECT_NATIVE_SIZE && sign && (ptr[i-1] & 0x80) == 0x80)
        BitVector_Interval_Fill(conv_bv, i*8, BITVECT_NATIVE_SIZE-1);

    intnum_frombv(intn, conv_bv);
    return intn;
}

yasm_intnum *
yasm_intnum_copy(const yasm_intnum *intn)
{
    yasm_intnum *n = yasm_xmalloc(sizeof(yasm_intnum));

    switch (intn->type) {
        case INTNUM_L:
            n->val.l = intn->val.l;
            break;
        case INTNUM_BV:
            n->val.bv = BitVector_Clone(intn->val.bv);
            break;
    }
    n->type = intn->type;

    return n;
}

void
yasm_intnum_destroy(yasm_intnum *intn)
{
    if (intn->type == INTNUM_BV)
        BitVector_Destroy(intn->val.bv);
    yasm_xfree(intn);
}

/*@-nullderef -nullpass -branchstate@*/
int
yasm_intnum_calc(yasm_intnum *acc, yasm_expr_op op, yasm_intnum *operand)
{
    boolean carry = 0;
    wordptr op1, op2 = NULL;
    N_int count;

    /* Always do computations with in full bit vector.
     * Bit vector results must be calculated through intermediate storage.
     */
    op1 = intnum_tobv(op1static, acc);
    if (operand)
        op2 = intnum_tobv(op2static, operand);

    if (!operand && op != YASM_EXPR_NEG && op != YASM_EXPR_NOT &&
        op != YASM_EXPR_LNOT) {
        yasm_error_set(YASM_ERROR_ARITHMETIC,
                       N_("operation needs an operand"));
        BitVector_Empty(result);
        return 1;
    }

    /* A operation does a bitvector computation if result is allocated. */
    switch (op) {
        case YASM_EXPR_ADD:
            BitVector_add(result, op1, op2, &carry);
            break;
        case YASM_EXPR_SUB:
            BitVector_sub(result, op1, op2, &carry);
            break;
        case YASM_EXPR_MUL:
            BitVector_Multiply(result, op1, op2);
            break;
        case YASM_EXPR_DIV:
            /* TODO: make sure op1 and op2 are unsigned */
            if (BitVector_is_empty(op2)) {
                yasm_error_set(YASM_ERROR_ZERO_DIVISION, N_("divide by zero"));
                BitVector_Empty(result);
                return 1;
            } else
                BitVector_Divide(result, op1, op2, spare);
            break;
        case YASM_EXPR_SIGNDIV:
            if (BitVector_is_empty(op2)) {
                yasm_error_set(YASM_ERROR_ZERO_DIVISION, N_("divide by zero"));
                BitVector_Empty(result);
                return 1;
            } else
                BitVector_Divide(result, op1, op2, spare);
            break;
        case YASM_EXPR_MOD:
            /* TODO: make sure op1 and op2 are unsigned */
            if (BitVector_is_empty(op2)) {
                yasm_error_set(YASM_ERROR_ZERO_DIVISION, N_("divide by zero"));
                BitVector_Empty(result);
                return 1;
            } else
                BitVector_Divide(spare, op1, op2, result);
            break;
        case YASM_EXPR_SIGNMOD:
            if (BitVector_is_empty(op2)) {
                yasm_error_set(YASM_ERROR_ZERO_DIVISION, N_("divide by zero"));
                BitVector_Empty(result);
                return 1;
            } else
                BitVector_Divide(spare, op1, op2, result);
            break;
        case YASM_EXPR_NEG:
            BitVector_Negate(result, op1);
            break;
        case YASM_EXPR_NOT:
            Set_Complement(result, op1);
            break;
        case YASM_EXPR_OR:
            Set_Union(result, op1, op2);
            break;
        case YASM_EXPR_AND:
            Set_Intersection(result, op1, op2);
            break;
        case YASM_EXPR_XOR:
            Set_ExclusiveOr(result, op1, op2);
            break;
        case YASM_EXPR_XNOR:
            Set_ExclusiveOr(result, op1, op2);
            Set_Complement(result, result);
            break;
        case YASM_EXPR_NOR:
            Set_Union(result, op1, op2);
            Set_Complement(result, result);
            break;
        case YASM_EXPR_SHL:
            if (operand->type == INTNUM_L && operand->val.l >= 0) {
                BitVector_Copy(result, op1);
                BitVector_Move_Left(result, (N_int)operand->val.l);
            } else      /* don't even bother, just zero result */
                BitVector_Empty(result);
            break;
        case YASM_EXPR_SHR:
            if (operand->type == INTNUM_L && operand->val.l >= 0) {
                BitVector_Copy(result, op1);
                carry = BitVector_msb_(op1);
                count = (N_int)operand->val.l;
                while (count-- > 0)
                    BitVector_shift_right(result, carry);
            } else      /* don't even bother, just zero result */
                BitVector_Empty(result);
            break;
        case YASM_EXPR_LOR:
            BitVector_Empty(result);
            BitVector_LSB(result, !BitVector_is_empty(op1) ||
                          !BitVector_is_empty(op2));
            break;
        case YASM_EXPR_LAND:
            BitVector_Empty(result);
            BitVector_LSB(result, !BitVector_is_empty(op1) &&
                          !BitVector_is_empty(op2));
            break;
        case YASM_EXPR_LNOT:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_is_empty(op1));
            break;
        case YASM_EXPR_LXOR:
            BitVector_Empty(result);
            BitVector_LSB(result, !BitVector_is_empty(op1) ^
                          !BitVector_is_empty(op2));
            break;
        case YASM_EXPR_LXNOR:
            BitVector_Empty(result);
            BitVector_LSB(result, !(!BitVector_is_empty(op1) ^
                          !BitVector_is_empty(op2)));
            break;
        case YASM_EXPR_LNOR:
            BitVector_Empty(result);
            BitVector_LSB(result, !(!BitVector_is_empty(op1) ||
                          !BitVector_is_empty(op2)));
            break;
        case YASM_EXPR_EQ:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_equal(op1, op2));
            break;
        case YASM_EXPR_LT:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_Compare(op1, op2) < 0);
            break;
        case YASM_EXPR_GT:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_Compare(op1, op2) > 0);
            break;
        case YASM_EXPR_LE:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_Compare(op1, op2) <= 0);
            break;
        case YASM_EXPR_GE:
            BitVector_Empty(result);
            BitVector_LSB(result, BitVector_Compare(op1, op2) >= 0);
            break;
        case YASM_EXPR_NE:
            BitVector_Empty(result);
            BitVector_LSB(result, !BitVector_equal(op1, op2));
            break;
        case YASM_EXPR_SEG:
            yasm_error_set(YASM_ERROR_ARITHMETIC, N_("invalid use of '%s'"),
                           "SEG");
            break;
        case YASM_EXPR_WRT:
            yasm_error_set(YASM_ERROR_ARITHMETIC, N_("invalid use of '%s'"),
                           "WRT");
            break;
        case YASM_EXPR_SEGOFF:
            yasm_error_set(YASM_ERROR_ARITHMETIC, N_("invalid use of '%s'"),
                           ":");
            break;
        case YASM_EXPR_IDENT:
            if (result)
                BitVector_Copy(result, op1);
            break;
        default:
            yasm_error_set(YASM_ERROR_ARITHMETIC,
                           N_("invalid operation in intnum calculation"));
            BitVector_Empty(result);
            return 1;
    }

    /* Try to fit the result into 32 bits if possible */
    if (acc->type == INTNUM_BV)
        BitVector_Destroy(acc->val.bv);
    intnum_frombv(acc, result);
    return 0;
}
/*@=nullderef =nullpass =branchstate@*/

int
yasm_intnum_compare(const yasm_intnum *intn1, const yasm_intnum *intn2)
{
    wordptr op1, op2;

    if (intn1->type == INTNUM_L && intn2->type == INTNUM_L) {
        if (intn1->val.l < intn2->val.l)
            return -1;
        if (intn1->val.l > intn2->val.l)
            return 1;
        return 0;
    }

    op1 = intnum_tobv(op1static, intn1);
    op2 = intnum_tobv(op2static, intn2);
    return BitVector_Compare(op1, op2);
}

void
yasm_intnum_zero(yasm_intnum *intn)
{
    yasm_intnum_set_int(intn, 0);
}

void
yasm_intnum_set(yasm_intnum *intn, const yasm_intnum *val)
{
    if (intn->type == val->type) {
        switch (val->type) {
            case INTNUM_L:
                intn->val.l = val->val.l;
                break;
            case INTNUM_BV:
                BitVector_Copy(intn->val.bv, val->val.bv);
                break;
        }
    } else {
        switch (val->type) {
            case INTNUM_L:
                BitVector_Destroy(intn->val.bv);
                intn->val.l = val->val.l;
                break;
            case INTNUM_BV:
                intn->val.bv = BitVector_Clone(val->val.bv);
                break;
        }
        intn->type = val->type;
    }
}

void
yasm_intnum_set_uint(yasm_intnum *intn, unsigned long val)
{
    if (val > LONG_MAX) {
        if (intn->type != INTNUM_BV) {
            intn->val.bv = BitVector_Create(BITVECT_NATIVE_SIZE, TRUE);
            intn->type = INTNUM_BV;
        }
        BitVector_Chunk_Store(intn->val.bv, 32, 0, val);
    } else {
        if (intn->type == INTNUM_BV) {
            BitVector_Destroy(intn->val.bv);
            intn->type = INTNUM_L;
        }
        intn->val.l = (long)val;
    }
}

void
yasm_intnum_set_int(yasm_intnum *intn, long val)
{
    if (intn->type == INTNUM_BV)
        BitVector_Destroy(intn->val.bv);
    intn->type = INTNUM_L;
    intn->val.l = val;
}

int
yasm_intnum_is_zero(const yasm_intnum *intn)
{
    return (intn->type == INTNUM_L && intn->val.l == 0);
}

int
yasm_intnum_is_pos1(const yasm_intnum *intn)
{
    return (intn->type == INTNUM_L && intn->val.l == 1);
}

int
yasm_intnum_is_neg1(const yasm_intnum *intn)
{
    return (intn->type == INTNUM_L && intn->val.l == -1);
}

int
yasm_intnum_sign(const yasm_intnum *intn)
{
    if (intn->type == INTNUM_L) {
        if (intn->val.l == 0)
            return 0;
        else if (intn->val.l < 0)
            return -1;
        else
            return 1;
    } else
        return BitVector_Sign(intn->val.bv);
}

unsigned long
yasm_intnum_get_uint(const yasm_intnum *intn)
{
    switch (intn->type) {
        case INTNUM_L:
            if (intn->val.l < 0)
                return 0;
            return (unsigned long)intn->val.l;
        case INTNUM_BV:
            if (BitVector_msb_(intn->val.bv))
                return 0;
            if (Set_Max(intn->val.bv) > 32)
                return ULONG_MAX;
            return BitVector_Chunk_Read(intn->val.bv, 32, 0);
        default:
            yasm_internal_error(N_("unknown intnum type"));
            /*@notreached@*/
            return 0;
    }
}

long
yasm_intnum_get_int(const yasm_intnum *intn)
{
    switch (intn->type) {
        case INTNUM_L:
            return intn->val.l;
        case INTNUM_BV:
            if (BitVector_msb_(intn->val.bv)) {
                /* it's negative: negate the bitvector to get a positive
                 * number, then negate the positive number.
                 */
                unsigned long ul;

                BitVector_Negate(conv_bv, intn->val.bv);
                if (Set_Max(conv_bv) >= 32) {
                    /* too negative */
                    return LONG_MIN;
                }
                ul = BitVector_Chunk_Read(conv_bv, 32, 0);
                /* check for too negative */
                return (ul & 0x80000000) ? LONG_MIN : -((long)ul);
            }

            /* it's positive, and since it's a BV, it must be >0x7FFFFFFF */
            return LONG_MAX;
        default:
            yasm_internal_error(N_("unknown intnum type"));
            /*@notreached@*/
            return 0;
    }
}

void
yasm_intnum_get_sized(const yasm_intnum *intn, unsigned char *ptr,
                      size_t destsize, size_t valsize, int shift,
                      int bigendian, int warn)
{
    wordptr op1 = op1static, op2;
    unsigned char *buf;
    unsigned int len;
    size_t rshift = shift < 0 ? (size_t)(-shift) : 0;
    int carry_in;

    /* Currently don't support destinations larger than our native size */
    if (destsize*8 > BITVECT_NATIVE_SIZE)
        yasm_internal_error(N_("destination too large"));

    /* General size warnings */
    if (warn<0 && !yasm_intnum_check_size(intn, valsize, rshift, 1))
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("value does not fit in signed %d bit field"),
                      valsize);
    if (warn>0 && !yasm_intnum_check_size(intn, valsize, rshift, 2))
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("value does not fit in %d bit field"), valsize);

    /* Read the original data into a bitvect */
    if (bigendian) {
        /* TODO */
        yasm_internal_error(N_("big endian not implemented"));
    } else
        BitVector_Block_Store(op1, ptr, (N_int)destsize);

    /* If not already a bitvect, convert value to be written to a bitvect */
    op2 = intnum_tobv(op2static, intn);

    /* Check low bits if right shifting and warnings enabled */
    if (warn && rshift > 0) {
        BitVector_Copy(conv_bv, op2);
        BitVector_Move_Left(conv_bv, (N_int)(BITVECT_NATIVE_SIZE-rshift));
        if (!BitVector_is_empty(conv_bv))
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("misaligned value, truncating to boundary"));
    }

    /* Shift right if needed */
    if (rshift > 0) {
        carry_in = BitVector_msb_(op2);
        while (rshift-- > 0)
            BitVector_shift_right(op2, carry_in);
        shift = 0;
    }

    /* Write the new value into the destination bitvect */
    BitVector_Interval_Copy(op1, op2, (unsigned int)shift, 0, (N_int)valsize);

    /* Write out the new data */
    buf = BitVector_Block_Read(op1, &len);
    if (bigendian) {
        /* TODO */
        yasm_internal_error(N_("big endian not implemented"));
    } else
        memcpy(ptr, buf, destsize);
    yasm_xfree(buf);
}

/* Return 1 if okay size, 0 if not */
int
yasm_intnum_check_size(const yasm_intnum *intn, size_t size, size_t rshift,
                       int rangetype)
{
    wordptr val;

    /* If not already a bitvect, convert value to a bitvect */
    if (intn->type == INTNUM_BV) {
        if (rshift > 0) {
            val = conv_bv;
            BitVector_Copy(val, intn->val.bv);
        } else
            val = intn->val.bv;
    } else
        val = intnum_tobv(conv_bv, intn);

    if (size >= BITVECT_NATIVE_SIZE)
        return 1;

    if (rshift > 0) {
        int carry_in = BitVector_msb_(val);
        while (rshift-- > 0)
            BitVector_shift_right(val, carry_in);
    }

    if (rangetype > 0) {
        if (BitVector_msb_(val)) {
            /* it's negative */
            int retval;

            BitVector_Negate(conv_bv, val);
            BitVector_dec(conv_bv, conv_bv);
            retval = Set_Max(conv_bv) < (long)size-1;

            return retval;
        }
        
        if (rangetype == 1)
            size--;
    }
    return (Set_Max(val) < (long)size);
}

int
yasm_intnum_in_range(const yasm_intnum *intn, long low, long high)
{
    wordptr val = intnum_tobv(result, intn);
    wordptr lval = op1static;
    wordptr hval = op2static;

    /* Convert high and low to bitvects */
    BitVector_Empty(lval);
    if (low >= 0)
        BitVector_Chunk_Store(lval, 32, 0, (unsigned long)low);
    else {
        BitVector_Chunk_Store(lval, 32, 0, (unsigned long)(-low));
        BitVector_Negate(lval, lval);
    }

    BitVector_Empty(hval);
    if (high >= 0)
        BitVector_Chunk_Store(hval, 32, 0, (unsigned long)high);
    else {
        BitVector_Chunk_Store(hval, 32, 0, (unsigned long)(-high));
        BitVector_Negate(hval, hval);
    }

    /* Compare! */
    return (BitVector_Compare(val, lval) >= 0
            && BitVector_Compare(val, hval) <= 0);
}

static unsigned long
get_leb128(wordptr val, unsigned char *ptr, int sign)
{
    unsigned long i, size;
    unsigned char *ptr_orig = ptr;

    if (sign) {
        /* Signed mode */
        if (BitVector_msb_(val)) {
            /* Negative */
            BitVector_Negate(conv_bv, val);
            size = Set_Max(conv_bv)+2;
        } else {
            /* Positive */
            size = Set_Max(val)+2;
        }
    } else {
        /* Unsigned mode */
        size = Set_Max(val)+1;
    }

    /* Positive/Unsigned write */
    for (i=0; i<size; i += 7) {
        *ptr = (unsigned char)BitVector_Chunk_Read(val, 7, i);
        *ptr |= 0x80;
        ptr++;
    }
    *(ptr-1) &= 0x7F;   /* Clear MSB of last byte */
    return (unsigned long)(ptr-ptr_orig);
}

static unsigned long
size_leb128(wordptr val, int sign)
{
    if (sign) {
        /* Signed mode */
        if (BitVector_msb_(val)) {
            /* Negative */
            BitVector_Negate(conv_bv, val);
            return (Set_Max(conv_bv)+8)/7;
        } else {
            /* Positive */
            return (Set_Max(val)+8)/7;
        }
    } else {
        /* Unsigned mode */
        return (Set_Max(val)+7)/7;
    }
}

unsigned long
yasm_intnum_get_leb128(const yasm_intnum *intn, unsigned char *ptr, int sign)
{
    wordptr val;

    /* Shortcut 0 */
    if (intn->type == INTNUM_L && intn->val.l == 0) {
        *ptr = 0;
        return 1;
    }

    /* If not already a bitvect, convert value to be written to a bitvect */
    val = intnum_tobv(op1static, intn);

    return get_leb128(val, ptr, sign);
}

unsigned long
yasm_intnum_size_leb128(const yasm_intnum *intn, int sign)
{
    wordptr val;

    /* Shortcut 0 */
    if (intn->type == INTNUM_L && intn->val.l == 0) {
        return 1;
    }

    /* If not already a bitvect, convert value to a bitvect */
    val = intnum_tobv(op1static, intn);

    return size_leb128(val, sign);
}

unsigned long
yasm_get_sleb128(long v, unsigned char *ptr)
{
    wordptr val = op1static;

    /* Shortcut 0 */
    if (v == 0) {
        *ptr = 0;
        return 1;
    }

    BitVector_Empty(val);
    if (v >= 0)
        BitVector_Chunk_Store(val, 32, 0, (unsigned long)v);
    else {
        BitVector_Chunk_Store(val, 32, 0, (unsigned long)(-v));
        BitVector_Negate(val, val);
    }
    return get_leb128(val, ptr, 1);
}

unsigned long
yasm_size_sleb128(long v)
{
    wordptr val = op1static;

    if (v == 0)
        return 1;

    BitVector_Empty(val);
    if (v >= 0)
        BitVector_Chunk_Store(val, 32, 0, (unsigned long)v);
    else {
        BitVector_Chunk_Store(val, 32, 0, (unsigned long)(-v));
        BitVector_Negate(val, val);
    }
    return size_leb128(val, 1);
}

unsigned long
yasm_get_uleb128(unsigned long v, unsigned char *ptr)
{
    wordptr val = op1static;

    /* Shortcut 0 */
    if (v == 0) {
        *ptr = 0;
        return 1;
    }

    BitVector_Empty(val);
    BitVector_Chunk_Store(val, 32, 0, v);
    return get_leb128(val, ptr, 0);
}

unsigned long
yasm_size_uleb128(unsigned long v)
{
    wordptr val = op1static;

    if (v == 0)
        return 1;

    BitVector_Empty(val);
    BitVector_Chunk_Store(val, 32, 0, v);
    return size_leb128(val, 0);
}

char *
yasm_intnum_get_str(const yasm_intnum *intn)
{
    unsigned char *s;

    switch (intn->type) {
        case INTNUM_L:
            s = yasm_xmalloc(16);
            sprintf((char *)s, "%ld", intn->val.l);
            return (char *)s;
            break;
        case INTNUM_BV:
            return (char *)BitVector_to_Dec(intn->val.bv);
            break;
    }
    /*@notreached@*/
    return NULL;
}

void
yasm_intnum_print(const yasm_intnum *intn, FILE *f)
{
    unsigned char *s;

    switch (intn->type) {
        case INTNUM_L:
            fprintf(f, "0x%lx", intn->val.l);
            break;
        case INTNUM_BV:
            s = BitVector_to_Hex(intn->val.bv);
            fprintf(f, "0x%s", (char *)s);
            yasm_xfree(s);
            break;
    }
}
