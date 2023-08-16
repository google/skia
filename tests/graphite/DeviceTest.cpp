/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkVertices.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"

namespace skgpu::graphite {

// Tests that vertex transparency will affect draw order.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestVertexTransparency, reporter, context,
                                   CtsEnforcement::kNextRelease) {
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

}  // namespace skgpu::graphite
