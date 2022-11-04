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
    constexpr float kTolerance = 0.00175f;
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
    constexpr float kTolerance = 0.00175f;
    for (float rad = -5*Pi; rad <= 5*Pi; rad += 0.1f) {
        F result = SK_OPTS_NS::cos_(rad);
        F expected = sk_float_cos(rad);
        F delta = SK_OPTS_NS::abs_(expected - result);

        REPORTER_ASSERT(r, SK_OPTS_NS::all(delta < kTolerance));
    }
}
