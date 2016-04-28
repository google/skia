/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMathPriv.h"
#include "SkFixed.h"
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
