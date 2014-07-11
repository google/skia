/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMathPriv_DEFINED
#define SkMathPriv_DEFINED

#include "SkMath.h"

#ifdef SK_BUILD_FOR_IOS
// The iOS ARM processor discards small denormalized numbers to go faster.
// Algorithms that rely on denormalized numbers need alternative implementations.
#define SK_DISCARD_DENORMALIZED_FOR_SPEED
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
    unsigned prod = SkMulS16(a, b) + 1;
    return (prod + (prod >> 8)) >> 8;
}

/** Return (a*b)/255, taking the ceiling of any fractional bits. Only valid if
 both a and b are 0..255. The expected result equals (a * b + 254) / 255.
 */
static inline U8CPU SkMulDiv255Ceiling(U8CPU a, U8CPU b) {
    SkASSERT((uint8_t)a == a);
    SkASSERT((uint8_t)b == b);
    unsigned prod = SkMulS16(a, b) + 255;
    return (prod + (prod >> 8)) >> 8;
}

/** Just the rounding step in SkDiv255Round: round(value / 255)
 */
static inline unsigned SkDiv255Round(unsigned prod) {
    prod += 128;
    return (prod + (prod >> 8)) >> 8;
}

#endif
