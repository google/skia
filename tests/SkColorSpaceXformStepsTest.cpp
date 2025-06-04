/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "tests/Test.h"

#include <cstdint>

static skcms_TransferFunction trfn_pq_100() {
    skcms_TransferFunction trfn;
    skcms_TransferFunction_makePQ(&trfn, 100.f);
    return trfn;
}

static skcms_TransferFunction trfn_pq_203() {
    skcms_TransferFunction trfn;
    skcms_TransferFunction_makePQ(&trfn, 203.f);
    return trfn;
}

static skcms_TransferFunction trfn_hlg_12x() {
    skcms_TransferFunction trfn;
    skcms_TransferFunction_makeHLG(&trfn, 1.f, 12.f, 1.f);
    return trfn;
}

static skcms_TransferFunction trfn_hlg_10a() {
    skcms_TransferFunction trfn;
    skcms_TransferFunction_makeHLG(&trfn, 100.f, 1000.f, 1.2f);
    return trfn;
}

static skcms_TransferFunction trfn_hlg_10b() {
    skcms_TransferFunction trfn;
    skcms_TransferFunction_makeHLG(&trfn, 10.f, 100.f, 1.2f);
    return trfn;
}

static skcms_TransferFunction trfn_hlg_203() {
  skcms_TransferFunction trfn;
  skcms_TransferFunction_makeHLG(&trfn, 203.f, 1000.f, 1.2f);
  return trfn;
}

static bool rgb_close(const float* rgb, float r, float g, float b) {
    return std::abs(rgb[0] - r) / r < 0.01 &&
           std::abs(rgb[1] - g) / g < 0.01 &&
           std::abs(rgb[2] - b) / b < 0.01;
}

DEF_TEST(SkColorSpaceXformSteps, r) {
    auto srgb   = SkColorSpace::MakeSRGB(),
         adobe  = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB),
         srgb22 = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kSRGB),
         srgb1  = srgb ->makeLinearGamma(),
         adobe1 = adobe->makeLinearGamma(),
         rec2020_pq_203 = SkColorSpace::MakeRGB(trfn_pq_203(), SkNamedGamut::kRec2020),
         rec2020_pq_100 = SkColorSpace::MakeRGB(trfn_pq_100(), SkNamedGamut::kRec2020),
         p3_pq_203 = SkColorSpace::MakeRGB(trfn_pq_203(), SkNamedGamut::kDisplayP3),
         rec2020_hlg_12x = SkColorSpace::MakeRGB(trfn_hlg_12x(), SkNamedGamut::kRec2020),
         rec2020_hlg_10a = SkColorSpace::MakeRGB(trfn_hlg_10a(), SkNamedGamut::kRec2020),
         rec2020_hlg_10b = SkColorSpace::MakeRGB(trfn_hlg_10b(), SkNamedGamut::kRec2020),
         rec2020_hlg_203 = SkColorSpace::MakeRGB(trfn_hlg_203(), SkNamedGamut::kRec2020);

    auto premul =   kPremul_SkAlphaType,
         opaque =   kOpaque_SkAlphaType,
       unpremul = kUnpremul_SkAlphaType;

    struct Test {
        sk_sp<SkColorSpace> src, dst;
        SkAlphaType         srcAT, dstAT;

        bool unpremul = false;
        bool linearize = false;
        bool gamut_transform = false;
        bool encode = false;
        bool premul = false;
        bool src_ootf = false;
        bool dst_ootf = false;
    };
    Test tests[] = {
        // The general case is converting between two color spaces with different gamuts
        // and different transfer functions.  There's no optimization possible here.
        { adobe, srgb, premul, premul,
            true,  // src is encoded as f(s)*a,a, so we unpremul to f(s),a before linearizing.
            true,  // linearize to s,a
            true,  // transform s to dst gamut, s'
            true,  // encode with dst transfer function, g(s'), a
            true,  // premul to g(s')*a, a
        },
        // All the same going the other direction.
        { srgb, adobe, premul, premul,  true,true,true,true,true },

        // If the src alpha type is unpremul, we'll not need that initial unpremul step.
        { adobe, srgb, unpremul, premul, false,true,true,true,true },
        { srgb, adobe, unpremul, premul, false,true,true,true,true },

        // If opaque, we need neither the initial unpremul, nor the premul later.
        { adobe, srgb, opaque, premul, false,true,true,true,false },
        { srgb, adobe, opaque, premul, false,true,true,true,false },


        // Now let's go between sRGB and sRGB with a 2.2 gamma, the gamut staying the same.
        { srgb, srgb22, premul, premul,
            true,  // we need to linearize, so we need to unpremul
            true,  // we need to encode to 2.2 gamma, so we need to get linear
            false, // no need to change gamut
            true,  // linear -> gamma 2.2
            true,  // premul going into the blend
        },
        // Same sort of logic in the other direction.
        { srgb22, srgb, premul, premul,  true,true,false,true,true },

        // As in the general case, when we change the alpha type unpremul and premul steps drop out.
        { srgb, srgb22, unpremul, premul, false,true,false,true,true },
        { srgb22, srgb, unpremul, premul, false,true,false,true,true },
        { srgb, srgb22,   opaque, premul, false,true,false,true,false },
        { srgb22, srgb,   opaque, premul, false,true,false,true,false },

        // Let's look at the special case of completely matching color spaces.
        // We should be ready to go into the blend without any fuss.
        { srgb, srgb,   premul, premul, false,false,false,false,false },
        { srgb, srgb, unpremul, premul, false,false,false,false,true },
        { srgb, srgb,   opaque, premul, false,false,false,false,false },

        // We can drop out the linearize step when the source is already linear.
        { srgb1, adobe,   premul, premul, true,false,true,true,true },
        { srgb1,  srgb,   premul, premul, true,false,false,true,true },
        // And we can drop the encode step when the destination is linear.
        { adobe, srgb1,   premul, premul, true,true,true,false,true },
        {  srgb, srgb1,   premul, premul, true,true,false,false,true },

        // Here's an interesting case where only gamut transform is needed.
        { adobe1, srgb1,   premul, premul, false,false,true,false,false },
        { adobe1, srgb1,   opaque, premul, false,false,true,false,false },
        { adobe1, srgb1, unpremul, premul, false,false,true,false, true },

        // Just finishing up with something to produce each other possible output.
        // Nothing terribly interesting in these eight.
        { srgb,  srgb1,   opaque, premul, false, true,false,false,false },
        { srgb,  srgb1, unpremul, premul, false, true,false,false, true },
        { srgb, adobe1,   opaque, premul, false, true, true,false,false },
        { srgb, adobe1, unpremul, premul, false, true, true,false, true },
        { srgb1,  srgb,   opaque, premul, false,false,false, true,false },
        { srgb1,  srgb, unpremul, premul, false,false,false, true, true },
        { srgb1, adobe,   opaque, premul, false,false, true, true,false },
        { srgb1, adobe, unpremul, premul, false,false, true, true, true },

        // Now test non-premul outputs.
        { srgb , srgb  , premul, unpremul, true,false,false,false,false },
        { srgb , srgb1 , premul, unpremul, true, true,false,false,false },
        { srgb1, adobe1, premul, unpremul, true,false, true,false,false },
        { srgb , adobe1, premul, unpremul, true, true, true,false,false },
        { srgb1, srgb  , premul, unpremul, true,false,false, true,false },
        { srgb , srgb22, premul, unpremul, true, true,false, true,false },
        { srgb1, adobe , premul, unpremul, true,false, true, true,false },
        { srgb , adobe , premul, unpremul, true, true, true, true,false },

        // Opaque outputs are treated as the same alpha type as the source input.
        // TODO: we'd really like to have a good way of explaining why we think this is useful.
        { srgb , srgb  , premul, opaque, false,false,false,false,false },
        { srgb , srgb1 , premul, opaque,  true, true,false,false, true },
        { srgb1, adobe1, premul, opaque, false,false, true,false,false },
        { srgb , adobe1, premul, opaque,  true, true, true,false, true },
        { srgb1, srgb  , premul, opaque,  true,false,false, true, true },
        { srgb , srgb22, premul, opaque,  true, true,false, true, true },
        { srgb1, adobe , premul, opaque,  true,false, true, true, true },
        { srgb , adobe , premul, opaque,  true, true, true, true, true },

        { srgb , srgb  , unpremul, opaque, false,false,false,false,false },
        { srgb , srgb1 , unpremul, opaque, false, true,false,false,false },
        { srgb1, adobe1, unpremul, opaque, false,false, true,false,false },
        { srgb , adobe1, unpremul, opaque, false, true, true,false,false },
        { srgb1, srgb  , unpremul, opaque, false,false,false, true,false },
        { srgb , srgb22, unpremul, opaque, false, true,false, true,false },
        { srgb1, adobe , unpremul, opaque, false,false, true, true,false },
        { srgb , adobe , unpremul, opaque, false, true, true, true,false },

        { rec2020_pq_203, srgb          , premul, premul, true , true , true , true , true  },
        { rec2020_pq_203, rec2020_pq_203, premul, premul, false, false, false, false, false },
        { rec2020_pq_203, rec2020_pq_100, premul, premul, true , true , true , true , true  },
        { rec2020_pq_203, p3_pq_203     , premul, premul, true , true , true , true , true  },

        { rec2020_hlg_203, srgb           , premul, premul, true , true , true , true , true , true , false },
        { rec2020_hlg_12x, srgb           , premul, premul, true , true , true , true , true , false, false },
        { srgb           , rec2020_hlg_12x, premul, premul, true , true , true , true , true , false, false },
        { rec2020_hlg_203, rec2020_pq_203 , premul, premul, true , true , true , true , true , true , false },
        { rec2020_hlg_10a, rec2020_hlg_10b, premul, premul, true , true , false, true , true , false, false },
        { rec2020_hlg_203, rec2020_hlg_203, premul, premul, false, false, false, false, false, false, false },
        { rec2020_hlg_203, rec2020_hlg_12x, premul, premul, true , true , true , true , true , true , false },
    };

    uint32_t tested = 0x00000000;
    for (const Test& t : tests) {
        SkColorSpaceXformSteps steps(t.src.get(), t.srcAT, t.dst.get(), t.dstAT);
        REPORTER_ASSERT(r, steps.fFlags.unpremul        == t.unpremul);
        REPORTER_ASSERT(r, steps.fFlags.linearize       == t.linearize);
        REPORTER_ASSERT(r, steps.fFlags.gamut_transform == t.gamut_transform);
        REPORTER_ASSERT(r, steps.fFlags.encode          == t.encode);
        REPORTER_ASSERT(r, steps.fFlags.premul          == t.premul);
        REPORTER_ASSERT(r, steps.fFlags.src_ootf        == t.src_ootf);
        REPORTER_ASSERT(r, steps.fFlags.dst_ootf        == t.dst_ootf);

        uint32_t bits = (uint32_t)t.unpremul        << 0
                      | (uint32_t)t.linearize       << 1
                      | (uint32_t)t.gamut_transform << 2
                      | (uint32_t)t.encode          << 3
                      | (uint32_t)t.premul          << 4;
        tested |= (1<<bits);
    }

    // We'll check our test cases cover all 2^5 == 32 possible outputs (excluding interactions
    // with the HLG OOTF).
    for (uint32_t t = 0; t < 32; t++) {
        if (tested & (1<<t)) {
            continue;
        }

        // There are a couple impossible outputs, so consider those bits tested.
        //
        // Unpremul then premul should be optimized away to a noop, so 0b10001 isn't possible.
        // A gamut transform in the middle is fine too, so 0b10101 isn't possible either.
        if (t == 0b10001 || t == 0b10101) {
            continue;
        }

        ERRORF(r, "{ xxx, yyy, at, %s,%s,%s,%s,%s }, not covered",
                (t& 1) ? " true" : "false",
                (t& 2) ? " true" : "false",
                (t& 4) ? " true" : "false",
                (t& 8) ? " true" : "false",
                (t&16) ? " true" : "false");
    }
}

DEF_TEST(SkColorSpaceXformStepsApplyPQ, r) {
    auto rec2020_pq_203 = SkColorSpace::MakeRGB(trfn_pq_203(), SkNamedGamut::kRec2020),
         rec2020_pq_100 = SkColorSpace::MakeRGB(trfn_pq_100(), SkNamedGamut::kRec2020),
         rec2020_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kRec2020),
         p3_pq_100 = SkColorSpace::MakeRGB(trfn_pq_100(), SkNamedGamut::kDisplayP3),
         p3_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kDisplayP3);
    constexpr float kPq100 = 0.508078421517399f;
    constexpr float kPq203 = 0.5806888810416109f;
    constexpr float kPq1000 = 0.751827096247041f;

    // Convert to linear with HDR reference white at 203 nits.
    {
        SkColorSpaceXformSteps steps(rec2020_pq_203.get(), kUnpremul_SkAlphaType,
                                     rec2020_linear.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {kPq100, kPq203, kPq1000, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 100/203.f, 203/203.f, 1000/203.f));
    }

    // Convert from linear with HDR reference white at 100 nits.
    {
        SkColorSpaceXformSteps steps(rec2020_linear.get(), kUnpremul_SkAlphaType,
                                     rec2020_pq_100.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {1.f, 2.03f, 10.f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, kPq100, kPq203, kPq1000));
    }

    // Convert PQ at 203 to PQ at 100.
    {
        SkColorSpaceXformSteps steps(rec2020_pq_203.get(), kUnpremul_SkAlphaType,
                                     rec2020_pq_100.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {kPq203, kPq203, kPq203, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, kPq100, kPq100, kPq100));
    }
}

DEF_TEST(SkColorSpaceXformStepsApplyHLG, r) {
    auto rec2020_hlg_12x = SkColorSpace::MakeRGB(trfn_hlg_12x(), SkNamedGamut::kRec2020),
         rec2020_hlg_203 = SkColorSpace::MakeRGB(trfn_hlg_203(), SkNamedGamut::kRec2020),
         rec2020_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kRec2020),
         srgb_hlg_203 = SkColorSpace::MakeRGB(trfn_hlg_203(), SkNamedGamut::kSRGB),
         srgb_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kSRGB);

    // Reference HLG has white at 75%.
    {
        SkColorSpaceXformSteps steps(rec2020_hlg_203.get(), kUnpremul_SkAlphaType,
                                     rec2020_linear.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {0.75f, 0.75f, 0.75f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 1.f, 1.f, 1.f));
    }
    {
        SkColorSpaceXformSteps steps(rec2020_linear.get(), kUnpremul_SkAlphaType,
                                     rec2020_hlg_203.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {1.f, 1.f, 1.f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 0.75f, 0.75f, 0.75f));
    }

    // Scene-referred (12x) HLG has white at 50%.
    {
        SkColorSpaceXformSteps steps(rec2020_hlg_12x.get(), kUnpremul_SkAlphaType,
                                     rec2020_linear.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {0.5f, 0.5f, 0.5f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 1.f, 1.f, 1.f));
    }
    {
        SkColorSpaceXformSteps steps(rec2020_linear.get(), kUnpremul_SkAlphaType,
                                     rec2020_hlg_12x.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {1.f, 1.f, 1.f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 0.5f, 0.5f, 0.5f));
    }

    // Testing the HLG OOTF in a non-rec2100 gamut.
    {
        SkColorSpaceXformSteps steps(srgb_hlg_203.get(), kUnpremul_SkAlphaType,
                                     srgb_linear.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {0.1f, 0.5f, 0.75f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 0.00989411f, 0.24735274f, 0.78647059f));
    }
    {
        SkColorSpaceXformSteps steps(srgb_linear.get(), kUnpremul_SkAlphaType,
                                     srgb_hlg_203.get(), kUnpremul_SkAlphaType);
        float rgba[4] = {0.00989411f, 0.24735274f, 0.78647059f, 1.f};
        steps.apply(rgba);
        REPORTER_ASSERT(r, rgb_close(rgba, 0.1f, 0.5f, 0.75f));
    }
}
