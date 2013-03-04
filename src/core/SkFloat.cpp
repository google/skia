
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFloat.h"
#include "SkMathPriv.h"

#define EXP_BIAS    (127+23)

static int get_unsigned_exp(uint32_t packed)
{
    return (packed << 1 >> 24);
}

static unsigned get_unsigned_value(uint32_t packed)
{
    return (packed << 9 >> 9) | (1 << 23);
}

static int get_signed_value(int32_t packed)
{
    return SkApplySign(get_unsigned_value(packed), SkExtractSign(packed));
}

/////////////////////////////////////////////////////////////////////////

int SkFloat::GetShift(int32_t packed, int shift)
{
    if (packed == 0)
        return 0;

    int exp = get_unsigned_exp(packed) - EXP_BIAS - shift;
    int value = get_unsigned_value(packed);

    if (exp >= 0)
    {
        if (exp > 8)    // overflow
            value = SK_MaxS32;
        else
            value <<= exp;
    }
    else
    {
        exp = -exp;
        if (exp > 23)   // underflow
            value = 0;
        else
            value >>= exp;
    }
    return SkApplySign(value, SkExtractSign(packed));
}

/////////////////////////////////////////////////////////////////////////////////////

int32_t SkFloat::SetShift(int value, int shift)
{
    if (value == 0)
        return 0;

    // record the sign and make value positive
    int sign = SkExtractSign(value);
    value = SkApplySign(value, sign);

    if (value >> 24)    // value is too big (has more than 24 bits set)
    {
        int bias = 8 - SkCLZ(value);
        SkASSERT(bias > 0 && bias < 8);
        value >>= bias;
        shift += bias;
    }
    else
    {
        int zeros = SkCLZ(value << 8);
        SkASSERT(zeros >= 0 && zeros <= 23);
        value <<= zeros;
        shift -= zeros;
    }
    // now value is left-aligned to 24 bits
    SkASSERT((value >> 23) == 1);

    shift += EXP_BIAS;
    if (shift < 0)  // underflow
        return 0;
    else
    {
        if (shift > 255)    // overflow
        {
            shift = 255;
            value = 0x00FFFFFF;
        }
        int32_t packed = sign << 31;        // set the sign-bit
        packed |= shift << 23;          // store the packed exponent
        packed |= ((unsigned)(value << 9) >> 9);    // clear 24th bit of value (its implied)

#ifdef SK_DEBUG
        {
            int n;

            n = SkExtractSign(packed);
            SkASSERT(n == sign);
            n = get_unsigned_exp(packed);
            SkASSERT(n == shift);
            n = get_unsigned_value(packed);
            SkASSERT(n == value);
        }
#endif
        return packed;
    }
}

int32_t SkFloat::Neg(int32_t packed)
{
    if (packed)
        packed = packed ^ (1 << 31);
    return packed;
}

int32_t SkFloat::Add(int32_t packed_a, int32_t packed_b)
{
    if (packed_a == 0)
        return packed_b;
    if (packed_b == 0)
        return packed_a;

    int exp_a = get_unsigned_exp(packed_a);
    int exp_b = get_unsigned_exp(packed_b);
    int exp_diff = exp_a - exp_b;

    int shift_a = 0, shift_b = 0;
    int exp;

    if (exp_diff >= 0)
    {
        if (exp_diff > 24)  // B is too small to contribute
            return packed_a;
        shift_b = exp_diff;
        exp = exp_a;
    }
    else
    {
        exp_diff = -exp_diff;
        if (exp_diff > 24)  // A is too small to contribute
            return packed_b;
        shift_a = exp_diff;
        exp = exp_b;
    }

    int value_a = get_signed_value(packed_a) >> shift_a;
    int value_b = get_signed_value(packed_b) >> shift_b;

    return SkFloat::SetShift(value_a + value_b, exp - EXP_BIAS);
}

#include "Sk64.h"

static inline int32_t mul24(int32_t a, int32_t b)
{
    Sk64 tmp;

    tmp.setMul(a, b);
    tmp.roundRight(24);
    return tmp.get32();
}

int32_t SkFloat::Mul(int32_t packed_a, int32_t packed_b)
{
    if (packed_a == 0 || packed_b == 0)
        return 0;

    int exp_a = get_unsigned_exp(packed_a);
    int exp_b = get_unsigned_exp(packed_b);

    int value_a = get_signed_value(packed_a);
    int value_b = get_signed_value(packed_b);

    return SkFloat::SetShift(mul24(value_a, value_b), exp_a + exp_b - 2*EXP_BIAS + 24);
}

int32_t SkFloat::MulInt(int32_t packed, int n)
{
    return Mul(packed, SetShift(n, 0));
}

int32_t SkFloat::Div(int32_t packed_n, int32_t packed_d)
{
    SkASSERT(packed_d != 0);

    if (packed_n == 0)
        return 0;

    int exp_n = get_unsigned_exp(packed_n);
    int exp_d = get_unsigned_exp(packed_d);

    int value_n = get_signed_value(packed_n);
    int value_d = get_signed_value(packed_d);

    return SkFloat::SetShift(SkDivBits(value_n, value_d, 24), exp_n - exp_d - 24);
}

int32_t SkFloat::DivInt(int32_t packed, int n)
{
    return Div(packed, SetShift(n, 0));
}

int32_t SkFloat::Invert(int32_t packed)
{
    return Div(packed, SetShift(1, 0));
}

int32_t SkFloat::Sqrt(int32_t packed)
{
    if (packed < 0)
    {
        SkDEBUGFAIL("can't sqrt a negative number");
        return 0;
    }

    int exp = get_unsigned_exp(packed);
    int value = get_unsigned_value(packed);

    int nexp = exp - EXP_BIAS;
    int root = SkSqrtBits(value << (nexp & 1), 26);
    nexp >>= 1;
    return SkFloat::SetShift(root, nexp - 11);
}

#if defined _WIN32 && _MSC_VER >= 1300  // disable warning : unreachable code
#pragma warning ( push )
#pragma warning ( disable : 4702 )
#endif

int32_t SkFloat::CubeRoot(int32_t packed)
{
    sk_throw();
    return 0;
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

static inline int32_t clear_high_bit(int32_t n)
{
    return ((uint32_t)(n << 1)) >> 1;
}

static inline int int_sign(int32_t a, int32_t b)
{
    return a > b ? 1 : (a < b ? -1 : 0);
}

int SkFloat::Cmp(int32_t packed_a, int32_t packed_b)
{
    packed_a = SkApplySign(clear_high_bit(packed_a), SkExtractSign(packed_a));
    packed_b = SkApplySign(clear_high_bit(packed_b), SkExtractSign(packed_b));

    return int_sign(packed_a, packed_b);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#include "SkRandom.h"
#include "SkFloatingPoint.h"

void SkFloat::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    SkFloat a, b, c, d;
    int     n;

    a.setZero();
    n = a.getInt();
    SkASSERT(n == 0);

    b.setInt(5);
    n = b.getInt();
    SkASSERT(n == 5);

    c.setInt(-3);
    n = c.getInt();
    SkASSERT(n == -3);

    d.setAdd(c, b);
    SkDebugf("SkFloat: %d + %d = %d\n", c.getInt(), b.getInt(), d.getInt());

    SkMWCRandom    rand;

    int i;
    for (i = 0; i < 1000; i++)
    {
        float fa, fb;
        int aa = rand.nextS() >> 14;
        int bb = rand.nextS() >> 14;
        a.setInt(aa);
        b.setInt(bb);
        SkASSERT(a.getInt() == aa);
        SkASSERT(b.getInt() == bb);

        c.setAdd(a, b);
        int cc = c.getInt();
        SkASSERT(cc == aa + bb);

        c.setSub(a, b);
        cc = c.getInt();
        SkASSERT(cc == aa - bb);

        aa >>= 5;
        bb >>= 5;
        a.setInt(aa);
        b.setInt(bb);
        c.setMul(a, b);
        cc = c.getInt();
        SkASSERT(cc == aa * bb);
        /////////////////////////////////////

        aa = rand.nextS() >> 11;
        a.setFixed(aa);
        cc = a.getFixed();
        SkASSERT(aa == cc);

        bb = rand.nextS() >> 11;
        b.setFixed(bb);
        cc = b.getFixed();
        SkASSERT(bb == cc);

        cc = SkFixedMul(aa, bb);
        c.setMul(a, b);
        SkFixed dd = c.getFixed();
        int diff = cc - dd;
        SkASSERT(SkAbs32(diff) <= 1);

        fa = (float)aa / 65536.0f;
        fb = (float)bb / 65536.0f;
        a.assertEquals(fa);
        b.assertEquals(fb);
        fa = a.getFloat();
        fb = b.getFloat();

        c.assertEquals(fa * fb, 1);

        c.setDiv(a, b);
        cc = SkFixedDiv(aa, bb);
        dd = c.getFixed();
        diff = cc - dd;
        SkASSERT(SkAbs32(diff) <= 3);

        c.assertEquals(fa / fb, 1);

        SkASSERT((aa == bb) == (a == b));
        SkASSERT((aa != bb) == (a != b));
        SkASSERT((aa < bb) == (a < b));
        SkASSERT((aa <= bb) == (a <= b));
        SkASSERT((aa > bb) == (a > b));
        SkASSERT((aa >= bb) == (a >= b));

        if (aa < 0)
        {
            aa = -aa;
            fa = -fa;
        }
        a.setFixed(aa);
        c.setSqrt(a);
        cc = SkFixedSqrt(aa);
        dd = c.getFixed();
        SkASSERT(dd == cc);

        c.assertEquals(sk_float_sqrt(fa), 2);

        // cuberoot
#if 0
        a.setInt(1);
        a.cubeRoot();
        a.assertEquals(1.0f, 0);
        a.setInt(8);
        a.cubeRoot();
        a.assertEquals(2.0f, 0);
        a.setInt(27);
        a.cubeRoot();
        a.assertEquals(3.0f, 0);
#endif
    }
#endif
}

#endif
