/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkRuntimeEffect.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#endif

namespace {

// Tests that draws to an F16 surface blend as expected.
void test_f16(skiatest::Reporter* reporter,
              std::function<sk_sp<SkSurface>(const SkImageInfo&)> createSurface) {
    // Some blend modes and their corresponding expected red channel output when blending premul src
    // (2, 0, 0, 0) with dst (0, 0, 0, 0) on an F16 surface.
    constexpr uint16_t kHalfFloat0 = 0x0000;
    constexpr uint16_t kHalfFloat1 = 0x3c00;
    constexpr uint16_t kHalfFloat2 = 0x4000;
    constexpr struct Expectation {
        SkBlendMode fBlendMode;
        uint16_t fRed;
    } kExpectations[19] = {
        { SkBlendMode::kClear,      kHalfFloat0 },
        { SkBlendMode::kSrc,        kHalfFloat2 },
        { SkBlendMode::kDst,        kHalfFloat0 },
        { SkBlendMode::kSrcOver,    kHalfFloat2 },
        { SkBlendMode::kDstOver,    kHalfFloat2 },
        { SkBlendMode::kSrcIn,      kHalfFloat0 },
        { SkBlendMode::kDstIn,      kHalfFloat0 },
        { SkBlendMode::kSrcOut,     kHalfFloat2 },
        { SkBlendMode::kDstOut,     kHalfFloat0 },
        { SkBlendMode::kPlus,       kHalfFloat1 },
        { SkBlendMode::kScreen,     kHalfFloat2 },
    };

    // Create an F16 surface, if possible.
    SkImageInfo imageInfo = SkImageInfo::Make(
            SkISize::Make(1, 1), kRGBA_F16_SkColorType, SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = createSurface(imageInfo);
    if (!surface) {
        return;
    }

    for (const Expectation& expectation : kExpectations) {
        // Draw to the F16 surface.
        SkPaint paint;
        auto [effect, error] = SkRuntimeEffect::MakeForShader(SkString(R"(
            float4 main(vec2 xy) {
                return float4(2.0, 0.0, 0.0, 0.0);
            }
        )"));
        paint.setShader(effect->makeShader(nullptr, {}));
        paint.setBlendMode(expectation.fBlendMode);
        surface->getCanvas()->clear(SkColors::kTransparent);
        surface->getCanvas()->drawPaint(paint);

        // Read pixels.
        SkBitmap bitmap;
        SkPixmap pixmap;
        bitmap.allocPixels(imageInfo);
        SkAssertResult(bitmap.peekPixels(&pixmap));
        if (!surface->readPixels(pixmap, 0, 0)) {
            ERRORF(reporter, "readPixels failed");
            return;
        }

        // Check that the correct color was drawn.
        const uint16_t* channels = static_cast<const uint16_t*>(pixmap.addr());
        const uint16_t actual = channels[0];
        const uint16_t expected = expectation.fRed;
        REPORTER_ASSERT(reporter,
                        actual == expected,
                        "Wrong color with blend mode %s, expected %04x, found %04x",
                        SkBlendMode_Name(expectation.fBlendMode),
                        expected,
                        actual);
    }
}

}  // namespace

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(F16DrawTest_Ganesh,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    GrRecordingContext* context = contextInfo.directContext();
    test_f16(reporter, [context](const SkImageInfo& imageInfo) {
        return SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, imageInfo);
    });
}
#endif

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(F16DrawTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    skgpu::graphite::Recorder* recorderPtr = recorder.get();
    test_f16(reporter, [recorderPtr](const SkImageInfo& imageInfo) {
        return SkSurfaces::RenderTarget(recorderPtr, imageInfo);
    });
}
#endif
