/*
 * Floating point number functions.
 *
 *  Copyright (C) 2001-2007  Peter Johnson
 *
 *  Based on public-domain x86 assembly code by Randall Hyde (8/28/91).
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

#include "coretype.h"
#include "bitvect.h"
#include "file.h"

#include "errwarn.h"
#include "floatnum.h"


/* 97-bit internal floating point format:
 * 0000000s eeeeeeee eeeeeeee m.....................................m
 * Sign          exponent     mantissa (80 bits)
 *                            79                                    0
 *
 * Only L.O. bit of Sign byte is significant.  The rest is zero.
 * Exponent is bias 32767.
 * Mantissa does NOT have an implied one bit (it's explicit).
 */
struct yasm_floatnum {
    /*@only@*/ wordptr mantissa;        /* Allocated to MANT_BITS bits */
    unsigned short exponent;
    unsigned char sign;
    unsigned char flags;
};

/* constants describing parameters of internal floating point format */
#define MANT_BITS       80
#define MANT_BYTES      10
#define MANT_SIGDIGITS  24
#define EXP_BIAS        0x7FFF
#define EXP_INF         0xFFFF
#define EXP_MAX         0xFFFE
#define EXP_MIN         1
#define EXP_ZERO        0

/* Flag settings for flags field */
#define FLAG_ISZERO     1<<0

/* Note this structure integrates the floatnum structure */
typedef struct POT_Entry_s {
    yasm_floatnum f;
    int dec_exponent;
} POT_Entry;

/* "Source" for POT_Entry. */
typedef struct POT_Entry_Source_s {
    unsigned char mantissa[MANT_BYTES];     /* little endian mantissa */
    unsigned short exponent;                /* Bias 32767 exponent */
} POT_Entry_Source;

/* Power of ten tables used by the floating point I/O routines.
 * The POT_Table? arrays are built from the POT_Table?_Source arrays at
 * runtime by POT_Table_Init().
 */

/* This table contains the powers of ten raised to negative powers of two:
 *
 * entry[12-n] = 10 ** (-2 ** n) for 0 <= n <= 12.
 * entry[13] = 1.0
 */
static /*@only@*/ POT_Entry *POT_TableN;
static POT_Entry_Source POT_TableN_Source[] = {
    {{0xe3,0x2d,0xde,0x9f,0xce,0xd2,0xc8,0x04,0xdd,0xa6},0x4ad8}, /* 1e-4096 */
    {{0x25,0x49,0xe4,0x2d,0x36,0x34,0x4f,0x53,0xae,0xce},0x656b}, /* 1e-2048 */
    {{0xa6,0x87,0xbd,0xc0,0x57,0xda,0xa5,0x82,0xa6,0xa2},0x72b5}, /* 1e-1024 */
    {{0x33,0x71,0x1c,0xd2,0x23,0xdb,0x32,0xee,0x49,0x90},0x795a}, /* 1e-512 */
    {{0x91,0xfa,0x39,0x19,0x7a,0x63,0x25,0x43,0x31,0xc0},0x7cac}, /* 1e-256 */
    {{0x7d,0xac,0xa0,0xe4,0xbc,0x64,0x7c,0x46,0xd0,0xdd},0x7e55}, /* 1e-128 */
    {{0x24,0x3f,0xa5,0xe9,0x39,0xa5,0x27,0xea,0x7f,0xa8},0x7f2a}, /* 1e-64 */
    {{0xde,0x67,0xba,0x94,0x39,0x45,0xad,0x1e,0xb1,0xcf},0x7f94}, /* 1e-32 */
    {{0x2f,0x4c,0x5b,0xe1,0x4d,0xc4,0xbe,0x94,0x95,0xe6},0x7fc9}, /* 1e-16 */
    {{0xc2,0xfd,0xfc,0xce,0x61,0x84,0x11,0x77,0xcc,0xab},0x7fe4}, /* 1e-8 */
    {{0xc3,0xd3,0x2b,0x65,0x19,0xe2,0x58,0x17,0xb7,0xd1},0x7ff1}, /* 1e-4 */
    {{0x71,0x3d,0x0a,0xd7,0xa3,0x70,0x3d,0x0a,0xd7,0xa3},0x7ff8}, /* 1e-2 */
    {{0xcd,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc},0x7ffb}, /* 1e-1 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80},0x7fff}, /* 1e-0 */
};

/* This table contains the powers of ten raised to positive powers of two:
 *
 * entry[12-n] = 10 ** (2 ** n) for 0 <= n <= 12.
 * entry[13] = 1.0
 * entry[-1] = entry[0];
 *
 * There is a -1 entry since it is possible for the algorithm to back up
 * before the table.  This -1 entry is created at runtime by duplicating the
 * 0 entry.
 */
static /*@only@*/ POT_Entry *POT_TableP;
static POT_Entry_Source POT_TableP_Source[] = {
    {{0x4c,0xc9,0x9a,0x97,0x20,0x8a,0x02,0x52,0x60,0xc4},0xb525}, /* 1e+4096 */
    {{0x4d,0xa7,0xe4,0x5d,0x3d,0xc5,0x5d,0x3b,0x8b,0x9e},0x9a92}, /* 1e+2048 */
    {{0x0d,0x65,0x17,0x0c,0x75,0x81,0x86,0x75,0x76,0xc9},0x8d48}, /* 1e+1024 */
    {{0x65,0xcc,0xc6,0x91,0x0e,0xa6,0xae,0xa0,0x19,0xe3},0x86a3}, /* 1e+512 */
    {{0xbc,0xdd,0x8d,0xde,0xf9,0x9d,0xfb,0xeb,0x7e,0xaa},0x8351}, /* 1e+256 */
    {{0x6f,0xc6,0xdf,0x8c,0xe9,0x80,0xc9,0x47,0xba,0x93},0x81a8}, /* 1e+128 */
    {{0xbf,0x3c,0xd5,0xa6,0xcf,0xff,0x49,0x1f,0x78,0xc2},0x80d3}, /* 1e+64 */
    {{0x20,0xf0,0x9d,0xb5,0x70,0x2b,0xa8,0xad,0xc5,0x9d},0x8069}, /* 1e+32 */
    {{0x00,0x00,0x00,0x00,0x00,0x04,0xbf,0xc9,0x1b,0x8e},0x8034}, /* 1e+16 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0xbc,0xbe},0x8019}, /* 1e+8 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x9c},0x800c}, /* 1e+4 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc8},0x8005}, /* 1e+2 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa0},0x8002}, /* 1e+1 */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80},0x7fff}, /* 1e+0 */
};


static void
POT_Table_Init_Entry(/*@out@*/ POT_Entry *e, POT_Entry_Source *s, int dec_exp)
{
    /* Save decimal exponent */
    e->dec_exponent = dec_exp;

    /* Initialize mantissa */
    e->f.mantissa = BitVector_Create(MANT_BITS, FALSE);
    BitVector_Block_Store(e->f.mantissa, s->mantissa, MANT_BYTES);

    /* Initialize exponent */
    e->f.exponent = s->exponent;

    /* Set sign to 0 (positive) */
    e->f.sign = 0;

    /* Clear flags */
    e->f.flags = 0;
}

/*@-compdef@*/
void
yasm_floatnum_initialize(void)
/*@globals undef POT_TableN, undef POT_TableP, POT_TableP_Source,
   POT_TableN_Source @*/
{
    int dec_exp = 1;
    int i;

    /* Allocate space for two POT tables */
    POT_TableN = yasm_xmalloc(14*sizeof(POT_Entry));
    POT_TableP = yasm_xmalloc(15*sizeof(POT_Entry)); /* note 1 extra for -1 */

    /* Initialize entry[0..12] */
    for (i=12; i>=0; i--) {
        POT_Table_Init_Entry(&POT_TableN[i], &POT_TableN_Source[i], 0-dec_exp);
        POT_Table_Init_Entry(&POT_TableP[i+1], &POT_TableP_Source[i], dec_exp);
        dec_exp *= 2;       /* Update decimal exponent */
    }

    /* Initialize entry[13] */
    POT_Table_Init_Entry(&POT_TableN[13], &POT_TableN_Source[13], 0);
    POT_Table_Init_Entry(&POT_TableP[14], &POT_TableP_Source[13], 0);

    /* Initialize entry[-1] for POT_TableP */
    POT_Table_Init_Entry(&POT_TableP[0], &POT_TableP_Source[0], 4096);

    /* Offset POT_TableP so that [0] becomes [-1] */
    POT_TableP++;
}
/*@=compdef@*/

/*@-globstate@*/
void
yasm_floatnum_cleanup(void)
{
    int i;

    /* Un-offset POT_TableP */
    POT_TableP--;

    for (i=0; i<14; i++) {
        BitVector_Destroy(POT_TableN[i].f.mantissa);
        BitVector_Destroy(POT_TableP[i].f.mantissa);
    }
    BitVector_Destroy(POT_TableP[14].f.mantissa);

    yasm_xfree(POT_TableN);
    yasm_xfree(POT_TableP);
}
/*@=globstate@*/

static void
floatnum_normalize(yasm_floatnum *flt)
{
    long norm_amt;

    if (BitVector_is_empty(flt->mantissa)) {
        flt->exponent = 0;
        return;
    }

    /* Look for the highest set bit, shift to make it the MSB, and adjust
     * exponent.  Don't let exponent go negative. */
    norm_amt = (MANT_BITS-1)-Set_Max(flt->mantissa);
    if (norm_amt > (long)flt->exponent)
        norm_amt = (long)flt->exponent;
    BitVector_Move_Left(flt->mantissa, (N_int)norm_amt);
    flt->exponent -= (unsigned short)norm_amt;
}

/* acc *= op */
static void
floatnum_mul(yasm_floatnum *acc, const yasm_floatnum *op)
{
    long expon;
    wordptr product, op1, op2;
    long norm_amt;

    /* Compute the new sign */
    acc->sign ^= op->sign;

    /* Check for multiply by 0 */
    if (BitVector_is_empty(acc->mantissa) || BitVector_is_empty(op->mantissa)) {
        BitVector_Empty(acc->mantissa);
        acc->exponent = EXP_ZERO;
        return;
    }

    /* Add exponents, checking for overflow/underflow. */
    expon = (((int)acc->exponent)-EXP_BIAS) + (((int)op->exponent)-EXP_BIAS);
    expon += EXP_BIAS;
    if (expon > EXP_MAX) {
        /* Overflow; return infinity. */
        BitVector_Empty(acc->mantissa);
        acc->exponent = EXP_INF;
        return;
    } else if (expon < EXP_MIN) {
        /* Underflow; return zero. */
        BitVector_Empty(acc->mantissa);
        acc->exponent = EXP_ZERO;
        return;
    }

    /* Add one to the final exponent, as the multiply shifts one extra time. */
    acc->exponent = (unsigned short)(expon+1);

    /* Allocate space for the multiply result */
    product = BitVector_Create((N_int)((MANT_BITS+1)*2), FALSE);

    /* Allocate 1-bit-longer fields to force the operands to be unsigned */
    op1 = BitVector_Create((N_int)(MANT_BITS+1), FALSE);
    op2 = BitVector_Create((N_int)(MANT_BITS+1), FALSE);

    /* Make the operands unsigned after copying from original operands */
    BitVector_Copy(op1, acc->mantissa);
    BitVector_MSB(op1, 0);
    BitVector_Copy(op2, op->mantissa);
    BitVector_MSB(op2, 0);

    /* Compute the product of the mantissas */
    BitVector_Multiply(product, op1, op2);

    /* Normalize the product.  Note: we know the product is non-zero because
     * both of the original operands were non-zero.
     *
     * Look for the highest set bit, shift to make it the MSB, and adjust
     * exponent.  Don't let exponent go negative.
     */
    norm_amt = (MANT_BITS*2-1)-Set_Max(product);
    if (norm_amt > (long)acc->exponent)
        norm_amt = (long)acc->exponent;
    BitVector_Move_Left(product, (N_int)norm_amt);
    acc->exponent -= (unsigned short)norm_amt;

    /* Store the highest bits of the result */
    BitVector_Interval_Copy(acc->mantissa, product, 0, MANT_BITS, MANT_BITS);

    /* Free allocated variables */
    BitVector_Destroy(product);
    BitVector_Destroy(op1);
    BitVector_Destroy(op2);
}

yasm_floatnum *
yasm_floatnum_create(const char *str)
{
    yasm_floatnum *flt;
    int dec_exponent, dec_exp_add;      /* decimal (powers of 10) exponent */
    int POT_index;
    wordptr operand[2];
    int sig_digits;
    int decimal_pt;
    boolean carry;

    flt = yasm_xmalloc(sizeof(yasm_floatnum));

    flt->mantissa = BitVector_Create(MANT_BITS, TRUE);

    /* allocate and initialize calculation variables */
    operand[0] = BitVector_Create(MANT_BITS, TRUE);
    operand[1] = BitVector_Create(MANT_BITS, TRUE);
    dec_exponent = 0;
    sig_digits = 0;
    decimal_pt = 1;

    /* set initial flags to 0 */
    flt->flags = 0;

    /* check for + or - character and skip */
    if (*str == '-') {
        flt->sign = 1;
        str++;
    } else if (*str == '+') {
        flt->sign = 0;
        str++;
    } else
        flt->sign = 0;

    /* eliminate any leading zeros (which do not count as significant digits) */
    while (*str == '0')
        str++;

    /* When we reach the end of the leading zeros, first check for a decimal
     * point.  If the number is of the form "0---0.0000" we need to get rid
     * of the zeros after the decimal point and not count them as significant
     * digits.
     */
    if (*str == '.') {
        str++;
        while (*str == '0') {
            str++;
            dec_exponent--;
        }
    } else {
        /* The number is of the form "yyy.xxxx" (where y <> 0). */
        while (isdigit(*str)) {
            /* See if we've processed more than the max significant digits: */
            if (sig_digits < MANT_SIGDIGITS) {
                /* Multiply mantissa by 10 [x = (x<<1)+(x<<3)] */
                BitVector_shift_left(flt->mantissa, 0);
                BitVector_Copy(operand[0], flt->mantissa);
                BitVector_Move_Left(flt->mantissa, 2);
                carry = 0;
                BitVector_add(operand[1], operand[0], flt->mantissa, &carry);

                /* Add in current digit */
                BitVector_Empty(operand[0]);
                BitVector_Chunk_Store(operand[0], 4, 0, (N_long)(*str-'0'));
                carry = 0;
                BitVector_add(flt->mantissa, operand[1], operand[0], &carry);
            } else {
                /* Can't integrate more digits with mantissa, so instead just
                 * raise by a power of ten.
                 */
                dec_exponent++;
            }
            sig_digits++;
            str++;
        }

        if (*str == '.')
            str++;
        else
            decimal_pt = 0;
    }

    if (decimal_pt) {
        /* Process the digits to the right of the decimal point. */
        while (isdigit(*str)) {
            /* See if we've processed more than 19 significant digits: */
            if (sig_digits < 19) {
                /* Raise by a power of ten */
                dec_exponent--;

                /* Multiply mantissa by 10 [x = (x<<1)+(x<<3)] */
                BitVector_shift_left(flt->mantissa, 0);
                BitVector_Copy(operand[0], flt->mantissa);
                BitVector_Move_Left(flt->mantissa, 2);
                carry = 0;
                BitVector_add(operand[1], operand[0], flt->mantissa, &carry);

                /* Add in current digit */
                BitVector_Empty(operand[0]);
                BitVector_Chunk_Store(operand[0], 4, 0, (N_long)(*str-'0'));
                carry = 0;
                BitVector_add(flt->mantissa, operand[1], operand[0], &carry);
            }
            sig_digits++;
            str++;
        }
    }

    if (*str == 'e' || *str == 'E') {
        str++;
        /* We just saw the "E" character, now read in the exponent value and
         * add it into dec_exponent.
         */
        dec_exp_add = 0;
        sscanf(str, "%d", &dec_exp_add);
        dec_exponent += dec_exp_add;
    }

    /* Free calculation variables. */
    BitVector_Destroy(operand[1]);
    BitVector_Destroy(operand[0]);

    /* Normalize the number, checking for 0 first. */
    if (BitVector_is_empty(flt->mantissa)) {
        /* Mantissa is 0, zero exponent too. */
        flt->exponent = 0;
        /* Set zero flag so output functions don't see 0 value as underflow. */
        flt->flags |= FLAG_ISZERO;
        /* Return 0 value. */
        return flt;
    }
    /* Exponent if already norm. */
    flt->exponent = (unsigned short)(0x7FFF+(MANT_BITS-1));
    floatnum_normalize(flt);

    /* The number is normalized.  Now multiply by 10 the number of times
     * specified in DecExponent.  This uses the power of ten tables to speed
     * up this operation (and make it more accurate).
     */
    if (dec_exponent > 0) {
        POT_index = 0;
        /* Until we hit 1.0 or finish exponent or overflow */
        while ((POT_index < 14) && (dec_exponent != 0) &&
               (flt->exponent != EXP_INF)) {
            /* Find the first power of ten in the table which is just less than
             * the exponent.
             */
            while (dec_exponent < POT_TableP[POT_index].dec_exponent)
                POT_index++;

            if (POT_index < 14) {
                /* Subtract out what we're multiplying in from exponent */
                dec_exponent -= POT_TableP[POT_index].dec_exponent;

                /* Multiply by current power of 10 */
                floatnum_mul(flt, &POT_TableP[POT_index].f);
            }
        }
    } else if (dec_exponent < 0) {
        POT_index = 0;
        /* Until we hit 1.0 or finish exponent or underflow */
        while ((POT_index < 14) && (dec_exponent != 0) &&
               (flt->exponent != EXP_ZERO)) {
            /* Find the first power of ten in the table which is just less than
             * the exponent.
             */
            while (dec_exponent > POT_TableN[POT_index].dec_exponent)
                POT_index++;

            if (POT_index < 14) {
                /* Subtract out what we're multiplying in from exponent */
                dec_exponent -= POT_TableN[POT_index].dec_exponent;

                /* Multiply by current power of 10 */
                floatnum_mul(flt, &POT_TableN[POT_index].f);
            }
        }
    }

    /* Round the result. (Don't round underflow or overflow).  Also don't
     * increment if this would cause the mantissa to wrap.
     */
    if ((flt->exponent != EXP_INF) && (flt->exponent != EXP_ZERO) &&
        !BitVector_is_full(flt->mantissa))
        BitVector_increment(flt->mantissa);

    return flt;
}

yasm_floatnum *
yasm_floatnum_copy(const yasm_floatnum *flt)
{
    yasm_floatnum *f = yasm_xmalloc(sizeof(yasm_floatnum));

    f->mantissa = BitVector_Clone(flt->mantissa);
    f->exponent = flt->exponent;
    f->sign = flt->sign;
    f->flags = flt->flags;

    return f;
}

void
yasm_floatnum_destroy(yasm_floatnum *flt)
{
    BitVector_Destroy(flt->mantissa);
    yasm_xfree(flt);
}

int
yasm_floatnum_calc(yasm_floatnum *acc, yasm_expr_op op,
                   /*@unused@*/ yasm_floatnum *operand)
{
    if (op != YASM_EXPR_NEG) {
        yasm_error_set(YASM_ERROR_FLOATING_POINT,
                       N_("Unsupported floating-point arithmetic operation"));
        return 1;
    }
    acc->sign ^= 1;
    return 0;
}

int
yasm_floatnum_get_int(const yasm_floatnum *flt, unsigned long *ret_val)
{
    unsigned char t[4];

    if (yasm_floatnum_get_sized(flt, t, 4, 32, 0, 0, 0)) {
        *ret_val = 0xDEADBEEFUL;    /* Obviously incorrect return value */
        return 1;
    }

    YASM_LOAD_32_L(*ret_val, &t[0]);
    return 0;
}

/* Function used by conversion routines to actually perform the conversion.
 *
 * ptr -> the array to return the little-endian floating point value into.
 * flt -> the floating point value to convert.
 * byte_size -> the size in bytes of the output format.
 * mant_bits -> the size in bits of the output mantissa.
 * implicit1 -> does the output format have an implicit 1? 1=yes, 0=no.
 * exp_bits -> the size in bits of the output exponent.
 *
 * Returns 0 on success, 1 if overflow, -1 if underflow.
 */
static int
floatnum_get_common(const yasm_floatnum *flt, /*@out@*/ unsigned char *ptr,
                    N_int byte_size, N_int mant_bits, int implicit1,
                    N_int exp_bits)
{
    long exponent = (long)flt->exponent;
    wordptr output;
    charptr buf;
    unsigned int len;
    unsigned int overflow = 0, underflow = 0;
    int retval = 0;
    long exp_bias = (1<<(exp_bits-1))-1;
    long exp_inf = (1<<exp_bits)-1;

    output = BitVector_Create(byte_size*8, TRUE);

    /* copy mantissa */
    BitVector_Interval_Copy(output, flt->mantissa, 0,
                            (N_int)((MANT_BITS-implicit1)-mant_bits),
                            mant_bits);

    /* round mantissa */
    if (BitVector_bit_test(flt->mantissa, (MANT_BITS-implicit1)-(mant_bits+1)))
        BitVector_increment(output);

    if (BitVector_bit_test(output, mant_bits)) {
        /* overflowed, so zero mantissa (and set explicit bit if necessary) */
        BitVector_Empty(output);
        BitVector_Bit_Copy(output, mant_bits-1, !implicit1);
        /* and up the exponent (checking for overflow) */
        if (exponent+1 >= EXP_INF)
            overflow = 1;
        else
            exponent++;
    }

    /* adjust the exponent to the output bias, checking for overflow */
    exponent -= EXP_BIAS-exp_bias;
    if (exponent >= exp_inf)
        overflow = 1;
    else if (exponent <= 0)
        underflow = 1;

    /* underflow and overflow both set!? */
    if (underflow && overflow)
        yasm_internal_error(N_("Both underflow and overflow set"));

    /* check for underflow or overflow and set up appropriate output */
    if (underflow) {
        BitVector_Empty(output);
        exponent = 0;
        if (!(flt->flags & FLAG_ISZERO))
            retval = -1;
    } else if (overflow) {
        BitVector_Empty(output);
        exponent = exp_inf;
        retval = 1;
    }

    /* move exponent into place */
    BitVector_Chunk_Store(output, exp_bits, mant_bits, (N_long)exponent);

    /* merge in sign bit */
    BitVector_Bit_Copy(output, byte_size*8-1, flt->sign);

    /* get little-endian bytes */
    buf = BitVector_Block_Read(output, &len);
    if (len < byte_size)
        yasm_internal_error(
            N_("Byte length of BitVector does not match bit length"));

    /* copy to output */
    memcpy(ptr, buf, byte_size*sizeof(unsigned char));

    /* free allocated resources */
    yasm_xfree(buf);

    BitVector_Destroy(output);

    return retval;
}

/* IEEE-754r "half precision" format:
 * 16 bits:
 * 15     9      Bit 0
 * |      |          |
 * seee eemm mmmm mmmm
 *
 * e = bias 15 exponent
 * s = sign bit
 * m = mantissa bits, bit 10 is an implied one bit.
 *
 * IEEE-754 (Intel) "single precision" format:
 * 32 bits:
 * Bit 31    Bit 22              Bit 0
 * |         |                       |
 * seeeeeee emmmmmmm mmmmmmmm mmmmmmmm
 *
 * e = bias 127 exponent
 * s = sign bit
 * m = mantissa bits, bit 23 is an implied one bit.
 *
 * IEEE-754 (Intel) "double precision" format:
 * 64 bits:
 * bit 63       bit 51                                               bit 0
 * |            |                                                        |
 * seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm
 *
 * e = bias 1023 exponent.
 * s = sign bit.
 * m = mantissa bits.  Bit 52 is an implied one bit.
 *
 * IEEE-754 (Intel) "extended precision" format:
 * 80 bits:
 * bit 79            bit 63                           bit 0
 * |                 |                                    |
 * seeeeeee eeeeeeee mmmmmmmm m...m m...m m...m m...m m...m
 *
 * e = bias 16383 exponent
 * m = 64 bit mantissa with NO implied bit!
 * s = sign (for mantissa)
 */
int
yasm_floatnum_get_sized(const yasm_floatnum *flt, unsigned char *ptr,
                        size_t destsize, size_t valsize, size_t shift,
                        int bigendian, int warn)
{
    int retval;
    if (destsize*8 != valsize || shift>0 || bigendian) {
        /* TODO */
        yasm_internal_error(N_("unsupported floatnum functionality"));
    }
    switch (destsize) {
        case 2:
            retval = floatnum_get_common(flt, ptr, 2, 10, 1, 5);
            break;
        case 4:
            retval = floatnum_get_common(flt, ptr, 4, 23, 1, 8);
            break;
        case 8:
            retval = floatnum_get_common(flt, ptr, 8, 52, 1, 11);
            break;
        case 10:
            retval = floatnum_get_common(flt, ptr, 10, 64, 0, 15);
            break;
        default:
            yasm_internal_error(N_("Invalid float conversion size"));
            /*@notreached@*/
            return 1;
    }
    if (warn) {
        if (retval < 0)
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("underflow in floating point expression"));
        else if (retval > 0)
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("overflow in floating point expression"));
    }
    return retval;
}

/* 1 if the size is valid, 0 if it isn't */
int
yasm_floatnum_check_size(/*@unused@*/ const yasm_floatnum *flt, size_t size)
{
    switch (size) {
        case 16:
        case 32:
        case 64:
        case 80:
            return 1;
        default:
            return 0;
    }
}

void
yasm_floatnum_print(const yasm_floatnum *flt, FILE *f)
{
    unsigned char out[10];
    unsigned char *str;
    int i;

    /* Internal format */
    str = BitVector_to_Hex(flt->mantissa);
    fprintf(f, "%c %s *2^%04x\n", flt->sign?'-':'+', (char *)str,
            flt->exponent);
    yasm_xfree(str);

    /* 32-bit (single precision) format */
    fprintf(f, "32-bit: %d: ",
            yasm_floatnum_get_sized(flt, out, 4, 32, 0, 0, 0));
    for (i=0; i<4; i++)
        fprintf(f, "%02x ", out[i]);
    fprintf(f, "\n");

    /* 64-bit (double precision) format */
    fprintf(f, "64-bit: %d: ",
            yasm_floatnum_get_sized(flt, out, 8, 64, 0, 0, 0));
    for (i=0; i<8; i++)
        fprintf(f, "%02x ", out[i]);
    fprintf(f, "\n");

    /* 80-bit (extended precision) format */
    fprintf(f, "80-bit: %d: ",
            yasm_floatnum_get_sized(flt, out, 10, 80, 0, 0, 0));
    for (i=0; i<10; i++)
        fprintf(f, "%02x ", out[i]);
    fprintf(f, "\n");
}
