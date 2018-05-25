/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
#include "Test.h"

DEF_TEST(SkColorSpaceXformSteps, r) {
    auto srgb   = SkColorSpace::MakeSRGB(),
         adobe  = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut),
         srgb22 = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::    kSRGB_Gamut),
         srgb1  = srgb ->makeLinearGamma(),
         adobe1 = adobe->makeLinearGamma();

    struct {
        sk_sp<SkColorSpace> src, dst;
        SkAlphaType         srcAT;

        bool unpremul;
        bool linearize;
        bool gamut_transform;
        bool encode;
        bool premul;

    } tests[] = {
        // The general case is converting between two color spaces with different gamuts
        // and different transfer functions.  There's no optimization possible here.
        { adobe, srgb, kPremul_SkAlphaType,
            true,  // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,  // linearize to s,a
            true,  // transform s to dst gamut, s'
            true,  // encode with dst transfer function, g(s'), a
            true,  // premul to g(s')*a, a
        },
        // All the same going the other direction.
        { srgb, adobe, kPremul_SkAlphaType,  true,true,true,true,true },

        // If the src alpha type is unpremul, we'll not need that initial unpremul step.
        { adobe, srgb, kUnpremul_SkAlphaType, false,true,true,true,true },
        { srgb, adobe, kUnpremul_SkAlphaType, false,true,true,true,true },

        // If opaque, we need neither the initial unpremul, nor the premul later.
        { adobe, srgb, kOpaque_SkAlphaType, false,true,true,true,false },
        { srgb, adobe, kOpaque_SkAlphaType, false,true,true,true,false },


        // Now let's go between sRGB and sRGB with a 2.2 gamma, the gamut staying the same.
        { srgb, srgb22, kPremul_SkAlphaType,
            true,  // we need to linearize, so we need to unpremul
            true,  // we need to encode to 2.2 gamma, so we need to get linear
            false, // no need to change gamut
            true,  // linear -> gamma 2.2
            true,  // premul going into the blend
        },
        // Same sort of logic in the other direction.
        { srgb22, srgb, kPremul_SkAlphaType,  true,true,false,true,true },

        // As in the general case, when we change the alpha type unpremul and premul steps drop out.
        { srgb, srgb22, kUnpremul_SkAlphaType, false,true,false,true,true },
        { srgb22, srgb, kUnpremul_SkAlphaType, false,true,false,true,true },
        { srgb, srgb22,   kOpaque_SkAlphaType, false,true,false,true,false },
        { srgb22, srgb,   kOpaque_SkAlphaType, false,true,false,true,false },

        // Let's look at the special case of completely matching color spaces.
        // We should be ready to go into the blend without any fuss.
        { srgb, srgb,   kPremul_SkAlphaType, false,false,false,false,false },
        { srgb, srgb, kUnpremul_SkAlphaType, false,false,false,false,true },
        { srgb, srgb,   kOpaque_SkAlphaType, false,false,false,false,false },

        // We can drop out the linearize step when the source is already linear.
        { srgb1, adobe,   kPremul_SkAlphaType, true,false,true,true,true },
        { srgb1,  srgb,   kPremul_SkAlphaType, true,false,false,true,true },
        // And we can drop the encode step when the destination is linear.
        { adobe, srgb1,   kPremul_SkAlphaType, true,true,true,false,true },
        {  srgb, srgb1,   kPremul_SkAlphaType, true,true,false,false,true },

        // Here's an interesting case where only gamut transform is needed.
        { adobe1, srgb1,  kPremul_SkAlphaType, false,false,true,false,false },
    };

    // It'd be nice if we can cover all 2^5 == 32 possible outputs.
    uint32_t tested = 0x00000000;
    for (auto t : tests) {
        SkColorSpaceXformSteps steps{t.src.get(), t.srcAT, t.dst.get()};
        REPORTER_ASSERT(r, steps.unpremul        == t.unpremul);
        REPORTER_ASSERT(r, steps.linearize       == t.linearize);
        REPORTER_ASSERT(r, steps.gamut_transform == t.gamut_transform);
        REPORTER_ASSERT(r, steps.encode          == t.encode);
        REPORTER_ASSERT(r, steps.premul          == t.premul);

        uint32_t bits = (uint32_t)t.unpremul        << 0
                      | (uint32_t)t.linearize       << 1
                      | (uint32_t)t.gamut_transform << 2
                      | (uint32_t)t.encode          << 3
                      | (uint32_t)t.premul          << 4;
        tested |= (1<<bits);
    }
    //REPORTER_ASSERT(r, tested == 0xffffffff);
}
