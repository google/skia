/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "tests/Test.h"

#include <algorithm>
#include <array>
#include <cstddef>

#define SK_OPTS_NS RPOptsTest

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "src/opts/SkRasterPipeline_opts.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

template <size_t N>
static std::array<int32_t, N> make_masks(int bits) {
    // Make an array of masks that correspond to the bit pattern of `bits`.
    std::array<int32_t, N> masks;
    for (size_t idx = 0; idx < N; ++idx) {
        masks[idx] = (bits & 1) ? ~0 : 0;
        bits >>= 1;
    }
    SkASSERT(!bits);
    return masks;
}

DEF_TEST(SkRasterPipelineOpts_Any, r) {
    using I32 = SK_OPTS_NS::I32;
    static constexpr size_t N = sizeof(I32) / sizeof(int32_t);

    for (int value = 0; value < (1 << N); ++value) {
        // Load masks corresponding to the bit-pattern of `value` into lanes of `i`.
        std::array<int32_t, N> masks = make_masks<N>(value);
        I32 i = sk_unaligned_load<I32>(masks.data());

        // Verify that the raster pipeline any() matches expectations.
        REPORTER_ASSERT(r, SK_OPTS_NS::any(i) == std::any_of(masks.begin(), masks.end(),
                                                             [](int32_t m) { return m != 0; }));
    }
}

DEF_TEST(SkRasterPipelineOpts_All, r) {
    using I32 = SK_OPTS_NS::I32;
    static constexpr size_t N = sizeof(I32) / sizeof(int32_t);

    for (int value = 0; value < (1 << N); ++value) {
        // Load masks corresponding to the bit-pattern of `value` into lanes of `i`.
        std::array<int32_t, N> masks = make_masks<N>(value);
        I32 i = sk_unaligned_load<I32>(masks.data());

        // Verify that the raster pipeline all() matches expectations.
        REPORTER_ASSERT(r, SK_OPTS_NS::all(i) == std::all_of(masks.begin(), masks.end(),
                                                             [](int32_t m) { return m != 0; }));
    }
}

DEF_TEST(SkRasterPipelineOpts_Sin, r) {
    using F = SK_OPTS_NS::F;

    constexpr float Pi = SK_ScalarPI;
    constexpr float kTolerance = 0.000875f;
    for (float rad = -5*Pi; rad <= 5*Pi; rad += 0.1f) {
        F result = SK_OPTS_NS::sin_(rad);
        F expected = sk_float_sin(rad);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Cos, r) {
    using F = SK_OPTS_NS::F;

    constexpr float Pi = SK_ScalarPI;
    constexpr float kTolerance = 0.000875f;
    for (float rad = -5*Pi; rad <= 5*Pi; rad += 0.1f) {
        F result = SK_OPTS_NS::cos_(rad);
        F expected = sk_float_cos(rad);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Tan, r) {
    using F = SK_OPTS_NS::F;

    // Our tangent diverges more as we get near infinities (x near +- Pi/2),
    // so we bring in the domain a little.
    constexpr float Pi = SK_ScalarPI;
    constexpr float kEpsilon = 0.16f;
    constexpr float kTolerance = 0.00175f;

    // Test against various multiples of Pi, to check our periodicity
    for (float period : {0.0f, -3*Pi, 3*Pi}) {
        for (float rad = -Pi/2 + kEpsilon; rad <= Pi/2 - kEpsilon; rad += 0.01f) {
            F result = SK_OPTS_NS::tan_(rad + period);
            F expected = sk_float_tan(rad);
            F delta = SK_OPTS_NS::abs_(expected - result);

            REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
        }
    }
}

DEF_TEST(SkRasterPipelineOpts_Asin, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.00175f;
    for (float x = -1; x <= 1; x += 1.0f/64) {
        F result = SK_OPTS_NS::asin_(x);
        F expected = asinf(x);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Acos, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.00175f;
    for (float x = -1; x <= 1; x += 1.0f/64) {
        F result = SK_OPTS_NS::acos_(x);
        F expected = acosf(x);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Atan, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.00175f;
    for (float x = -10.0f; x <= 10.0f; x += 0.1f) {
        F result = SK_OPTS_NS::atan_(x);
        F expected = atanf(x);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Atan2, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.00175f;
    for (float y = -3.0f; y <= 3.0f; y += 0.1f) {
        for (float x = -3.0f; x <= 3.0f; x += 0.1f) {
            F result = SK_OPTS_NS::atan2_(y, x);
            F expected = sk_float_atan2(y, x);
            F delta = SK_OPTS_NS::abs_(expected - result);

            REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
        }
    }
}

DEF_TEST(SkRasterPipelineOpts_Log2, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.001f;
    for (float value : {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f}) {
        F result = SK_OPTS_NS::approx_log2(value);
        F expected = std::log2(value);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Pow2, r) {
    using F = SK_OPTS_NS::F;

    constexpr float kTolerance = 0.001f;
    for (float value : {-80, -5, -2, -1, 0, 1, 2, 3, 5}) {
        F result = SK_OPTS_NS::approx_pow2(value);
        F expected = std::pow(2.0, value);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }

    F result = SK_OPTS_NS::approx_pow2(160);
    REPORTER_ASSERT(r, SK_OPTS_NS::all(result == INFINITY));
}
