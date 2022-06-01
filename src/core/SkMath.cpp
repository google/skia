/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "include/private/SkFixed.h"
#include "include/private/SkFloatBits.h"
#include "include/private/SkFloatingPoint.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkSafeMath.h"

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

// Kernighan's method
int SkPopCount_portable(uint32_t n) {
    int count = 0;

    while (n) {
        n &= (n - 1); // Remove the lowest bit in the integer.
        count++;
    }
    return count;
}

#if defined(SK_BUILD_FOR_WIN) && !defined(_M_ARM) && !defined(_M_ARM64)
#include <intrin.h>

int SkPopCount(uint32_t n) {
    static const bool kHasPopCnt = [] {
        static constexpr int kPopCntBit = 0x1 << 23; // Bit 23 is the popcnt feature bit in ECX
        static constexpr int kECX = 2;
        static constexpr int kProcessorInfoAndFeatureBits = 1;

        int info[4];         // contents of the EAX, EBX, ECX, and EDX registers, in that order
        __cpuid(info, kProcessorInfoAndFeatureBits);
        return static_cast<bool>(info[kECX] & kPopCntBit);
    }();

    if (kHasPopCnt) {
        return __popcnt(n);
    } else {
        return SkPopCount_portable(n);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkSafeMath::Add(size_t x, size_t y) {
    SkSafeMath tmp;
    size_t sum = tmp.add(x, y);
    return tmp.ok() ? sum : SIZE_MAX;
}

size_t SkSafeMath::Mul(size_t x, size_t y) {
    SkSafeMath tmp;
    size_t prod = tmp.mul(x, y);
    return tmp.ok() ? prod : SIZE_MAX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool sk_floats_are_unit(const float array[], size_t count) {
    bool is_unit = true;
    for (size_t i = 0; i < count; ++i) {
        is_unit &= (array[i] >= 0) & (array[i] <= 1);
    }
    return is_unit;
}
