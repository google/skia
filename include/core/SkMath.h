/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMath_DEFINED
#define SkMath_DEFINED

#include "../private/SkTo.h"
#include "SkTypes.h"

// 64bit -> 32bit utilities

// Handy util that can be passed two ints, and will automatically promote to
// 64bits before the multiply, so the caller doesn't have to remember to cast
// e.g. (int64_t)a * b;
static inline int64_t sk_64_mul(int64_t a, int64_t b) {
    return a * b;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the integer square root of value, with a bias of bitBias
 */
int32_t SkSqrtBits(int32_t value, int bitBias);

/** Return the integer square root of n, treated as a SkFixed (16.16)
 */
#define SkSqrt32(n)         SkSqrtBits(n, 15)

/**
 *  Returns (value < 0 ? 0 : value) efficiently (i.e. no compares or branches)
 */
static inline int SkClampPos(int value) {
    return value & ~(value >> 31);
}

/** Given an integer and a positive (max) integer, return the value
 *  pinned against 0 and max, inclusive.
 *  @param value    The value we want returned pinned between [0...max]
 *  @param max      The positive max value
 *  @return 0 if value < 0, max if value > max, else value
 */
static inline int SkClampMax(int value, int max) {
    // ensure that max is positive
    SkASSERT(max >= 0);
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

/**
 *  Returns true if value is a power of 2. Does not explicitly check for
 *  value <= 0.
 */
template <typename T> constexpr inline bool SkIsPow2(T value) {
    return (value & (value - 1)) == 0;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return a*b/((1 << shift) - 1), rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767 and shift is > 0 and <= 8
 */
static inline unsigned SkMul16ShiftRound(U16CPU a, U16CPU b, int shift) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    SkASSERT(shift > 0 && shift <= 8);
    unsigned prod = a*b + (1 << (shift - 1));
    return (prod + (prod >> shift)) >> shift;
}

/**
 *  Return a*b/255, rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767.
 */
static inline U8CPU SkMulDiv255Round(U16CPU a, U16CPU b) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    unsigned prod = a*b + 128;
    return (prod + (prod >> 8)) >> 8;
}

/**
 * Stores numer/denom and numer%denom into div and mod respectively.
 */
template <typename In, typename Out>
inline void SkTDivMod(In numer, In denom, Out* div, Out* mod) {
#ifdef SK_CPU_ARM32
    // If we wrote this as in the else branch, GCC won't fuse the two into one
    // divmod call, but rather a div call followed by a divmod.  Silly!  This
    // version is just as fast as calling __aeabi_[u]idivmod manually, but with
    // prettier code.
    //
    // This benches as around 2x faster than the code in the else branch.
    const In d = numer/denom;
    *div = static_cast<Out>(d);
    *mod = static_cast<Out>(numer-d*denom);
#else
    // On x86 this will just be a single idiv.
    *div = static_cast<Out>(numer/denom);
    *mod = static_cast<Out>(numer%denom);
#endif
}

#endif
