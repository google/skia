/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

// Tests that a drawing with MSAA will have contents retained between recordings.
// This is for testing MSAA load from resolve feature.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleRetainTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            16, 16, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(surfaceRecorder.get(), surfaceImageInfo);

    // Clear entire surface to red
    SkCanvas* surfaceCanvas = surface->getCanvas();
    surfaceCanvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> surfaceRecording = surfaceRecorder->snap();
    // Flush the clearing
    context->insertRecording({surfaceRecording.get()});

    // Draw a blue path. The old red background should be retained between recordings.
    SkPaint paint;
    paint.setStrokeWidth(3);
    paint.setColor(SkColors::kBlue);
    paint.setStyle(SkPaint::Style::kStroke_Style);

    SkPath path;
    constexpr int kPathPoints[][2] = {
        {3, 2},
        {3, 4},
        {6, 8},
        {3, 15},
    };

    for (size_t i = 0; i < std::size(kPathPoints); ++i) {
        path.lineTo(kPathPoints[i][0], kPathPoints[i][1]);
    }

    surfaceCanvas->drawPath(path, paint);

    std::unique_ptr<Recording> surfaceRecording2 = surfaceRecorder->snap();
    // Play back recording.
    context->insertRecording({surfaceRecording2.get()});

    // Read pixels.
    SkBitmap bitmap;
    bitmap.allocPixels(surfaceImageInfo);
    if (!surface->readPixels(bitmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Verify recording was replayed.
    REPORTER_ASSERT(reporter, bitmap.getColor4f(8, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(0, 8) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(15, 14) == SkColors::kRed);

    // Verify points on the path have blue color. We don't verify last point because it is on the
    // edge of the path thus might have blurry color.
    for (size_t i = 0; i < std::size(kPathPoints) - 1; ++i) {
        REPORTER_ASSERT(reporter,
                        bitmap.getColor4f(kPathPoints[i][0], kPathPoints[i][1]) == SkColors::kBlue);
    }
}

}  // namespace skgpu::graphite
