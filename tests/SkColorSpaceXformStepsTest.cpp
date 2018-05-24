/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformSteps.h"
#include "Test.h"

template <typename T>
static void check_eq(skiatest::Reporter* r, SkColorSpaceXformSteps steps, T baseline) {
    REPORTER_ASSERT(r, steps.early_unpremul  == baseline.early_unpremul);
    REPORTER_ASSERT(r, steps.linearize_src   == baseline.linearize_src);
    REPORTER_ASSERT(r, steps.late_unpremul   == baseline.late_unpremul);
    REPORTER_ASSERT(r, steps.gamut_transform == baseline.gamut_transform);
    REPORTER_ASSERT(r, steps.early_encode    == baseline.early_encode);
    REPORTER_ASSERT(r, steps.premul          == baseline.premul);
    REPORTER_ASSERT(r, steps.linearize_dst   == baseline.linearize_dst);
    REPORTER_ASSERT(r, steps.late_encode     == baseline.late_encode);
}

DEF_TEST(SkColorSpaceXformSteps, r) {
    auto srgb_L = SkColorSpace::MakeSRGB(),
         srgb_N = SkColorSpace::MakeSRGB()->makeNonlinearBlending();

    struct {
        sk_sp<SkColorSpace> src, dst;

        bool early_unpremul;
        bool linearize_src;
        bool late_unpremul;

        bool gamut_transform;
        bool early_encode;
        bool premul;

        bool linearize_dst;
        bool late_encode;
    } tests[] = {
        // This is essentially legacy 8888, non-linearly blended srcs and dsts.
        // With optimization one day these will all be false.
        { srgb_N, srgb_N,
            true,   // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,   // Linearize to s,a.
            false,

            true,  // Gamut transform.
            true,  // Non-linear blending, so we encode to sRGB g(s),a early.
            true,  // Premul to g(s)*a,a

            false,  // Non-linear blending, so no need to linearize dst.
            false,  // Non-linear blending, so the output of our blend function is what we want.
        },

        // This is <canvas>'s linearly blended sRGB use case.
        { srgb_L, srgb_L,
            false,  // src is encoded as f(s*a),a, so we linearize before unpremul.
            true,   // Linearize,
            true,   // then unpremul (we haven't optimized away unpremul-premul pairs yet).

            true,   // (We haven't optimized away identity gamut transforms yet.)
            false,  // We're doing linear blending, so we don't encode to sRGB yet.
            true,   // (We haven't optimized away unpremul-premul pairs yet.)

            true,   // We're doing linear blending, so we need to linearize dst.
            true,   // Once blending is done, finally encode to sRGB.
        },

        { srgb_L, srgb_N,
            false,  // src is encoded as f(s*a),a, so we linearize before unpremul.
            true,   // Linearize,
            true,   // then unpremul.

            true,   // (We haven't optimized away identity gamut transforms yet.)
            true,   // We're doing non-linear blending, so encode back to sRGB now.
            true,   // (non-linear) premul

            false,  // We're doing non-linear blending, so dst is already ready to blend.
            false,  // The output of the blend is just what we want.
        },

        { srgb_N, srgb_L,
            true,   // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,   // Linearize to s,a.
            false,

            true,   // (We haven't optimized away identity gamut transforms yet.)
            false,  // We're doing linear blending, so we don't encode to sRGB yet.
            true,   // (linear) premul

            true,   // We're doing linear blending, so we need to linearize dst.
            true,   // Once blending is done, finally encode to sRGB.
        },

        // TODO:
        //   This function has essentially 256 different outputs.
        //   It's an interesting question how to test to make sure it's correct.
        //
        // Here are some ideas for further test cases:
        //  - all combinations with the same gamut and transfer function,
        //    varying non-linear blending bit
        //
        //  - all combinations in both directions with the same gamut and different
        //    (non-identity) transfer function  (16 cases)
        //
        //  - all combinations in both directions with different gamuts and the same
        //    (non-identity) transfer function  (16 cases)
        //
        //  - all combinations in both directions with different gamuts and different
        //    (non-identity) transfer function  (16 cases)
        //
        //  - mix some identity transfer functions in there to taste.
        //  - test unpremul sources?  are they any different than opaque?
    };

    for (auto t : tests) {
        // Our expectations are written for premul source alpha types.
        check_eq(r, SkColorSpaceXformSteps(t.src.get(), kPremul_SkAlphaType, t.dst.get()), t);

        // Opaque and unpremul sources should always go through the same steps,
        // and they should be the same as premul's steps, with these fixed premul/unpremul steps.
        auto upm = t;
        upm.early_unpremul = false;
        upm.late_unpremul  = false;
        upm.premul         = true;

        check_eq(r, SkColorSpaceXformSteps(t.src.get(), kUnpremul_SkAlphaType, t.dst.get()), upm);
        check_eq(r, SkColorSpaceXformSteps(t.src.get(),   kOpaque_SkAlphaType, t.dst.get()), upm);
    }
}
