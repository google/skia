/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpacePriv.h"
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

    // A couple (redundant) sanity checks that cover impossible states.
    // At most one of the early/late options should happen, possibly neither.
    REPORTER_ASSERT(r, !(steps.early_unpremul && steps.late_unpremul));
    REPORTER_ASSERT(r, !(steps.early_encode   && steps.late_encode));
}

DEF_TEST(SkColorSpaceXformSteps, r) {
    auto srgb_L = SkColorSpace::MakeSRGB(),
        adobe_L = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut),
       srgb22_L = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::    kSRGB_Gamut),
         srgb_N =   srgb_L->makeNonlinearBlending(),
        adobe_N =  adobe_L->makeNonlinearBlending(),
       srgb22_N = srgb22_L->makeNonlinearBlending();

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
        // The first eight cases we test are back and forth between two color spaces with
        // different gamuts and transfer functions.  There's not much optimization possible here.

        { adobe_N, srgb_N,
            true,   // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,   // Linearize to s,a.
            false,

            true,  // Gamut transform.
            true,  // Non-linear blending, so we encode to sRGB g(s),a early.
            true,  // Premul to g(s)*a,a

            false,  // Non-linear blending, so no need to linearize dst.
            false,  // Non-linear blending, so the output of our blend function is what we want.
        },
        { srgb_N, adobe_N,  true,true,false, true,true,true, false,false },

        { adobe_L, srgb_L,
            false,  // src is encoded as f(s*a),a, so we linearize before unpremul.
            true,   // Linearize,
            true,   // then unpremul.

            true,   // Gamut transform.
            false,  // We're doing linear blending, so we don't encode to sRGB yet.
            true,   // Premul so we can blend.

            true,   // We're doing linear blending, so we need to linearize dst.
            true,   // Once blending is done, finally encode to sRGB.
        },
        { srgb_L, adobe_L, false,true,true, true,false,true, true,true },

        { adobe_L, srgb_N,
            false,  // src is encoded as f(s*a),a, so we linearize before unpremul.
            true,   // Linearize,
            true,   // then unpremul.

            true,   // Gamut transform
            true,   // We're doing non-linear blending, so encode to sRGB now.
            true,   // (non-linear) premul

            false,  // We're doing non-linear blending, so dst is already ready to blend.
            false,  // The output of the blend is just what we want.
        },
        { srgb_L, adobe_N,  false,true,true, true,true,true, false,false },

        { adobe_N, srgb_L,
            true,   // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,   // Linearize to s,a.
            false,

            true,   // Gamut transform
            false,  // We're doing linear blending, so we don't encode to sRGB yet.
            true,   // (linear) premul

            true,   // We're doing linear blending, so we need to linearize dst.
            true,   // Once blending is done, finally encode to sRGB.
        },
        { srgb_N, adobe_L,  true,true,false, true,false,true, true,true },

        // These eight cases transform between color spaces with different
        // transfer functions and the same gamut.  Optimization here is limited
        // to skipping the gamut_transform step: |
        //                                       v This column of true can all become false.
        { srgb_N, srgb22_N,   true,true,false, true,true,true,  false,false },
        { srgb22_N, srgb_N,   true,true,false, true,true,true,  false,false },

        { srgb_L, srgb22_L,   false,true,true, true,false,true, true,true   },
        { srgb22_L, srgb_L,   false,true,true, true,false,true, true,true   },

        { srgb_N, srgb22_L,   true,true,false, true,false,true, true,true   },
        { srgb22_N, srgb_L,   true,true,false, true,false,true, true,true   },

        { srgb_L, srgb22_N,   false,true,true, true,true,true,  false,false },
        { srgb22_L, srgb_N,   false,true,true, true,true,true,  false,false },

        // These four test cases test drawing in the same color space.
        // There is lots of room for optimization here, but none implemented yet.
        { srgb_N, srgb_N,   true,true,false, true,true,true,  false,false },  // a.k.a  legacy 8888
        { srgb_L, srgb_L,   false,true,true, true,false,true, true,true   },  // <canvas> use case
        { srgb_N, srgb_L,   true,true,false, true,false,true, true,true   },
        { srgb_L, srgb_N,   false,true,true, true,true,true,  false,false },

        // TODO: versions of above crossing in linear transfer functions
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
