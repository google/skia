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

constexpr auto F_ = SK_OPTS_NS::F_;
using F = SK_OPTS_NS::F;
using I32 = SK_OPTS_NS::I32;

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
    constexpr float Pi = SK_ScalarPI;
    constexpr float kTolerance = 0.000875f;
    for (float rad = -5*Pi; rad <= 5*Pi; rad += 0.1f) {
        F result = SK_OPTS_NS::sin_(F_(rad));
        F expected = F_(std::sin(rad));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Cos, r) {
    constexpr float Pi = SK_ScalarPI;
    constexpr float kTolerance = 0.000875f;
    for (float rad = -5*Pi; rad <= 5*Pi; rad += 0.1f) {
        F result = SK_OPTS_NS::cos_(F_(rad));
        F expected = F_(std::cos(rad));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Tan, r) {
    // Our tangent diverges more as we get near infinities (x near +- Pi/2),
    // so we bring in the domain a little.
    constexpr float Pi = SK_ScalarPI;
    constexpr float kEpsilon = 0.16f;
    constexpr float kTolerance = 0.00175f;

    // Test against various multiples of Pi, to check our periodicity
    for (float period : {0.0f, -3*Pi, 3*Pi}) {
        for (float rad = -Pi/2 + kEpsilon; rad <= Pi/2 - kEpsilon; rad += 0.01f) {
            F result = SK_OPTS_NS::tan_(F_(rad + period));
            F expected = F_(std::tan(rad));
            F delta = SK_OPTS_NS::abs_(expected - result);

            REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
        }
    }
}

DEF_TEST(SkRasterPipelineOpts_Asin, r) {
    constexpr float kTolerance = 0.00175f;
    for (float x = -1; x <= 1; x += 1.0f/64) {
        F result = SK_OPTS_NS::asin_(F_(x));
        F expected = F_(asinf(x));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Acos, r) {
    constexpr float kTolerance = 0.00175f;
    for (float x = -1; x <= 1; x += 1.0f/64) {
        F result = SK_OPTS_NS::acos_(F_(x));
        F expected = F_(acosf(x));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Atan, r) {
    constexpr float kTolerance = 0.00175f;
    for (float x = -10.0f; x <= 10.0f; x += 0.1f) {
        F result = SK_OPTS_NS::atan_(F_(x));
        F expected = F_(atanf(x));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Atan2, r) {
    constexpr float kTolerance = 0.00175f;
    for (float y = -3.0f; y <= 3.0f; y += 0.1f) {
        for (float x = -3.0f; x <= 3.0f; x += 0.1f) {
            F result = SK_OPTS_NS::atan2_(F_(y), F_(x));
            F expected = F_(std::atan2(y, x));
            F delta = SK_OPTS_NS::abs_(expected - result);

            REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
        }
    }
}

DEF_TEST(SkRasterPipelineOpts_Log2, r) {
    constexpr float kTolerance = 0.001f;
    for (float value : {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f}) {
        F result = SK_OPTS_NS::approx_log2(F_(value));
        F expected = F_(std::log2(value));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}

DEF_TEST(SkRasterPipelineOpts_Pow2, r) {
    constexpr float kTolerance = 0.001f;
    for (float value : {-80, -5, -2, -1, 0, 1, 2, 3, 5}) {
        F result = SK_OPTS_NS::approx_pow2(F_(value));
        F expected = F_(std::pow(2.0, value));
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }

    F result = SK_OPTS_NS::approx_pow2(F_(160));
    REPORTER_ASSERT(r, SK_OPTS_NS::all(result == INFINITY));
}
