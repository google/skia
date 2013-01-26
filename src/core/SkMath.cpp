/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMathPriv.h"
#include "SkCordic.h"
#include "SkFloatBits.h"
#include "SkFloatingPoint.h"
#include "Sk64.h"
#include "SkScalar.h"

#ifdef SK_SCALAR_IS_FLOAT
    const uint32_t gIEEENotANumber = 0x7FFFFFFF;
    const uint32_t gIEEEInfinity = 0x7F800000;
    const uint32_t gIEEENegativeInfinity = 0xFF800000;
#endif

#define sub_shift(zeros, x, n)  \
    zeros -= n;                 \
    x >>= n

int SkCLZ_portable(uint32_t x) {
    if (x == 0) {
        return 32;
    }

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

int32_t SkMulDiv(int32_t numer1, int32_t numer2, int32_t denom) {
    SkASSERT(denom);

    Sk64 tmp;
    tmp.setMul(numer1, numer2);
    tmp.div(denom, Sk64::kTrunc_DivOption);
    return tmp.get32();
}

int32_t SkMulShift(int32_t a, int32_t b, unsigned shift) {
    int sign = SkExtractSign(a ^ b);

    if (shift > 63) {
        return sign;
    }

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

    if (sign < 0) {
        hi = -hi - Sk32ToBool(lo);
        lo = 0 - lo;
    }

    if (shift == 0) {
#ifdef SK_DEBUGx
        SkASSERT(((int32_t)lo >> 31) == hi);
#endif
        return lo;
    } else if (shift >= 32) {
        return hi >> (shift - 32);
    } else {
#ifdef SK_DEBUGx
        int32_t tmp = hi >> shift;
        SkASSERT(tmp == 0 || tmp == -1);
#endif
        // we want (hi << (32 - shift)) | (lo >> shift) but rounded
        int roundBit = (lo >> (shift - 1)) & 1;
        return ((hi << (32 - shift)) | (lo >> shift)) + roundBit;
    }
}

SkFixed SkFixedMul_portable(SkFixed a, SkFixed b) {
#if 0
    Sk64    tmp;

    tmp.setMul(a, b);
    tmp.shiftRight(16);
    return tmp.fLo;
#elif defined(SkLONGLONG)
    return static_cast<SkFixed>((SkLONGLONG)a * b >> 16);
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

SkFract SkFractMul_portable(SkFract a, SkFract b) {
#if 0
    Sk64 tmp;
    tmp.setMul(a, b);
    return tmp.getFract();
#elif defined(SkLONGLONG)
    return static_cast<SkFract>((SkLONGLONG)a * b >> 30);
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

int SkFixedMulCommon(SkFixed a, int b, int bias) {
    // this function only works if b is 16bits
    SkASSERT(b == (int16_t)b);
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

SkFixed SkFixedFastInvert(SkFixed x) {
/*  Adapted (stolen) from gglRecip()
*/

    if (x == SK_Fixed1) {
        return SK_Fixed1;
    }

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

///////////////////////////////////////////////////////////////////////////////

#define DIVBITS_ITER(n)                                 \
    case n:                                             \
        if ((numer = (numer << 1) - denom) >= 0)        \
            result |= 1 << (n - 1); else numer += denom

int32_t SkDivBits(int32_t numer, int32_t denom, int shift_bias) {
    SkASSERT(denom != 0);
    if (numer == 0) {
        return 0;
    }

    // make numer and denom positive, and sign hold the resulting sign
    int32_t sign = SkExtractSign(numer ^ denom);
    numer = SkAbs32(numer);
    denom = SkAbs32(denom);

    int nbits = SkCLZ(numer) - 1;
    int dbits = SkCLZ(denom) - 1;
    int bits = shift_bias - nbits + dbits;

    if (bits < 0) {  // answer will underflow
        return 0;
    }
    if (bits > 31) {  // answer will overflow
        return SkApplySign(SK_MaxS32, sign);
    }

    denom <<= dbits;
    numer <<= nbits;

    SkFixed result = 0;

    // do the first one
    if ((numer -= denom) >= 0) {
        result = 1;
    } else {
        numer += denom;
    }

    // Now fall into our switch statement if there are more bits to compute
    if (bits > 0) {
        // make room for the rest of the answer bits
        result <<= bits;
        switch (bits) {
            DIVBITS_ITER(31); DIVBITS_ITER(30); DIVBITS_ITER(29);
            DIVBITS_ITER(28); DIVBITS_ITER(27); DIVBITS_ITER(26);
            DIVBITS_ITER(25); DIVBITS_ITER(24); DIVBITS_ITER(23);
            DIVBITS_ITER(22); DIVBITS_ITER(21); DIVBITS_ITER(20);
            DIVBITS_ITER(19); DIVBITS_ITER(18); DIVBITS_ITER(17);
            DIVBITS_ITER(16); DIVBITS_ITER(15); DIVBITS_ITER(14);
            DIVBITS_ITER(13); DIVBITS_ITER(12); DIVBITS_ITER(11);
            DIVBITS_ITER(10); DIVBITS_ITER( 9); DIVBITS_ITER( 8);
            DIVBITS_ITER( 7); DIVBITS_ITER( 6); DIVBITS_ITER( 5);
            DIVBITS_ITER( 4); DIVBITS_ITER( 3); DIVBITS_ITER( 2);
            // we merge these last two together, makes GCC make better ARM
            default:
            DIVBITS_ITER( 1);
        }
    }

    if (result < 0) {
        result = SK_MaxS32;
    }
    return SkApplySign(result, sign);
}

/*  mod(float numer, float denom) seems to always return the sign
    of the numer, so that's what we do too
*/
SkFixed SkFixedMod(SkFixed numer, SkFixed denom) {
    int sn = SkExtractSign(numer);
    int sd = SkExtractSign(denom);

    numer = SkApplySign(numer, sn);
    denom = SkApplySign(denom, sd);

    if (numer < denom) {
        return SkApplySign(numer, sn);
    } else if (numer == denom) {
        return 0;
    } else {
        SkFixed div = SkFixedDiv(numer, denom);
        return SkApplySign(SkFixedMul(denom, div & 0xFFFF), sn);
    }
}

/* www.worldserver.com/turk/computergraphics/FixedSqrt.pdf
*/
int32_t SkSqrtBits(int32_t x, int count) {
    SkASSERT(x >= 0 && count > 0 && (unsigned)count <= 30);

    uint32_t    root = 0;
    uint32_t    remHi = 0;
    uint32_t    remLo = x;

    do {
        root <<= 1;

        remHi = (remHi<<2) | (remLo>>30);
        remLo <<= 2;

        uint32_t testDiv = (root << 1) + 1;
        if (remHi >= testDiv) {
            remHi -= testDiv;
            root++;
        }
    } while (--count >= 0);

    return root;
}

int32_t SkCubeRootBits(int32_t value, int bits) {
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
        if (guess < curr) {
            curr -= guess + 1;
            root |= 1;
        }
        curr = (curr << 3) | ((uint32_t)value >> 29);
        value <<= 3;
    } while (--bits);

    return SkApplySign(root, sign);
}

SkFixed SkFixedMean(SkFixed a, SkFixed b) {
    Sk64 tmp;

    tmp.setMul(a, b);
    return tmp.getSqrt();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT
float SkScalarSinCos(float radians, float* cosValue) {
    float sinValue = sk_float_sin(radians);

    if (cosValue) {
        *cosValue = sk_float_cos(radians);
        if (SkScalarNearlyZero(*cosValue)) {
            *cosValue = 0;
        }
    }

    if (SkScalarNearlyZero(sinValue)) {
        sinValue = 0;
    }
    return sinValue;
}
#endif

#define INTERP_SINTABLE
#define BUILD_TABLE_AT_RUNTIMEx

#define kTableSize  256

#ifdef BUILD_TABLE_AT_RUNTIME
    static uint16_t gSkSinTable[kTableSize];

    static void build_sintable(uint16_t table[]) {
        for (int i = 0; i < kTableSize; i++) {
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
static SkFixed interp_table(const uint16_t table[], int index, int partial255) {
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

SkFixed SkFixedSinCos(SkFixed radians, SkFixed* cosValuePtr) {
    SkASSERT(SK_ARRAY_COUNT(gSkSinTable) == kTableSize);

#ifdef BUILD_TABLE_AT_RUNTIME
    static bool gFirstTime = true;
    if (gFirstTime) {
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

    if (index == 0) {
        sinValue = 0;
        cosValue = SK_Fixed1;
    } else {
        sinValue = gSkSinTable[index];
        cosValue = gSkSinTable[kTableSize - index];
    }
    int quad = ((unsigned)radians / kTableSize) & 3;
#endif

    if (quad & 1) {
        SkTSwap<SkFixed>(sinValue, cosValue);
    }
    if (quad & 2) {
        sinSign = ~sinSign;
    }
    if (((quad - 1) & 2) == 0) {
        cosSign = ~cosSign;
    }

    // restore the sign for negative angles
    sinValue = SkApplySign(sinValue, sinSign);
    cosValue = SkApplySign(cosValue, cosSign);

#ifdef SK_DEBUG
    if (1) {
        SkFixed sin2 = SkFixedMul(sinValue, sinValue);
        SkFixed cos2 = SkFixedMul(cosValue, cosValue);
        int diff = cos2 + sin2 - SK_Fixed1;
        SkASSERT(SkAbs32(diff) <= 7);
    }
#endif

    if (cosValuePtr) {
        *cosValuePtr = cosValue;
    }
    return sinValue;
}

///////////////////////////////////////////////////////////////////////////////

SkFixed SkFixedTan(SkFixed radians) { return SkCordicTan(radians); }
SkFixed SkFixedASin(SkFixed x) { return SkCordicASin(x); }
SkFixed SkFixedACos(SkFixed x) { return SkCordicACos(x); }
SkFixed SkFixedATan2(SkFixed y, SkFixed x) { return SkCordicATan2(y, x); }
SkFixed SkFixedExp(SkFixed x) { return SkCordicExp(x); }
SkFixed SkFixedLog(SkFixed x) { return SkCordicLog(x); }
