/* libs/corecg/SkMath.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkMath.h"
#include "SkCordic.h"
#include "SkFloatingPoint.h"
#include "Sk64.h"

#ifdef SK_SCALAR_IS_FLOAT
    const uint32_t gIEEENotANumber = 0x7FFFFFFF;
    const uint32_t gIEEEInfinity = 0x7F800000;
#endif

#define sub_shift(zeros, x, n)  \
    zeros -= n;                 \
    x >>= n
    
int SkCLZ_portable(uint32_t x)
{
    if (x == 0)
        return 32;

#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    int zeros = 31;
    if (x & 0xFFFF0000) {
        sub_shift(zeros, x, 16);
    }
    if (x & 0xFF00) {
        sub_shift(zeros, x, 8);
    }
    if (x & 0xF0) {
        sub_shift(zeros, x, 4);
    }
    if (x & 0xC) {
        sub_shift(zeros, x, 2);
    }
    if (x & 0x2) {
        sub_shift(zeros, x, 1);
    }
#else
    int zeros = ((x >> 16) - 1) >> 31 << 4;
    x <<= zeros;

    int nonzero = ((x >> 24) - 1) >> 31 << 3;
    zeros += nonzero;
    x <<= nonzero;

    nonzero = ((x >> 28) - 1) >> 31 << 2;
    zeros += nonzero;
    x <<= nonzero;

    nonzero = ((x >> 30) - 1) >> 31 << 1;
    zeros += nonzero;
    x <<= nonzero;

    zeros += (~x) >> 31;
#endif

    return zeros;
}

int32_t SkMulDiv(int32_t numer1, int32_t numer2, int32_t denom)
{
    SkASSERT(denom);

    Sk64 tmp;
    tmp.setMul(numer1, numer2);
    tmp.div(denom, Sk64::kTrunc_DivOption);
    return tmp.get32();
}

int32_t SkMulShift(int32_t a, int32_t b, unsigned shift)
{
    int sign = SkExtractSign(a ^ b);

    if (shift > 63)
        return sign;

    a = SkAbs32(a);
    b = SkAbs32(b);

    uint32_t ah = a >> 16;
    uint32_t al = a & 0xFFFF;
    uint32_t bh = b >> 16;
    uint32_t bl = b & 0xFFFF;

    uint32_t A = ah * bh;
    uint32_t B = ah * bl + al * bh;
    uint32_t C = al * bl;

    /*  [  A  ]
           [  B  ]
              [  C  ]
    */
    uint32_t lo = C + (B << 16);
    int32_t  hi = A + (B >> 16) + (lo < C);

    if (sign < 0)
    {
        hi = -hi - Sk32ToBool(lo);
        lo = 0 - lo;
    }

    if (shift == 0)
    {
#ifdef SK_DEBUGx
        SkASSERT(((int32_t)lo >> 31) == hi);
#endif
        return lo;
    }
    else if (shift >= 32)
        return hi >> (shift - 32);
    else
    {
#ifdef SK_DEBUGx
        int32_t tmp = hi >> shift;
        SkASSERT(tmp == 0 || tmp == -1);
#endif
        return (hi << (32 - shift)) | (lo >> shift);
    }
}

SkFixed SkFixedMul_portable(SkFixed a, SkFixed b)
{
#if 0
    Sk64    tmp;

    tmp.setMul(a, b);
    tmp.shiftRight(16);
    return tmp.fLo;
#else
    int sa = SkExtractSign(a);
    int sb = SkExtractSign(b);
    // now make them positive
    a = SkApplySign(a, sa);
    b = SkApplySign(b, sb);

    uint32_t    ah = a >> 16;
    uint32_t    al = a & 0xFFFF;
    uint32_t bh = b >> 16;
    uint32_t bl = b & 0xFFFF;

    uint32_t R = ah * b + al * bh + (al * bl >> 16);

    return SkApplySign(R, sa ^ sb);
#endif
}

SkFract SkFractMul_portable(SkFract a, SkFract b)
{
#if 0
    Sk64 tmp;
    tmp.setMul(a, b);
    return tmp.getFract();
#else
    int sa = SkExtractSign(a);
    int sb = SkExtractSign(b);
    // now make them positive
    a = SkApplySign(a, sa);
    b = SkApplySign(b, sb);

    uint32_t ah = a >> 16;
    uint32_t al = a & 0xFFFF;
    uint32_t bh = b >> 16;
    uint32_t bl = b & 0xFFFF;

    uint32_t A = ah * bh;
    uint32_t B = ah * bl + al * bh;
    uint32_t C = al * bl;

    /*  [  A  ]
           [  B  ]
              [  C  ]
    */
    uint32_t Lo = C + (B << 16);
    uint32_t Hi = A + (B >>16) + (Lo < C);

    SkASSERT((Hi >> 29) == 0);  // else overflow

    int32_t R = (Hi << 2) + (Lo >> 30);

    return SkApplySign(R, sa ^ sb);
#endif
}

int SkFixedMulCommon(SkFixed a, int b, int bias)
{
    // this function only works if b is 16bits
    SkASSERT(b == (S16)b);
    SkASSERT(b >= 0);

    int sa = SkExtractSign(a);
    a = SkApplySign(a, sa);
    uint32_t ah = a >> 16;
    uint32_t al = a & 0xFFFF;
    uint32_t R = ah * b + ((al * b + bias) >> 16);
    return SkApplySign(R, sa);
}

#ifdef SK_DEBUGx
    #define TEST_FASTINVERT
#endif

SkFixed SkFixedFastInvert(SkFixed x)
{
/*  Adapted (stolen) from Mathias' gglRecip()
*/

    if (x == SK_Fixed1)
        return SK_Fixed1;

    int      sign = SkExtractSign(x);
    uint32_t a = SkApplySign(x, sign);

    if (a <= 2) {
        return SkApplySign(SK_MaxS32, sign);
    }

#ifdef TEST_FASTINVERT
    SkFixed orig = a;
    uint32_t slow = SkFixedDiv(SK_Fixed1, a);
#endif

    // normalize a
    int lz = SkCLZ(a);
    a = a << lz >> 16;

    // compute 1/a approximation (0.5 <= a < 1.0) 
    uint32_t r = 0x17400 - a;      // (2.90625 (~2.914) - 2*a) >> 1

    // Newton-Raphson iteration:
    // x = r*(2 - a*r) = ((r/2)*(1 - a*r/2))*4
    r = ( (0x10000 - ((a*r)>>16)) * r ) >> 15;
    r = ( (0x10000 - ((a*r)>>16)) * r ) >> (30 - lz);

#ifdef TEST_FASTINVERT
    SkDebugf("SkFixedFastInvert(%x %g) = %x %g Slow[%x %g]\n",
                orig, orig/65536.,
                r, r/65536.,
                slow, slow/65536.);
#endif

    return SkApplySign(r, sign);
}

int32_t SkDivBits(int32_t numer, int32_t denom, int shift_bias)
{
    SkASSERT(denom != 0);
    if (numer == 0)
        return 0;
        
    SkFixed result;
    int32_t sign;

    // make numer and denom positive, and sign hold the resulting sign
    sign = SkExtractSign(numer ^ denom);
    numer = SkAbs32(numer);
    denom = SkAbs32(denom);

#if 0   // faster assuming we have HW divide
    if ((numer >> (32 - shift_bias)) == 0)
    {
        result = (uint32_t)(numer << shift_bias) / denom;
    }
    else
#endif
    {
        int nbits = SkCLZ(numer) - 1;
        int dbits = SkCLZ(denom) - 1;
        int bits = shift_bias - nbits + dbits;

        if (bits <= 0)  // answer will underflow
            return 0;
        if (bits > 31)  // answer will overflow
            return SkApplySign(SK_MaxS32, sign);

        denom <<= dbits;
        numer <<= nbits;
        result = 0;
        do {
            result <<= 1;
    #ifdef SK_CPU_HAS_CONDITIONAL_INSTR
            if ((uint32_t)denom <= (uint32_t)numer)
            {
                numer -= denom;
                result |= 1;
            }
    #else
            int32_t diff = (denom - numer - 1) >> 31;
            result -= diff;
            numer -= denom & diff;
    #endif
            numer <<= 1;
        } while (--bits >= 0);
    }

    if (result < 0)
        result = SK_MaxS32;
    return SkApplySign(result, sign);
}

/*  mod(float numer, float denom) seems to always return the sign
    of the numer, so that's what we do too
*/
SkFixed SkFixedMod(SkFixed numer, SkFixed denom)
{
    int sn = SkExtractSign(numer);
    int sd = SkExtractSign(denom);

    numer = SkApplySign(numer, sn);
    denom = SkApplySign(denom, sd);
    
    if (numer < denom)
        return SkApplySign(numer, sn);
    else if (numer == denom)
        return 0;
    else
    {
        SkFixed div = SkFixedDiv(numer, denom);
        return SkApplySign(SkFixedMul(denom, div & 0xFFFF), sn);
    }
}

/* www.worldserver.com/turk/computergraphics/FixedSqrt.pdf
*/
int32_t SkSqrtBits(int32_t x, int count)
{
    SkASSERT(x >= 0 && count > 0 && (unsigned)count <= 30);

    uint32_t    root = 0;
    uint32_t    remHi = 0;
    uint32_t    remLo = x;

    do {
        root <<= 1;

        remHi = (remHi<<2) | (remLo>>30);
        remLo <<= 2;

        uint32_t testDiv = (root << 1) + 1;
        if (remHi >= testDiv)
        {
            remHi -= testDiv;
            root++;
        }
    } while (--count >= 0);

    return root;
}

int32_t SkCubeRootBits(int32_t value, int bits)
{
    SkASSERT(bits > 0);

    int sign = SkExtractSign(value);
    value = SkApplySign(value, sign);

    uint32_t root = 0;
    uint32_t curr = (uint32_t)value >> 30;
    value <<= 2;

    do {
        root <<= 1;
        uint32_t guess = root * root + root;
        guess = (guess << 1) + guess;   // guess *= 3
        if (guess < curr)
        {   curr -= guess + 1;
            root |= 1;
        }
        curr = (curr << 3) | ((uint32_t)value >> 29);
        value <<= 3;
    } while (--bits);

    return SkApplySign(root, sign);
}

SkFixed SkFixedMean(SkFixed a, SkFixed b)
{
    Sk64 tmp;
    
    tmp.setMul(a, b);
    return tmp.getSqrt();
}

////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT
float SkScalarSinCos(float radians, float* cosValue)
{
    float sinValue = sk_float_sin(radians);

    if (cosValue)
    {
        *cosValue = sk_float_cos(radians);
        if (SkScalarNearlyZero(*cosValue))
            *cosValue = 0;
    }

    if (SkScalarNearlyZero(sinValue))
        sinValue = 0;

    return sinValue;
}
#endif

#define INTERP_SINTABLE
#define BUILD_TABLE_AT_RUNTIMEx

#define kTableSize  256

#ifdef BUILD_TABLE_AT_RUNTIME
    static uint16_t gSkSinTable[kTableSize];

    static void build_sintable(uint16_t table[])
    {
        for (int i = 0; i < kTableSize; i++)
        {
            double  rad = i * 3.141592653589793 / (2*kTableSize);
            double  val = sin(rad);
            int     ival = (int)(val * SK_Fixed1);
            table[i] = SkToU16(ival);
        }
    }
#else
    #include "SkSinTable.h"
#endif

#define SK_Fract1024SizeOver2PI     0x28BE60    /* floatToFract(1024 / 2PI) */

#ifdef INTERP_SINTABLE
static SkFixed interp_table(const uint16_t table[], int index, int partial255)
{
    SkASSERT((unsigned)index < kTableSize);
    SkASSERT((unsigned)partial255 <= 255);

    SkFixed lower = table[index];
    SkFixed upper = (index == kTableSize - 1) ? SK_Fixed1 : table[index + 1];

    SkASSERT(lower < upper);
    SkASSERT(lower >= 0);
    SkASSERT(upper <= SK_Fixed1);

    partial255 += (partial255 >> 7);
    return lower + ((upper - lower) * partial255 >> 8);
}
#endif

SkFixed SkFixedSinCos(SkFixed radians, SkFixed* cosValuePtr)
{
    SkASSERT(SK_ARRAY_COUNT(gSkSinTable) == kTableSize);

#ifdef BUILD_TABLE_AT_RUNTIME
    static bool gFirstTime = true;
    if (gFirstTime)
    {
        build_sintable(gSinTable);
        gFirstTime = false;
    }
#endif

    // make radians positive
    SkFixed sinValue, cosValue;
    int32_t cosSign = 0;
    int32_t sinSign = SkExtractSign(radians);
    radians = SkApplySign(radians, sinSign);
    // scale it to 0...1023 ...

#ifdef INTERP_SINTABLE
    radians = SkMulDiv(radians, 2 * kTableSize * 256, SK_FixedPI);
    int findex = radians & (kTableSize * 256 - 1);
    int index = findex >> 8;
    int partial = findex & 255;
    sinValue = interp_table(gSkSinTable, index, partial);

    findex = kTableSize * 256 - findex - 1;
    index = findex >> 8;
    partial = findex & 255;
    cosValue = interp_table(gSkSinTable, index, partial);

    int quad = ((unsigned)radians / (kTableSize * 256)) & 3;
#else
    radians = SkMulDiv(radians, 2 * kTableSize, SK_FixedPI);
    int     index = radians & (kTableSize - 1);

    if (index == 0)
    {
        sinValue = 0;
        cosValue = SK_Fixed1;
    }
    else
    {
        sinValue = gSkSinTable[index];
        cosValue = gSkSinTable[kTableSize - index];
    }
    int quad = ((unsigned)radians / kTableSize) & 3;
#endif

    if (quad & 1)
        SkTSwap<SkFixed>(sinValue, cosValue);
    if (quad & 2)
        sinSign = ~sinSign;
    if (((quad - 1) & 2) == 0)
        cosSign = ~cosSign;

    // restore the sign for negative angles
    sinValue = SkApplySign(sinValue, sinSign);
    cosValue = SkApplySign(cosValue, cosSign);

#ifdef SK_DEBUG
    if (1)
    {
        SkFixed sin2 = SkFixedMul(sinValue, sinValue);
        SkFixed cos2 = SkFixedMul(cosValue, cosValue);
        int diff = cos2 + sin2 - SK_Fixed1;
        SkASSERT(SkAbs32(diff) <= 7);
    }
#endif

    if (cosValuePtr)
        *cosValuePtr = cosValue;
    return sinValue;
}

////////////////////////////////////////////////////////////////////////////

SkFixed SkFixedTan(SkFixed radians) { return SkCordicTan(radians); }
SkFixed SkFixedASin(SkFixed x) { return SkCordicASin(x); }
SkFixed SkFixedACos(SkFixed x) { return SkCordicACos(x); }
SkFixed SkFixedATan2(SkFixed y, SkFixed x) { return SkCordicATan2(y, x); }
SkFixed SkFixedExp(SkFixed x) { return SkCordicExp(x); }
SkFixed SkFixedLog(SkFixed x) { return SkCordicLog(x); }

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#include "SkRandom.h"

#ifdef SK_CAN_USE_LONGLONG
static int symmetric_fixmul(int a, int b)
{
    int sa = SkExtractSign(a);
    int sb = SkExtractSign(b);

    a = SkApplySign(a, sa);
    b = SkApplySign(b, sb);

#if 1
    int c = (int)(((SkLONGLONG)a * b) >> 16);
    
    return SkApplySign(c, sa ^ sb);
#else
    SkLONGLONG ab = (SkLONGLONG)a * b;
    if (sa ^ sb)
        ab = -ab;
    return ab >> 16;
#endif
}
#endif

#include "SkPoint.h"

#ifdef SK_SUPPORT_UNITTEST
static void check_length(const SkPoint& p, SkScalar targetLen)
{
    float x = SkScalarToFloat(p.fX);
    float y = SkScalarToFloat(p.fY);
    float len = sk_float_sqrt(x*x + y*y);
    
    len /= SkScalarToFloat(targetLen);

    SkASSERT(len > 0.999f && len < 1.001f);
}
#endif

void SkMath::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    int         i;
    int32_t     x;
    SkRandom    rand;

    SkToS8(127);    SkToS8(-128);       SkToU8(255);
    SkToS16(32767); SkToS16(-32768);    SkToU16(65535);
    SkToS32(2*1024*1024);   SkToS32(-2*1024*1024);  SkToU32(4*1024*1024);

    SkCordic_UnitTest();

    // these should assert
#if 0
    SkToS8(128);
    SkToS8(-129);
    SkToU8(256);
    SkToU8(-5);

    SkToS16(32768);
    SkToS16(-32769);
    SkToU16(65536);
    SkToU16(-5);

    if (sizeof(size_t) > 4)
    {
        SkToS32(4*1024*1024);
        SkToS32(-4*1024*1024);
        SkToU32(5*1024*1024);
        SkToU32(-5);
    }
#endif

    {
        SkScalar x = SK_ScalarNaN;
        SkASSERT(SkScalarIsNaN(x));
    }

    for (i = 1; i <= 10; i++)
    {
        x = SkCubeRootBits(i*i*i, 11);
        SkASSERT(x == i);
    }

    x = SkFixedSqrt(SK_Fixed1);
    SkASSERT(x == SK_Fixed1);
    x = SkFixedSqrt(SK_Fixed1/4);
    SkASSERT(x == SK_Fixed1/2);
    x = SkFixedSqrt(SK_Fixed1*4);
    SkASSERT(x == SK_Fixed1*2);

    x = SkFractSqrt(SK_Fract1);
    SkASSERT(x == SK_Fract1);
    x = SkFractSqrt(SK_Fract1/4);
    SkASSERT(x == SK_Fract1/2);
    x = SkFractSqrt(SK_Fract1/16);
    SkASSERT(x == SK_Fract1/4);

    for (i = 1; i < 100; i++)
    {
        x = SkFixedSqrt(SK_Fixed1 * i * i);
        SkASSERT(x == SK_Fixed1 * i);
    }

    for (i = 0; i < 1000; i++)
    {
        int value = rand.nextS16();
        int max = rand.nextU16();

        int clamp = SkClampMax(value, max);
        int clamp2 = value < 0 ? 0 : (value > max ? max : value);
        SkASSERT(clamp == clamp2);
    }
    
    for (i = 0; i < 100000; i++)
    {
        SkPoint p;
        
        p.setLength(rand.nextS(), rand.nextS(), SK_Scalar1);
        check_length(p, SK_Scalar1);
        p.setLength(rand.nextS() >> 13, rand.nextS() >> 13, SK_Scalar1);
        check_length(p, SK_Scalar1);
    }

#ifdef SK_CAN_USE_LONGLONG
    for (i = 0; i < 100000; i++)
    {
        SkFixed numer = rand.nextS();
        SkFixed denom = rand.nextS();
        SkFixed result = SkFixedDiv(numer, denom);
        SkLONGLONG check = ((SkLONGLONG)numer << 16) / denom;

        (void)SkCLZ(numer);
        (void)SkCLZ(denom);

        SkASSERT(result != (SkFixed)SK_NaN32);
        if (check > SK_MaxS32)
            check = SK_MaxS32;
        else if (check < -SK_MaxS32)
            check = SK_MinS32;
        SkASSERT(result == (int32_t)check);

        result = SkFractDiv(numer, denom);
        check = ((SkLONGLONG)numer << 30) / denom;

        SkASSERT(result != (SkFixed)SK_NaN32);
        if (check > SK_MaxS32)
            check = SK_MaxS32;
        else if (check < -SK_MaxS32)
            check = SK_MinS32;
        SkASSERT(result == (int32_t)check);

        // make them <= 2^24, so we don't overflow in fixmul
        numer = numer << 8 >> 8;
        denom = denom << 8 >> 8;

        result = SkFixedMul(numer, denom);
        SkFixed r2 = symmetric_fixmul(numer, denom);
        SkASSERT(result == r2);

        result = SkFixedMul(numer, numer);
        r2 = SkFixedSquare(numer);
        SkASSERT(result == r2);
        
#ifdef SK_CAN_USE_FLOAT
        if (numer >= 0 && denom >= 0)
        {
            SkFixed mean = SkFixedMean(numer, denom);
            float fm = sk_float_sqrt(sk_float_abs(SkFixedToFloat(numer) * SkFixedToFloat(denom)));
            SkFixed mean2 = SkFloatToFixed(fm);
            int diff = SkAbs32(mean - mean2);
            SkASSERT(diff <= 1);
        }

        {
            SkFixed mod = SkFixedMod(numer, denom);
            float n = SkFixedToFloat(numer);
            float d = SkFixedToFloat(denom);
            float m = sk_float_mod(n, d);
#if 0
            SkDebugf("%g mod %g = %g [%g]\n",
                    SkFixedToFloat(numer), SkFixedToFloat(denom),
                    SkFixedToFloat(mod), m);
#endif
            SkASSERT(mod == 0 || (mod < 0) == (m < 0)); // ensure the same sign
            int diff = SkAbs32(mod - SkFloatToFixed(m));
            SkASSERT((diff >> 7) == 0);
        }
#endif
    }
#endif

#ifdef SK_CAN_USE_FLOAT
    for (i = 0; i < 100000; i++)
    {
        SkFract x = rand.nextU() >> 1;
        double xx = (double)x / SK_Fract1;
        SkFract xr = SkFractSqrt(x);
        SkFract check = SkFloatToFract(sqrt(xx));
        SkASSERT(xr == check || xr == check-1 || xr == check+1);

        xr = SkFixedSqrt(x);
        xx = (double)x / SK_Fixed1;
        check = SkFloatToFixed(sqrt(xx));
        SkASSERT(xr == check || xr == check-1);

        xr = SkSqrt32(x);
        xx = (double)x;
        check = (int32_t)sqrt(xx);
        SkASSERT(xr == check || xr == check-1);
    }
#endif

#if !defined(SK_SCALAR_IS_FLOAT) && defined(SK_CAN_USE_FLOAT)
    {
        SkFixed s, c;
        s = SkFixedSinCos(0, &c);
        SkASSERT(s == 0);
        SkASSERT(c == SK_Fixed1);
    }

    int maxDiff = 0;
    for (i = 0; i < 10000; i++)
    {
        SkFixed rads = rand.nextS() >> 10;
        double frads = SkFixedToFloat(rads);

        SkFixed s, c;
        s = SkScalarSinCos(rads, &c);

        double fs = sin(frads);
        double fc = cos(frads);

        SkFixed is = SkFloatToFixed(fs);
        SkFixed ic = SkFloatToFixed(fc);

        maxDiff = SkMax32(maxDiff, SkAbs32(is - s));
        maxDiff = SkMax32(maxDiff, SkAbs32(ic - c));
    }
    SkDebugf("SinCos: maximum error = %d\n", maxDiff);
#endif
#endif
}

#endif

