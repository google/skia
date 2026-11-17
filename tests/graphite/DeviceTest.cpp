/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/draw/DrawListBase.h"

namespace skgpu::graphite {

// Tests that vertex transparency will affect draw order.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestVertexTransparency, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    // Set up transparent vertices, in a 5px wide by 10px tall rectangle.
    static constexpr int kVertexCount = 5;
    SkPoint positions[kVertexCount];
    positions[0].set(2.5, 5);
    positions[1].set(0, 0);
    positions[2].set(5, 0);
    positions[3].set(5, 10);
    positions[4].set(0, 10);

    static constexpr int kIndexCount = 6;
    static constexpr uint16_t kIndices[kIndexCount] = {0, 1, 2, 3, 4, 1};

    SkColor colors[kVertexCount];
    for (size_t i = 0; i < kVertexCount; ++i) {
        colors[i] = 0x7F00FF00;
    }

    auto v = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
                                  kVertexCount,
                                  positions,
                                  nullptr,
                                  colors,
                                  kIndexCount,
                                  kIndices);

    // Draw vertices at x = 0.
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(10, 10),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawVertices(v, SkBlendMode::kDst, SkPaint());

    // Draw a square that will overlap both vertex draws.
    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 10, 10), redPaint);

    // Draw vertices at x = 5.
    canvas->translate(5, 0);
    canvas->drawVertices(v, SkBlendMode::kDst, SkPaint());

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(ii);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Check that draws weren't reordered to put vertex draws together.
    // The second vertex draw should have been 50% green on top of red.
    SkColor color = pixmap.getColor(9, 5);
    SkColor expected = 0xFF807F00;
    REPORTER_ASSERT(
            reporter, color == expected, "Wrong color, expected %08x, found %08x", expected, color);
}

// Tests that large clip stacks flush appropriately when handleable and safely skip draws when a
// single draw's effective clip elements exceed DrawListBase::kMaxRenderSteps.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestClipRenderStepLimits,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(64, 64),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    auto makeClipTriangle = [](int index) {
        float shift = index * 0.01f;
        SkPathBuilder builder;
        builder.moveTo(shift, 0.f);
        builder.lineTo(64.f, 8.f + shift * 0.5f);
        builder.lineTo(8.f, 64.f - shift);
        builder.close();
        return builder.detach();
    };

    // 1. Handleable case: accumulate clips and draws that fit within a single pass individually
    //    but require flushing across multiple batches when total render steps approach
    //    DrawListBase::kMaxRenderSteps, while also issuing multiple draws under the same clip stack
    //    to exercise already-drawn clip tracking (`undrawnClipElements < clipElements.size()`).
    for (int batch = 0; batch < 10; ++batch) {
        canvas->save();
        for (int subBatch = 0; subBatch < 2; ++subBatch) {
            for (int i = 0; i < 150; ++i) {
                canvas->clipPath(makeClipTriangle(batch * 1000 + subBatch * 150 + i),
                                 SkClipOp::kIntersect,
                                 /*doAntiAlias=*/false);
            }
            canvas->drawRect(SkRect::MakeWH(64, 64), paint);
            canvas->drawRect(SkRect::MakeWH(64, 64), paint);
        }
        canvas->restore();
    }

    // 2. Pathological case: a single draw with more effective clip elements than can fit in one
    //    DrawList even after flushing. This should be skipped without asserting or overflowing.
    constexpr int kPathologicalClipCount =
            DrawListBase::kMaxRenderSteps / Renderer::kMaxRenderSteps + 10;
    canvas->save();
    for (int i = 0; i < kPathologicalClipCount; ++i) {
        canvas->clipPath(makeClipTriangle(20000 + i), SkClipOp::kIntersect, /*doAntiAlias=*/false);
    }
    canvas->drawRect(SkRect::MakeWH(64, 64), paint);
    canvas->restore();

    // 3. Subsequent normal draw after restoring the pathological clip stack should still succeed.
    canvas->drawRect(SkRect::MakeWH(64, 64), paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
}

// Tests that promoting a full-device flood fill to clear() or discard() resets Device draw-ordering
// state so 16-bit CompressedPaintersOrder and PaintersDepth sequences do not saturate or wrap
// across clear-delimited batches.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestClearResetsOrderingState,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    if (context->supportsProtectedContent()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(64, 64),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    // 1. Accumulate > 65535 overlapping translucent draws across clear-delimited batches smaller
    //    than DrawListBase::kMaxRenderSteps so no automatic flush is triggered. Without resetting
    //    ordering state on clear(), CompressedPaintersOrder saturates at 0xFFFF.
    SkPaint translucentRed;
    translucentRed.setColor(0x80FF0000);
    for (int batch = 0; batch < 66; ++batch) {
        for (int i = 0; i < 1000; ++i) {
            canvas->drawRect(SkRect::MakeWH(64, 64), translucentRed);
        }
        canvas->clear(SK_ColorBLACK);
    }

    // Issue an opaque shader flood-fill, which promotes to fDC->discard() + a fullscreen draw.
    constexpr SkPoint kPts[] = {{0.f, 0.f}, {64.f, 64.f}};
    constexpr SkColor4f kGreenStops[] = {{0.f, 1.f, 0.f, 1.f}, {0.f, 0.95f, 0.f, 1.f}};
    SkPaint gradientPaint;
    gradientPaint.setShader(
            SkShaders::LinearGradient(kPts, {{kGreenStops, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(gradientPaint);

    // Follow with a translucent concave path using kStrokeAndFill_Style so stencil-then-cover is
    // chosen. If CompressedPaintersOrder saturated at 0xFFFF, next() wraps to 0 and this path sorts
    // ahead of the flood-fill and occludes it in the depth buffer.
    SkPath concavePath = SkPathBuilder()
                                 .moveTo(4.f, 4.f)
                                 .lineTo(60.f, 4.f)
                                 .lineTo(60.f, 60.f)
                                 .lineTo(32.f, 50.f)
                                 .lineTo(4.f, 60.f)
                                 .close()
                                 .detach();
    SkPaint pathPaint;
    pathPaint.setColor(0x80FF0000);
    pathPaint.setStyle(SkPaint::kStrokeAndFill_Style);
    pathPaint.setStrokeWidth(1.f);
    canvas->drawPath(concavePath, pathPaint);

    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(ii);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // 50% red blended over ~100% green should be opaque with both red and green ~120..135.
    SkColor center = pixmap.getColor(32, 32);
    REPORTER_ASSERT(reporter,
                    SkColorGetA(center) == 0xFF &&
                    SkColorGetR(center) >= 115 && SkColorGetR(center) <= 140 &&
                    SkColorGetG(center) >= 110 && SkColorGetG(center) <= 140 &&
                    SkColorGetB(center) == 0,
                    "Expected 50%% red over opaque green at (32, 32), found %08x",
                    center);

    // 2. Also verify PaintersDepth does not wrap across clears (e.g. even when DrawListLayer is
    //    enabled): advance 65534 depths across clears, then draw opaque blue followed by opaque
    //    green in the same pass. Without resetting fCurrentDepth on clear(), blue gets depth 0xFFFF
    //    (depthAsFloat 0.0f) and green wraps to 0 (depthAsFloat 1.0f), so green fails the depth
    //    test against the earlier blue draw.
    SkPaint opaqueBlue;
    opaqueBlue.setColor(SK_ColorBLUE);
    SkPaint opaqueGreen;
    opaqueGreen.setColor(SK_ColorGREEN);
    for (int batch = 0; batch < 65; ++batch) {
        for (int i = 0; i < 1008; ++i) {
            canvas->drawRect(SkRect::MakeXYWH(8, 8, 32, 32), opaqueGreen);
        }
        canvas->clear(SK_ColorRED);
    }
    for (int i = 0; i < 14; ++i) {
        canvas->drawRect(SkRect::MakeXYWH(8, 8, 32, 32), opaqueGreen);
    }
    canvas->clear(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(8, 8, 32, 32), opaqueBlue);
    canvas->drawRect(SkRect::MakeXYWH(8, 8, 32, 32), opaqueGreen);

    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }
    SkColor depthTestColor = pixmap.getColor(16, 16);
    REPORTER_ASSERT(reporter,
                    depthTestColor == SK_ColorGREEN,
                    "Expected opaque green at (16, 16), found %08x",
                    depthTestColor);
}

}  // namespace skgpu::graphite
