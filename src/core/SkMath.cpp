/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMathPriv.h"
#include "SkFloatBits.h"
#include "SkFloatingPoint.h"
#include "SkScalar.h"

const uint32_t gIEEENotANumber = 0x7FFFFFFF;
const uint32_t gIEEEInfinity = 0x7F800000;
const uint32_t gIEEENegativeInfinity = 0xFF800000;

#define sub_shift(zeros, x, n)  \
    zeros -= n;                 \
    x >>= n

int SkCLZ_portable(uint32_t x) {
    if (x == 0) {
        return 32;
    }

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

    return zeros;
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

///////////////////////////////////////////////////////////////////////////////

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
