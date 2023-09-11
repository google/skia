/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cmath>
#include <vector>

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
#include "include/gpu/GpuTypes.h"
#endif

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
struct GrContextOptions;
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#endif

static void check_isaimage(skiatest::Reporter* reporter, SkShader* shader,
                           int expectedW, int expectedH,
                           SkTileMode expectedX, SkTileMode expectedY,
                           const SkMatrix& expectedM) {
    SkTileMode tileModes[2];
    SkMatrix localM;

    // wack these so we don't get a false positive
    localM.setScale(9999, -9999);
    tileModes[0] = tileModes[1] = (SkTileMode)99;

    SkImage* image = shader->isAImage(&localM, tileModes);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->width() == expectedW);
    REPORTER_ASSERT(reporter, image->height() == expectedH);
    REPORTER_ASSERT(reporter, localM == expectedM);
    REPORTER_ASSERT(reporter, tileModes[0] == expectedX);
    REPORTER_ASSERT(reporter, tileModes[1] == expectedY);
}

DEF_TEST(Shader_isAImage, reporter) {
    const int W = 100;
    const int H = 100;
    SkBitmap bm;
    bm.allocN32Pixels(W, H);
    auto img = bm.asImage();
    const SkMatrix localM = SkMatrix::Scale(2, 3);
    const SkTileMode tmx = SkTileMode::kRepeat;
    const SkTileMode tmy = SkTileMode::kMirror;

    auto shader0 = bm.makeShader(tmx, tmy, SkSamplingOptions(), localM);
    auto shader1 = bm.asImage()->makeShader(tmx, tmy, SkSamplingOptions(), localM);

    check_isaimage(reporter, shader0.get(), W, H, tmx, tmy, localM);
    check_isaimage(reporter, shader1.get(), W, H, tmx, tmy, localM);
}

// Make sure things are ok with just a single leg.
DEF_TEST(ComposeShaderSingle, reporter) {
    SkBitmap srcBitmap;
    srcBitmap.allocN32Pixels(10, 10);
    srcBitmap.eraseColor(SK_ColorRED);
    SkCanvas canvas(srcBitmap);
    SkPaint p;
    p.setShader(SkShaders::Blend(SkBlendMode::kClear,
                                 SkShaders::Empty(),
                                 SkShaders::MakeFractalNoise(1.0f, 1.0f, 2, 0.0f)));
    SkRRect rr;
    SkVector rd[] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    rr.setRectRadii({0, 0, 0, 0}, rd);
    canvas.drawRRect(rr, p);
}

// Tests that nested blending will render as expected.
static void test_nested_blends(skiatest::Reporter* reporter, SkSurface* surface) {
    auto [redEffect, redError] = SkRuntimeEffect::MakeForShader(SkString(R"(
        half4 main(float2 coord) {
            return half4(1, 0, 0, 1);
        }
    )"));

    auto [greenEffect, greenError] = SkRuntimeEffect::MakeForShader(SkString(R"(
        half4 main(float2 coord) {
            return half4(0, 1, 0, 1);
        }
    )"));

    auto [blendEffect, blenderError] = SkRuntimeEffect::MakeForBlender(SkString(R"(
        half4 main(half4 src, half4 dst) {
            return (src + dst) * 0.5;
        }
    )"));

    auto [nestedBlendEffect, nestedBlenderError] = SkRuntimeEffect::MakeForBlender(SkString(R"(
        uniform blender child_blender;
        half4 main(half4 src, half4 dst) {
            return (child_blender.eval(src, dst) + dst) * 0.5;
        }
    )"));

    sk_sp<SkShader> redShader = redEffect->makeShader(nullptr, {});
    sk_sp<SkShader> greenShader = greenEffect->makeShader(nullptr, {});
    sk_sp<SkBlender> blender = blendEffect->makeBlender(nullptr);
    std::vector<SkRuntimeEffect::ChildPtr> children = {SkRuntimeEffect::ChildPtr(blender)};
    sk_sp<SkBlender> nestedBlender = nestedBlendEffect->makeBlender(nullptr, children);

    SkPaint paint;
    paint.setShader(SkShaders::Blend(nestedBlender, greenShader, redShader));
    paint.setBlender(blender);

    // Do the drawing.
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawPaint(paint);

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Check the resulting blended color.
    // First, in the paint's shader, red and green are averaged in the child blender to get
    // (0.5, 0.5, 0, 1), which is then averaged with green in the parent blender to get
    // (0.25, 0.75, 0, 1). Then, in the paint's blender this is averaged with a transparent
    // background to get (0.125, 0.375, 0, 0.5) and then unpremuled to get (0.25, 0.75, 0, 0.5).
    constexpr SkColor4f kExpected = {0.25f, 0.75f, 0.0f, 0.5f};
    constexpr float kTolerance[4] = {0.01f, 0.01f, 0.0f, 0.01f};
    SkColor4f color = pixmap.getColor4f(0, 0);
    for (int i = 0; i < 4; ++i) {
        if (std::abs(color[i] - kExpected[i]) > kTolerance[i]) {
            ERRORF(reporter,
                   "Wrong color, expected (%.2f %.2f %.2f %.2f), actual (%.2f, %.2f, %.2f, %.2f)",
                   kExpected.fR, kExpected.fG, kExpected.fB, kExpected.fA,
                   color.fR, color.fG, color.fB, color.fA);
            break;
        }
    }
}

DEF_TEST(ShaderTestNestedBlendsCpu, reporter) {
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(1, 1),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(ii);
    test_nested_blends(reporter, surface.get());
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ShaderTestNestedBlendsGanesh,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(1, 1),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    GrDirectContext* context = contextInfo.directContext();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, ii);
    test_nested_blends(reporter, surface.get());
}
#endif

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ShaderTestNestedBlendsGraphite, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    using namespace skgpu::graphite;

    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(1, 1),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    test_nested_blends(reporter, surface.get());
}
#endif
