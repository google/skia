/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMathPriv_DEFINED
#define SkMathPriv_DEFINED

#include "SkMath.h"

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

/** Computes the 64bit product of a * b, and then shifts the answer down by
 shift bits, returning the low 32bits. shift must be [0..63]
 e.g. to perform a fixedmul, call SkMulShift(a, b, 16)
 */
int32_t SkMulShift(int32_t a, int32_t b, unsigned shift);

/** Return the integer cube root of value, with a bias of bitBias
 */
int32_t SkCubeRootBits(int32_t value, int bitBias);

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
