/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkHalf.h"
#include "src/base/SkRandom.h"
#include "src/base/SkVx.h"
#include "tests/Test.h"

#include <cmath>
#include <cstdint>
#include <cstring>

// float = s[31] e[30:23] m[22:0]
static constexpr uint32_t kF32_Sign = 1 << 31;
static constexpr uint32_t kF32_Exp  = 255 << 23;
static constexpr uint32_t kF32_Mant = ~(kF32_Sign | kF32_Exp);
static constexpr int      kF32_Bias = 127;

// half  = s[15] e[14:10] m[9:0]
static constexpr uint32_t kF16_Sign = 1 << 15;
static constexpr uint32_t kF16_Exp  = 31 << 10;
static constexpr uint32_t kF16_Mant = ~(kF16_Sign | kF16_Exp);
static constexpr int      kF16_Bias = 15;

DEF_TEST(FloatToHalf, r) {
#if 0
    // Exhaustive test (slow)
    for (uint64_t bits = 0; bits <= 0xffffffff; bits++) {
        if (bits % (1 << 24) == 0) {
            SkDebugf("progress 0x%08X\n", (int) bits);
        }
#else
    // Check all 8-bit exponents and all 10-bit upper mantissas, with a combination of all 0s,
    // all 1s, and random bits in the remaining 13 fractional mantissa bits.
    static constexpr int kTestCount = /*sign*/2 * /*exp*/255 * /*man*/1024 * /*frac*/8;
    SkRandom rand;
    for (int i = 0; i < kTestCount; ++i) {
        uint32_t sign = (i & 1) << 31;
        uint32_t exp  = ((i >> 1) & 255) << 23;
        uint32_t man  = ((i >> 9) & 1023) << 13;
        uint32_t frac = ((i >> 19) & 7); // 0 and 1 are special, 6 other values are random bits
        uint64_t bits = sign | exp | man | ((frac == 0) ? 0 :  // all 0s in lost fraction
                                            (frac == 1) ? (1 << 13) - 1 // all 1s in lost fraction
                                                        : rand.nextBits(13)); // random lost bits
#endif

        float f = SkBits2Float(bits);
        if (SkIsNaN(f)) {
#ifndef SK_DEBUG
            // We want float->half and half->float to play well with infinities and max
            // representable values in the 16-bit precision, but NaNs should have been caught ahead
            // of time, so the conversion logic is allowed to convert them to infinities in release
            // builds. We skip calling `to_half` in debug since it asserts that NaN isn't passed in.
            uint16_t actual2 = to_half(skvx::float2{f})[0];
            uint16_t actual4 = to_half(skvx::float4{f})[0];
            REPORTER_ASSERT(r, (actual2 & kF16_Exp) == kF16_Exp);
            REPORTER_ASSERT(r, (actual4 & kF16_Exp) == kF16_Exp);
#endif
            continue;
        }

        uint32_t s32 = (uint32_t) bits & kF32_Sign;
        uint32_t e32 = (uint32_t) bits & kF32_Exp;
        uint32_t m32 = (uint32_t) bits & kF32_Mant;

        // Half floats can represent a real exponent from -14 to 15. Anything less than that would
        // need to be a denorm, which is flushed to zero, or overflows and becomes infinity.
        int      e   = (int) (e32 >> 23) - kF32_Bias; // the true signed exponent

        uint32_t s16 = s32 >> 16;
        uint32_t e16;
        uint32_t m16;
        if (e < -kF16_Bias-10 || (e == -kF16_Bias-10 && m32 <= 0)) {
            // Rounds to zero
            e16 = 0;
            m16 = 0;
        } else if ((e32 | m32) < 0x38fe'0000) {
            // A subnormal non-zero f16 value
            e16 = 0;
            m16 = 0xffff & sk_bit_cast<uint32_t>(0.5f + SkBits2Float(e32 | m32));
        } else if ((e32 | m32) < 0x3880'0000) {
            // Rounds up to smallest normal f16 (2^-14)
            e16 = 1;
            m16 = 0;
        } else if (e > kF16_Bias) {
            // Either f32 infinity or a value larger than what rounds down to the max normal half.
            e16 = kF16_Exp;
            m16 = 0;
        } else {
            // A normal half value, which is rounded towards nearest even.
            e16 = (uint32_t) (e + kF16_Bias) << 10;
            SkASSERT((e16 & ~kF16_Exp) == 0);

            // round to nearest even
            m32 += 0xfff + ((m32>>13)&1);

            if (m32 > kF32_Mant) {
                // overflow
                e16 += (1 << 10);
                m16 = 0;
            } else {
                m16 = m32 >> 13;
            }
        }

        // Expected conversion from f32 to f16
        uint16_t expected = s16 | e16 | m16;
        uint16_t actual2 = to_half(skvx::float2{f})[0];
        uint16_t actual4 = to_half(skvx::float4{f})[0];
        REPORTER_ASSERT(r, expected == actual2);
        REPORTER_ASSERT(r, expected == actual4);
    }
}

DEF_TEST(FloatToHalf_Constants, r) {
    auto to_half = [](float f) { return skvx::to_half(skvx::float4{f})[0]; };
    REPORTER_ASSERT(r, 0 == to_half(0.f));
    REPORTER_ASSERT(r, kF16_Sign == to_half(-0.f));
    REPORTER_ASSERT(r, SK_Half1 == to_half(1.f));
    REPORTER_ASSERT(r, (kF16_Sign | SK_Half1) == to_half(-1.f));
    REPORTER_ASSERT(r, SK_HalfMax == to_half(65504.f));
    REPORTER_ASSERT(r, SK_HalfMin == to_half(1.f / (1 << 14)));
}

DEF_TEST(HalfToFloat, r) {
     for (uint32_t bits = 0; bits <= 0xffff; bits++) {
        uint32_t s16 = bits & kF16_Sign;
        uint32_t e16 = bits & kF16_Exp;
        uint32_t m16 = bits & kF16_Mant;

        float actual2 = from_half(skvx::half2{(uint16_t) bits})[0];
        float actual4 = from_half(skvx::half4{(uint16_t) bits})[0];

        if (e16 == 0) {
            // De-normal f16 or a zero = 2^-14 * 0.[m16] = 2^-14 * 2^-10 * [m16].0
            float expected = (1.f / (1 << 14)) * (1.f / (1 << 10)) * m16;
            if (s16 != 0) {
                expected *= -1.f;
            }
            REPORTER_ASSERT(r, actual2 == expected);
            REPORTER_ASSERT(r, actual4 == expected);
        } else if (e16 == kF16_Exp) {
            if (m16 != 0) {
                // A NaN stays NaN
                REPORTER_ASSERT(r, SkIsNaN(actual2));
                REPORTER_ASSERT(r, SkIsNaN(actual4));
            } else {
                // +/- infinity stays infinite
                if (s16) {
                    REPORTER_ASSERT(r, actual2 == SK_ScalarNegativeInfinity);
                    REPORTER_ASSERT(r, actual4 == SK_ScalarNegativeInfinity);
                } else {
                    REPORTER_ASSERT(r, actual2 == SK_ScalarInfinity);
                    REPORTER_ASSERT(r, actual4 == SK_ScalarInfinity);
                }
            }
        } else {
            // A normal f16 is exactly representable in f32
            uint32_t s32 = s16 << 16;
            uint32_t e32 = ((e16 >> 10) + kF32_Bias - kF16_Bias) << 23;
            uint32_t m32 = m16 << 13;

            float expected = SkBits2Float(s32 | e32 | m32);
            REPORTER_ASSERT(r, actual2 == expected);
            REPORTER_ASSERT(r, actual4 == expected);
        }
    }
}
