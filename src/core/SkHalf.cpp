/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkHalf.h"
#include "SkFloatBits.h"

uint16_t halfMantissa(SkHalf h) {
    return h & 0x03ff;
}

uint16_t halfExponent(SkHalf h) {
    return (h >> 10) & 0x001f;
}

uint16_t halfSign(SkHalf h) {
    return h >> 15;
}

union FloatUIntUnion {
    uint32_t fUInt;    // this must come first for the initializations below to work
    float    fFloat;
};

// based on Fabien Giesen's float_to_half_fast3()
// see https://gist.github.com/rygorous/2156668
SkHalf SkFloatToHalf(float f) {
    static const uint32_t f32infty = { 255 << 23 };
    static const uint32_t f16infty = { 31 << 23 };
    static const FloatUIntUnion magic = { 15 << 23 };
    static const uint32_t sign_mask = 0x80000000u;
    static const uint32_t round_mask = ~0xfffu;
    SkHalf o = 0;

    FloatUIntUnion floatUnion;
    floatUnion.fFloat = f;

    uint32_t sign = floatUnion.fUInt & sign_mask;
    floatUnion.fUInt ^= sign;

    // NOTE all the integer compares in this function can be safely
    // compiled into signed compares since all operands are below
    // 0x80000000. Important if you want fast straight SSE2 code
    // (since there's no unsigned PCMPGTD).

    // Inf or NaN (all exponent bits set)
    if (floatUnion.fUInt >= f32infty)
        // NaN->qNaN and Inf->Inf
        o = (floatUnion.fUInt > f32infty) ? 0x7e00 : 0x7c00;
    // (De)normalized number or zero
    else {
        floatUnion.fUInt &= round_mask;
        floatUnion.fFloat *= magic.fFloat;
        floatUnion.fUInt -= round_mask;
        // Clamp to signed infinity if overflowed
        if (floatUnion.fUInt > f16infty) {
            floatUnion.fUInt = f16infty;
        }

        o = floatUnion.fUInt >> 13; // Take the bits!
    }

    o |= sign >> 16;
    return o;
}

// based on Fabien Giesen's half_to_float_fast2()
// see https://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/
float SkHalfToFloat(SkHalf h) {
    static const FloatUIntUnion magic = { 126 << 23 };
    FloatUIntUnion o;

    if (halfExponent(h) == 0)
    {
        // Zero / Denormal
        o.fUInt = magic.fUInt + halfMantissa(h);
        o.fFloat -= magic.fFloat;
    }
    else
    {
        // Set mantissa
        o.fUInt = halfMantissa(h) << 13;
        // Set exponent
        if (halfExponent(h) == 0x1f)
            // Inf/NaN
            o.fUInt |= (255 << 23);
        else
            o.fUInt |= ((127 - 15 + halfExponent(h)) << 23);
    }

    // Set sign
    o.fUInt |= (halfSign(h) << 31);
    return o.fFloat;
}
