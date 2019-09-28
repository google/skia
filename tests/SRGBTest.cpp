/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "tests/Test.h"

#include <math.h>

DEF_TEST(srgb_roundtrip, r) {
    for (int normalized = 0; normalized < 2; normalized++) {
        uint32_t reds[256];
        for (int i = 0; i < 256; i++) {
            reds[i] = i;
        }

        SkRasterPipeline_MemoryCtx ptr = { reds, 0 };

        sk_sp<SkColorSpace> sRGB = SkColorSpace::MakeSRGB(),
                            linear = sRGB->makeLinearGamma();
        const SkAlphaType upm = kUnpremul_SkAlphaType;

        SkColorSpaceXformSteps linearize{  sRGB.get(),upm,  linear.get(),upm},
                               reencode {linear.get(),upm,    sRGB.get(),upm};

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_8888,  &ptr);
        linearize.apply(&p, !!normalized);
        reencode .apply(&p, !!normalized);
        p.append(SkRasterPipeline::store_8888, &ptr);

        p.run(0,0,256,1);

        for (int i = 0; i < 256; i++) {
            if (reds[i] != (uint32_t)i) {
                ERRORF(r, "%d doesn't round trip, %d", i, reds[i]);
            }
        }
    }
}

DEF_TEST(srgb_edge_cases, r) {
    // We need to run at least 4 pixels to make sure we hit all specializations.
    float colors[4][4] = { {0,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
    auto& color = colors[0];

    SkRasterPipeline_MemoryCtx dst = { &color, 0 };

    sk_sp<SkColorSpace> sRGB = SkColorSpace::MakeSRGB(),
                        linear = sRGB->makeLinearGamma();
    const SkAlphaType upm = kUnpremul_SkAlphaType;

    SkColorSpaceXformSteps steps {linear.get(),upm,    sRGB.get(),upm};

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, color);
    steps.apply(&p, true/*inputs are normalized*/);
    p.append(SkRasterPipeline::store_f32, &dst);
    p.run(0,0,4,1);

    if (color[0] != 0.0f) {
        ERRORF(r, "expected to_srgb() to map 0.0f to 0.0f, got %f", color[0]);
    }
    if (color[1] != 1.0f) {
        float f = color[1];
        uint32_t x;
        memcpy(&x, &f, 4);
        ERRORF(r, "expected to_srgb() to map 1.0f to 1.0f, got %f (%08x)", color[1], x);
    }
}

// Linearize and then re-encode pixel values, testing that the output is close to the input.
static void test_roundtripping(skiatest::Reporter* r,
                               sk_sp<SkColorSpace> cs,
                               float range,
                               float tolerance,
                               bool normalized) {
    static const int kSteps = 128;
    SkColor4f rgba[kSteps];

    auto expected = [=](int i) {
        float scale = range / (3*kSteps);
        return SkColor4f{
            (3*i+0) * scale,
            (3*i+1) * scale,
            (3*i+2) * scale,
            1.0f,
        };
    };

    for (int i = 0; i < kSteps; i++) {
        rgba[i] = expected(i);
    }

    SkRasterPipeline_MemoryCtx ptr = { rgba, 0 };

    sk_sp<SkColorSpace> linear = cs->makeLinearGamma();
    const SkAlphaType upm = kUnpremul_SkAlphaType;

    SkColorSpaceXformSteps linearize{    cs.get(),upm,  linear.get(),upm},
                           reencode {linear.get(),upm,      cs.get(),upm};

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f32,  &ptr);
    linearize.apply(&p, normalized);
    reencode .apply(&p, normalized);
    p.append(SkRasterPipeline::store_f32, &ptr);
    p.run(0,0,kSteps,1);

    auto close = [=](float x, float y) {
        return x == y
            || (x/y < tolerance && y/x < tolerance);
    };

    for (int i = 0; i < kSteps; i++) {
        SkColor4f want = expected(i);
    #if 0
        SkDebugf("got %g %g %g, want %g %g %g\n",
                 rgba[i].fR, rgba[i].fG, rgba[i].fB,
                 want.fR, want.fG, want.fB);
    #endif
        REPORTER_ASSERT(r, close(rgba[i].fR, want.fR));
        REPORTER_ASSERT(r, close(rgba[i].fG, want.fG));
        REPORTER_ASSERT(r, close(rgba[i].fB, want.fB));
    }
}

DEF_TEST(srgb_roundtrip_extended, r) {
    // We're lying when we set normalized=true, but it allows us to test the to_srgb/from_srgb path.
    test_roundtripping(r,  SkColorSpace::MakeSRGB(), 2.0f, 1.025f, true);

    // This normalized=false path should have much better round-tripping properties.
    test_roundtripping(r,  SkColorSpace::MakeSRGB(), 10000.0f, 1.001f, false);
}
