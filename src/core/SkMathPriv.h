/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMathPriv_DEFINED
#define SkMathPriv_DEFINED

#include "SkMath.h"

#if defined(SK_BUILD_FOR_IOS) && (defined(SK_BUILD_FOR_ARM32) || defined(SK_BUILD_FOR_ARM64))
// iOS on ARM starts processes with the Flush-To-Zero (FTZ) and
// Denormals-Are-Zero (DAZ) bits in the fpscr register set.
// Algorithms that rely on denormalized numbers need alternative implementations.
// This can also be controlled in SSE with the MXCSR register,
// x87 with FSTCW/FLDCW, and mips with FCSR. This should be detected at runtime,
// or the library built one way or the other more generally (by the build).
#define SK_CPU_FLUSH_TO_ZERO
#endif

/** Returns -1 if n < 0, else returns 0
 */
#define SkExtractSign(n)    ((int32_t)(n) >> 31)

/** If sign == -1, returns -n, else sign must be 0, and returns n.
 Typically used in conjunction with SkExtractSign().
 */
static inline int32_t SkApplySign(int32_t n, int32_t sign) {
    SkASSERT(sign == 0 || sign == -1);
    return (n ^ sign) - sign;
}

/** Return x with the sign of y */
static inline int32_t SkCopySign32(int32_t x, int32_t y) {
    return SkApplySign(x, SkExtractSign(x ^ y));
}

/** Given a positive value and a positive max, return the value
 pinned against max.
 Note: only works as long as max - value doesn't wrap around
 @return max if value >= max, else value
 */
static inline unsigned SkClampUMax(unsigned value, unsigned max) {
    if (value > max) {
        value = max;
    }
    return value;
}

///////////////////////////////////////////////////////////////////////////////

/** Return a*b/255, truncating away any fractional bits. Only valid if both
 a and b are 0..255
 */
static inline U8CPU SkMulDiv255Trunc(U8CPU a, U8CPU b) {
    SkASSERT((uint8_t)a == a);
    SkASSERT((uint8_t)b == b);
    unsigned prod = a*b + 1;
    return (prod + (prod >> 8)) >> 8;
}

/** Return (a*b)/255, taking the ceiling of any fractional bits. Only valid if
 both a and b are 0..255. The expected result equals (a * b + 254) / 255.
 */
static inline U8CPU SkMulDiv255Ceiling(U8CPU a, U8CPU b) {
    SkASSERT((uint8_t)a == a);
    SkASSERT((uint8_t)b == b);
    unsigned prod = a*b + 255;
    return (prod + (prod >> 8)) >> 8;
}

/** Just the rounding step in SkDiv255Round: round(value / 255)
 */
static inline unsigned SkDiv255Round(unsigned prod) {
    prod += 128;
    return (prod + (prod >> 8)) >> 8;
}

static inline float SkPinToUnitFloat(float x) {
    return SkTMin(SkTMax(x, 0.0f), 1.0f);
}

/**
 * Swap byte order of a 4-byte value, e.g. 0xaarrggbb -> 0xbbggrraa.
 */
#if defined(_MSC_VER)
    #include <intrin.h>
    static inline uint32_t SkBSwap32(uint32_t v) { return _byteswap_ulong(v); }
#else
    static inline uint32_t SkBSwap32(uint32_t v) { return __builtin_bswap32(v); }
#endif

//! Returns the number of leading zero bits (0...32)
int SkCLZ_portable(uint32_t);

#ifndef SkCLZ
    #if defined(SK_BUILD_FOR_WIN32)
        #include <intrin.h>

        static inline int SkCLZ(uint32_t mask) {
            if (mask) {
                unsigned long index;
                _BitScanReverse(&index, mask);
                // Suppress this bogus /analyze warning. The check for non-zero
                // guarantees that _BitScanReverse will succeed.
#pragma warning(suppress : 6102) // Using 'index' from failed function call
                return index ^ 0x1F;
            } else {
                return 32;
            }
        }
    #elif defined(SK_CPU_ARM32) || defined(__GNUC__) || defined(__clang__)
        static inline int SkCLZ(uint32_t mask) {
            // __builtin_clz(0) is undefined, so we have to detect that case.
            return mask ? __builtin_clz(mask) : 32;
        }
    #else
        #define SkCLZ(x)    SkCLZ_portable(x)
    #endif
#endif

/**
 *  Returns the smallest power-of-2 that is >= the specified value. If value
 *  is already a power of 2, then it is returned unchanged. It is undefined
 *  if value is <= 0.
 */
static inline int SkNextPow2(int value) {
    SkASSERT(value > 0);
    return 1 << (32 - SkCLZ(value - 1));
}

/**
*  Returns the largest power-of-2 that is <= the specified value. If value
*  is already a power of 2, then it is returned unchanged. It is undefined
*  if value is <= 0.
*/
static inline int SkPrevPow2(int value) {
    SkASSERT(value > 0);
    return 1 << (32 - SkCLZ(value >> 1));
}

/**
 *  Returns the log2 of the specified value, were that value to be rounded up
 *  to the next power of 2. It is undefined to pass 0. Examples:
 *  SkNextLog2(1) -> 0
 *  SkNextLog2(2) -> 1
 *  SkNextLog2(3) -> 2
 *  SkNextLog2(4) -> 2
 *  SkNextLog2(5) -> 3
 */
static inline int SkNextLog2(uint32_t value) {
    SkASSERT(value != 0);
    return 32 - SkCLZ(value - 1);
}

/**
*  Returns the log2 of the specified value, were that value to be rounded down
*  to the previous power of 2. It is undefined to pass 0. Examples:
*  SkPrevLog2(1) -> 0
*  SkPrevLog2(2) -> 1
*  SkPrevLog2(3) -> 1
*  SkPrevLog2(4) -> 2
*  SkPrevLog2(5) -> 2
*/
static inline int SkPrevLog2(uint32_t value) {
    SkASSERT(value != 0);
    return 32 - SkCLZ(value >> 1);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the next power of 2 >= n.
 */
static inline uint32_t GrNextPow2(uint32_t n) {
    return n ? (1 << (32 - SkCLZ(n - 1))) : 1;
}

/**
 * Returns the next power of 2 >= n or n if the next power of 2 can't be represented by size_t.
 */
static inline size_t GrNextSizePow2(size_t n) {
    constexpr int kNumSizeTBits = 8 * sizeof(size_t);
    constexpr size_t kHighBitSet = size_t(1) << (kNumSizeTBits - 1);

    if (!n) {
        return 1;
    } else if (n >= kHighBitSet) {
        return n;
    }

    n--;
    uint32_t shift = 1;
    while (shift < kNumSizeTBits) {
        n |= n >> shift;
        shift <<= 1;
    }
    return n + 1;
}

#endif
