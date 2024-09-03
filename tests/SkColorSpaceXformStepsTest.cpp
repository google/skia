/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Surface.h"
#endif  // defined(SK_GRAPHITE)

#include "src/core/SkColorSpaceXformSteps.h"
#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "tools/graphite/GraphiteTestContext.h"
#endif  // defined(SK_GRAPHITE)

#include <cstdint>
#include <functional>

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

static bool rgba_close(const float* expected, const float* actual) {
    // Allow 1% relative error.
    constexpr float kEpsilon = 0.01f;
    constexpr float kMinDenom = 0.001f;
    return std::abs(expected[0] - actual[0]) / std::max(expected[0], kMinDenom) < kEpsilon &&
           std::abs(expected[1] - actual[1]) / std::max(expected[1], kMinDenom) < kEpsilon &&
           std::abs(expected[2] - actual[2]) / std::max(expected[2], kMinDenom) < kEpsilon &&
           std::abs(expected[3] - actual[3]) / std::max(expected[3], kMinDenom) < kEpsilon;
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

// Body of test to ensure that SkColorSpaceXformSteps::apply, raster, ganesh, and graphite all
// produce the same results for color space conversions.
static void run_color_space_xform_test(
      skiatest::Reporter* reporter,
      std::optional<std::function<sk_sp<SkSurface>(const SkImageInfo&)>> make_surface =
          std::nullopt,
      std::optional<std::function<sk_sp<SkImage>(sk_sp<SkImage>)>> upload_image =
          std::nullopt) {
    constexpr int kWidth = 2;
    constexpr int kHeight = 2;
    constexpr float kPq100 = 0.508078421517399f;
    constexpr float kPq203 = 0.5806888810416109f;
    constexpr float kPq1000 = 0.751827096247041f;

    auto rec2020_pq_203 = SkColorSpace::MakeRGB(trfn_pq_203(), SkNamedGamut::kRec2020),
         rec2020_pq_100 = SkColorSpace::MakeRGB(trfn_pq_100(), SkNamedGamut::kRec2020),
         rec2020_hlg_12x = SkColorSpace::MakeRGB(trfn_hlg_12x(), SkNamedGamut::kRec2020),
         rec2020_hlg_203 = SkColorSpace::MakeRGB(trfn_hlg_203(), SkNamedGamut::kRec2020),
         rec2020_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear,
                                                SkNamedGamut::kRec2020),
         srgb_hlg_203 = SkColorSpace::MakeRGB(trfn_hlg_203(), SkNamedGamut::kSRGB),
         srgb_linear = SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kSRGB);

    const struct Rec {
        sk_sp<SkColorSpace> src_cs = nullptr;
        float src_rgba[4] = {0.f, 0.f, 0.f, 0.f};
        sk_sp<SkColorSpace> dst_cs = nullptr;
        float expected_rgba[4] = {0.f, 0.f, 0.f, 0.f};
    } recs[] = {
        {
            rec2020_hlg_203, {0.75f, 0.75f, 0.75f, 1.f},
            rec2020_linear,  {1.f,   1.f,   1.f,   1.f},
        },
        {
            rec2020_linear,  {1.f,   1.f,   1.f,   1.f},
            rec2020_hlg_203, {0.75f, 0.75f, 0.75f, 1.f},
        },
        {
            rec2020_hlg_12x, {0.5f, 0.5f, 0.5f, 1.f},
            rec2020_linear,  {1.f,  1.f,  1.f,  1.f},
        },
        {
            rec2020_linear,  {1.f,  1.f,  1.f,  1.f},
            rec2020_hlg_12x, {0.5f, 0.5f, 0.5f, 1.f},
        },
        {
            srgb_hlg_203, {0.1f,        0.5f,        0.75f,       1.f},
            srgb_linear,  {0.00989411f, 0.24735274f, 0.78647059f, 1.f},
        },
        {
            srgb_linear,  {0.00989411f, 0.24735274f, 0.78647059f, 1.f},
            srgb_hlg_203, {0.1f,        0.5f,        0.75f,       1.f},
        },
        {
            rec2020_pq_203, {kPq100,    kPq203,    kPq1000,    1.f},
            // Note: the blue expected component should be 1000/203, but the skcms formulation
            // of PQ evaluates to this.
            // TODO(https://issues.skia.org/issues/420956739): Investiage this.
            rec2020_linear, {100/203.f, 203/203.f, 1003/203.f, 1.f},
        },
        {
            rec2020_pq_203, {kPq203, kPq203, kPq203, 1.f},
            rec2020_pq_100, {kPq100, kPq100, kPq100, 1.f},
        },
        // Note: the next two tests use color values outside of [0,1], so this will fail if
        // there is clamping to [0,1].
        {
            rec2020_linear, {1.f,    2.03f,  10.f,    1.f},
            rec2020_pq_100, {kPq100, kPq203, kPq1000, 1.f},
        },
        {
            rec2020_pq_100, {kPq100, kPq203, kPq1000, 1.f},
            rec2020_linear, {1.f,    2.03f,  10.f,    1.f},
        },
    };

    for (const auto& rec : recs) {
        if (!make_surface.has_value()) {
            SkColorSpaceXformSteps steps(rec.src_cs.get(), kUnpremul_SkAlphaType,
                                         rec.dst_cs.get(), kUnpremul_SkAlphaType);
            float xform_rgba[4] = {
                rec.src_rgba[0], rec.src_rgba[1], rec.src_rgba[2], rec.src_rgba[3]};
            steps.apply(xform_rgba);
            REPORTER_ASSERT(reporter, rgba_close(xform_rgba,  rec.expected_rgba));
            continue;
        }

        // Create an F16 image with the specified color. If we do not convert explicitly to
        // F16, then when the GPU based tests attempt to implicitly convert to F32 textures
        // and fail, they fall back to converting to 8888, which results in clamping and
        // ginormous error. Write the values directly (rather than ask SkColor4fs) to ensure
        // we are testing the full pipeline.
        sk_sp<SkImage> src_image;
        {
            auto src_info = SkImageInfo::Make(kWidth, kHeight, kRGBA_F32_SkColorType,
                                              kPremul_SkAlphaType, rec.src_cs);

            // Write the pixels as F32.
            SkBitmap src_bm_f32;
            src_bm_f32.allocPixels(src_info);
            src_bm_f32.eraseColor(SK_ColorTRANSPARENT);
            for (int x = 0; x < kWidth; ++x) {
                for (int y = 0; y < kHeight; ++y) {
                    float* p = reinterpret_cast<float*>(
                        src_bm_f32.pixmap().writable_addr(x, y));
                    for (int c = 0; c < 4; ++c) {
                        p[c] = rec.src_rgba[c];
                    }
                }
            }
            SkBitmap src_bm;
            src_bm.allocPixels(src_info.makeColorType(kRGBA_F16_SkColorType));
            bool rp_result = src_bm_f32.readPixels(src_bm.pixmap(), 0, 0);
            REPORTER_ASSERT(reporter, rp_result);
            src_bm.setImmutable();

            src_image = SkImages::RasterFromBitmap(src_bm);
        }
        if (upload_image.has_value()) {
            src_image = upload_image.value()(src_image);
            REPORTER_ASSERT(reporter, src_image);
        }

        // Render the image to an F16 target.
        auto dst_info = SkImageInfo::Make(kWidth, kHeight, kRGBA_F16_SkColorType,
                                          kPremul_SkAlphaType, rec.dst_cs);
        auto dst_surface = make_surface.value()(dst_info);
        if (!dst_surface) {
            continue;
        }
        dst_surface->getCanvas()->clear(SK_ColorWHITE);
        dst_surface->getCanvas()->drawImage(src_image, 0, 0);

        // Read back to an F32 target.
        const SkImageInfo rb_info = dst_info.makeColorType(kRGBA_F32_SkColorType);
        SkBitmap rb_bm;
        rb_bm.allocPixels(rb_info);
        bool rb_result = dst_surface->readPixels(rb_bm.pixmap(), 0, 0);
        REPORTER_ASSERT(reporter, rb_result);

        const float* rb_rgba = reinterpret_cast<const float*>(rb_bm.pixmap().addr(0, 0));
        REPORTER_ASSERT(reporter, rgba_close(rb_rgba, rec.expected_rgba));
    }
}

// Test color space space conversion using SkColorSpaceXformSteps::apply.
DEF_TEST(SkColorSpaceXform_Apply, reporter) {
    run_color_space_xform_test(reporter);
}

// Test color space space conversion using raster.
DEF_TEST(SkColorSpaceXform_Raster, reporter) {
    auto make_surface = [&](const SkImageInfo& info) {
        return SkSurfaces::Raster(info);
    };
    run_color_space_xform_test(reporter, make_surface);
}

#if defined(SK_GANESH)
// Test color space conversion using Ganesh.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkColorSpaceXform_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* rContext = ctxInfo.directContext();
    auto make_surface = [&](const SkImageInfo& info) {
        return SkSurfaces::RenderTarget(
            rContext, skgpu::Budgeted::kNo, info, 0, kTopLeft_GrSurfaceOrigin, nullptr);
    };
    auto upload_image = [&](sk_sp<SkImage> image) {
      return SkImages::TextureFromImage(rContext, image.get());
    };
    run_color_space_xform_test(reporter, make_surface, upload_image);
}
#endif

#if defined(SK_GRAPHITE)
// Test color space conversion using Graphite.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SkColorSpaceXform_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    using namespace skgpu::graphite;
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    auto make_surface = [&](const SkImageInfo& info) {
        return SkSurfaces::RenderTarget(recorder.get(), info);
    };
    auto upload_image = [&](sk_sp<SkImage> image) {
      return SkImages::TextureFromImage(recorder.get(), image.get(), {false});
    };
    run_color_space_xform_test(reporter, make_surface, upload_image);
}
#endif

