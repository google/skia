/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkTaskGroup.h"
#include "SkXfermode.h"

#define ASSERT(x) REPORTER_ASSERT(r, x)

static uint8_t double_to_u8(double d) {
    SkASSERT(d >= 0);
    SkASSERT(d < 256);
    return uint8_t(d);
}

// All algorithms we're testing have this interface.
// We want a single channel blend, src over dst, assuming src is premultiplied by srcAlpha.
typedef uint8_t(*Blend)(uint8_t dst, uint8_t src, uint8_t srcAlpha);

// This is our golden algorithm.
static uint8_t blend_double_round(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    SkASSERT(src <= srcAlpha);
    return double_to_u8(0.5 + src + dst * (255.0 - srcAlpha) / 255.0);
}

static uint8_t abs_diff(uint8_t a, uint8_t b) {
    const int diff = a - b;
    return diff > 0 ? diff : -diff;
}

static void test(skiatest::Reporter* r, int maxDiff, Blend algorithm,
                 uint8_t dst, uint8_t src, uint8_t alpha) {
    const uint8_t golden = blend_double_round(dst, src, alpha);
    const uint8_t  blend =          algorithm(dst, src, alpha);
    if (abs_diff(blend, golden) > maxDiff) {
        SkDebugf("dst %02x, src %02x, alpha %02x, |%02x - %02x| > %d\n",
                 dst, src, alpha, blend, golden, maxDiff);
        ASSERT(abs_diff(blend, golden) <= maxDiff);
    }
}

// Exhaustively compare an algorithm against our golden, for a given alpha.
static void test_alpha(skiatest::Reporter* r, uint8_t alpha, int maxDiff, Blend algorithm) {
    SkASSERT(maxDiff >= 0);

    for (unsigned src = 0; src <= alpha; src++) {
        for (unsigned dst = 0; dst < 256; dst++) {
            test(r, maxDiff, algorithm, dst, src, alpha);
        }
    }
}

// Exhaustively compare an algorithm against our golden, for a given dst.
static void test_dst(skiatest::Reporter* r, uint8_t dst, int maxDiff, Blend algorithm) {
    SkASSERT(maxDiff >= 0);

    for (unsigned alpha = 0; alpha < 256; alpha++) {
        for (unsigned src = 0; src <= alpha; src++) {
            test(r, maxDiff, algorithm, dst, src, alpha);
        }
    }
}

static uint8_t blend_double_trunc(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    return double_to_u8(src + dst * (255.0 - srcAlpha) / 255.0);
}

static uint8_t blend_float_trunc(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    return double_to_u8(src + dst * (255.0f - srcAlpha) / 255.0f);
}

static uint8_t blend_float_round(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    return double_to_u8(0.5f + src + dst * (255.0f - srcAlpha) / 255.0f);
}

static uint8_t blend_255_trunc(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 255 - srcAlpha;
    const uint16_t product = dst * invAlpha;
    return src + (product >> 8);
}

static uint8_t blend_255_round(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 255 - srcAlpha;
    const uint16_t product = dst * invAlpha + 128;
    return src + (product >> 8);
}

static uint8_t blend_256_trunc(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 256 - (srcAlpha + (srcAlpha >> 7));
    const uint16_t product = dst * invAlpha;
    return src + (product >> 8);
}

static uint8_t blend_256_round(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 256 - (srcAlpha + (srcAlpha >> 7));
    const uint16_t product = dst * invAlpha + 128;
    return src + (product >> 8);
}

static uint8_t blend_256_round_alt(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint8_t invAlpha8 = 255 - srcAlpha;
    const uint16_t invAlpha = invAlpha8 + (invAlpha8 >> 7);
    const uint16_t product = dst * invAlpha + 128;
    return src + (product >> 8);
}

static uint8_t blend_256_plus1_trunc(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 256 - (srcAlpha + 1);
    const uint16_t product = dst * invAlpha;
    return src + (product >> 8);
}

static uint8_t blend_256_plus1_round(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint16_t invAlpha = 256 - (srcAlpha + 1);
    const uint16_t product = dst * invAlpha + 128;
    return src + (product >> 8);
}

static uint8_t blend_perfect(uint8_t dst, uint8_t src, uint8_t srcAlpha) {
    const uint8_t invAlpha = 255 - srcAlpha;
    const uint16_t product = dst * invAlpha + 128;
    return src + ((product + (product >> 8)) >> 8);
}


// We want 0 diff whenever src is fully transparent.
DEF_TEST(Blend_alpha_0x00, r) {
    const uint8_t alpha = 0x00;

    // GOOD
    test_alpha(r, alpha, 0, blend_256_round);
    test_alpha(r, alpha, 0, blend_256_round_alt);
    test_alpha(r, alpha, 0, blend_256_trunc);
    test_alpha(r, alpha, 0, blend_double_trunc);
    test_alpha(r, alpha, 0, blend_float_round);
    test_alpha(r, alpha, 0, blend_float_trunc);
    test_alpha(r, alpha, 0, blend_perfect);

    // BAD
    test_alpha(r, alpha, 1, blend_255_round);
    test_alpha(r, alpha, 1, blend_255_trunc);
    test_alpha(r, alpha, 1, blend_256_plus1_round);
    test_alpha(r, alpha, 1, blend_256_plus1_trunc);
}

// We want 0 diff whenever dst is 0.
DEF_TEST(Blend_dst_0x00, r) {
    const uint8_t dst = 0x00;

    // GOOD
    test_dst(r, dst, 0, blend_255_round);
    test_dst(r, dst, 0, blend_255_trunc);
    test_dst(r, dst, 0, blend_256_plus1_round);
    test_dst(r, dst, 0, blend_256_plus1_trunc);
    test_dst(r, dst, 0, blend_256_round);
    test_dst(r, dst, 0, blend_256_round_alt);
    test_dst(r, dst, 0, blend_256_trunc);
    test_dst(r, dst, 0, blend_double_trunc);
    test_dst(r, dst, 0, blend_float_round);
    test_dst(r, dst, 0, blend_float_trunc);
    test_dst(r, dst, 0, blend_perfect);

    // BAD
}

// We want 0 diff whenever src is fully opaque.
DEF_TEST(Blend_alpha_0xFF, r) {
    const uint8_t alpha = 0xFF;

    // GOOD
    test_alpha(r, alpha, 0, blend_255_round);
    test_alpha(r, alpha, 0, blend_255_trunc);
    test_alpha(r, alpha, 0, blend_256_plus1_round);
    test_alpha(r, alpha, 0, blend_256_plus1_trunc);
    test_alpha(r, alpha, 0, blend_256_round);
    test_alpha(r, alpha, 0, blend_256_round_alt);
    test_alpha(r, alpha, 0, blend_256_trunc);
    test_alpha(r, alpha, 0, blend_double_trunc);
    test_alpha(r, alpha, 0, blend_float_round);
    test_alpha(r, alpha, 0, blend_float_trunc);
    test_alpha(r, alpha, 0, blend_perfect);

    // BAD
}

// We want 0 diff whenever dst is 0xFF.
DEF_TEST(Blend_dst_0xFF, r) {
    const uint8_t dst = 0xFF;

    // GOOD
    test_dst(r, dst, 0, blend_256_round);
    test_dst(r, dst, 0, blend_256_round_alt);
    test_dst(r, dst, 0, blend_double_trunc);
    test_dst(r, dst, 0, blend_float_round);
    test_dst(r, dst, 0, blend_float_trunc);
    test_dst(r, dst, 0, blend_perfect);

    // BAD
    test_dst(r, dst, 1, blend_255_round);
    test_dst(r, dst, 1, blend_255_trunc);
    test_dst(r, dst, 1, blend_256_plus1_round);
    test_dst(r, dst, 1, blend_256_plus1_trunc);
    test_dst(r, dst, 1, blend_256_trunc);
}

// We'd like diff <= 1 everywhere.
DEF_TEST(Blend_alpha_Exhaustive, r) {
    for (unsigned alpha = 0; alpha < 256; alpha++) {
        // PERFECT
        test_alpha(r, alpha, 0, blend_float_round);
        test_alpha(r, alpha, 0, blend_perfect);

        // GOOD
        test_alpha(r, alpha, 1, blend_255_round);
        test_alpha(r, alpha, 1, blend_256_plus1_round);
        test_alpha(r, alpha, 1, blend_256_round);
        test_alpha(r, alpha, 1, blend_256_round_alt);
        test_alpha(r, alpha, 1, blend_256_trunc);
        test_alpha(r, alpha, 1, blend_double_trunc);
        test_alpha(r, alpha, 1, blend_float_trunc);

        // BAD
        test_alpha(r, alpha, 2, blend_255_trunc);
        test_alpha(r, alpha, 2, blend_256_plus1_trunc);
    }
}

// We'd like diff <= 1 everywhere.
DEF_TEST(Blend_dst_Exhaustive, r) {
    for (unsigned dst = 0; dst < 256; dst++) {
        // PERFECT
        test_dst(r, dst, 0, blend_float_round);
        test_dst(r, dst, 0, blend_perfect);

        // GOOD
        test_dst(r, dst, 1, blend_255_round);
        test_dst(r, dst, 1, blend_256_plus1_round);
        test_dst(r, dst, 1, blend_256_round);
        test_dst(r, dst, 1, blend_256_round_alt);
        test_dst(r, dst, 1, blend_256_trunc);
        test_dst(r, dst, 1, blend_double_trunc);
        test_dst(r, dst, 1, blend_float_trunc);

        // BAD
        test_dst(r, dst, 2, blend_255_trunc);
        test_dst(r, dst, 2, blend_256_plus1_trunc);
    }
}
// Overall summary:
// PERFECT
//  blend_double_round
//  blend_float_round
//  blend_perfect
// GOOD ENOUGH
//  blend_double_trunc
//  blend_float_trunc
//  blend_256_round
//  blend_256_round_alt
// NOT GOOD ENOUGH
//  all others
//
//  Algorithms that make sense to use in Skia: blend_256_round, blend_256_round_alt, blend_perfect

DEF_TEST(Blend_premul_begets_premul, r) {
    // This test is quite slow, even if you have enough cores to run each mode in parallel.
    if (!r->allowExtendedTest()) {
        return;
    }

    // No matter what xfermode we use, premul inputs should create premul outputs.
    auto test_mode = [&](int m) {
        SkXfermode::Mode mode = (SkXfermode::Mode)m;
        if (mode == SkXfermode::kSrcOver_Mode) {
            return;  // TODO: can't create a SrcOver xfermode.
        }
        SkAutoTUnref<SkXfermode> xfermode(SkXfermode::Create(mode));
        SkASSERT(xfermode);
        // We'll test all alphas and legal color values, assuming all colors work the same.
        // This is not true for non-separable blend modes, but this test still can't hurt.
        for (int sa = 0; sa <= 255; sa++) {
        for (int da = 0; da <= 255; da++) {
        for (int  s = 0;  s <= sa;   s++) {
        for (int  d = 0;  d <= da;   d++) {
            SkPMColor src = SkPackARGB32(sa, s, s, s),
                      dst = SkPackARGB32(da, d, d, d);
            xfermode->xfer32(&dst, &src, 1, nullptr);  // To keep it simple, no AA.
            if (!SkPMColorValid(dst)) {
                ERRORF(r, "%08x is not premul using %s", dst, SkXfermode::ModeName(mode));
            }
        }}}}
    };

    // Parallelism helps speed things up on my desktop from ~725s to ~50s.
    sk_parallel_for(SkXfermode::kLastMode, test_mode);
}
