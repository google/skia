/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

using DrawCallback = std::function<void(SkCanvas*)>;

struct Expectation {
    int fX;
    int fY;
    SkColor4f fColor;
};

void run_test(skiatest::Reporter* reporter,
              Context* context,
              SkISize surfaceSize,
              SkISize recordingSize,
              SkISize replayOffset,
              DrawCallback draw,
              const std::vector<Expectation>& expectations) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            surfaceSize, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(surfaceRecorder.get(), surfaceImageInfo);
    Surface* graphiteSurface = static_cast<Surface*>(surface.get());
    const TextureInfo& textureInfo = graphiteSurface->backingTextureProxy()->textureInfo();

    // Flush the initial clear added by MakeGraphite.
    std::unique_ptr<skgpu::graphite::Recording> surfaceRecording = surfaceRecorder->snap();
    context->insertRecording({surfaceRecording.get()});

    // Snap a recording without a bound target.
    const SkImageInfo recordingImageInfo = surfaceImageInfo.makeDimensions(recordingSize);
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkCanvas* canvas = recorder->makeDeferredCanvas(recordingImageInfo, textureInfo);
    draw(canvas);

    // Can't make another canvas before snapping.
    REPORTER_ASSERT(reporter,
                    recorder->makeDeferredCanvas(recordingImageInfo, textureInfo) == nullptr);
    std::unique_ptr<Recording> recording = recorder->snap();

    // Play back recording.
    context->insertRecording(
            {recording.get(), surface.get(), {replayOffset.fWidth, replayOffset.fHeight}});

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surfaceImageInfo);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Veryify expectations are met and recording is uninstantiated.
    REPORTER_ASSERT(reporter, !recording->priv().isTargetProxyInstantiated());
    for (const Expectation& e : expectations) {
        SkColor4f color = pixmap.getColor4f(e.fX, e.fY);
#ifdef SK_DEBUG
        if (color != e.fColor) {
            SkDebugf("Wrong color\n\texpected: %f %f %f %f\n\tactual: %f %f %f %f",
                     color.fR,
                     color.fG,
                     color.fB,
                     color.fA,
                     e.fColor.fR,
                     e.fColor.fG,
                     e.fColor.fB,
                     e.fColor.fA);
        }
#endif
        REPORTER_ASSERT(reporter, color == e.fColor);
    }
}

// Tests that clear does not clear an entire replayed-to surface if recorded onto a smaller surface.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestClear, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkISize replayOffset = SkISize::Make(0, 0);

    auto draw = [](SkCanvas* canvas) { canvas->clear(SkColors::kRed); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kRed},
                                             {4, 0, SkColors::kTransparent}};

    run_test(reporter, context, surfaceSize, recordingSize, replayOffset, draw, expectations);
}

// Tests that writePixels is translated correctly when replayed with an offset.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestWritePixels, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(4, 4, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), paint);

    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkISize replayOffset = SkISize::Make(4, 0);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent},
                                             {4, 0, SkColors::kRed}};

    run_test(reporter, context, surfaceSize, recordingSize, replayOffset, draw, expectations);
}

// Tests that the result of writePixels is cropped correctly when offscreen.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestWritePixelsOffscreen, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(4, 4, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), paint);
    paint.setColor(SkColors::kGreen);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(2, 2, 2, 2), paint);

    SkISize surfaceSize = SkISize::Make(4, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkISize replayOffset = SkISize::Make(-2, -2);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kGreen}};

    run_test(reporter, context, surfaceSize, recordingSize, replayOffset, draw, expectations);
}

}  // namespace skgpu::graphite
