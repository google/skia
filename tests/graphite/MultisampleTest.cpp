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
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

// Tests that a drawing with MSAA will have contents retained between recordings.
// This is for testing MSAA load from resolve feature.
// TODO(b/296420752): enable in CTS after debugging failure at coordinates 32,30.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleRetainTest, reporter, context,
                                   CtsEnforcement::kNever) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            33, 33, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

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

    constexpr int kPathPoints[][2] = {
        {9, 8},
        {9, 12},
        {14, 16},
        {10, 32},
    };

    SkPathBuilder builder;
    builder.moveTo(kPathPoints[0][0], kPathPoints[0][1]);
    for (size_t i = 0; i < std::size(kPathPoints); ++i) {
        builder.lineTo(kPathPoints[i][0], kPathPoints[i][1]);
    }
    SkPath path = builder.detach();

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
    REPORTER_ASSERT(reporter, bitmap.getColor4f(16, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(0, 16) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(32, 30) == SkColors::kRed);

    // Verify points on the path have blue color. We don't verify last point because it is on the
    // edge of the path thus might have blurry color.
    for (size_t i = 0; i < std::size(kPathPoints) - 1; ++i) {
        REPORTER_ASSERT(reporter,
                        bitmap.getColor4f(kPathPoints[i][0], kPathPoints[i][1]) == SkColors::kBlue);
    }
}

// Tests that multisampled rendering with LoadOp::Clear in one pass and LoadOp::Load in another pass
// works. With the Vulkan backend in particular, without
// VK_EXT_multisampled_render_to_single_sampled, the render pass with LoadOp::Load has an extra
// "unresolve" pass at the start.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleClearThenLoad, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            33, 33, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(surfaceRecorder.get(), surfaceImageInfo);

    // Clear entire surface to red
    SkCanvas* surfaceCanvas = surface->getCanvas();
    surfaceCanvas->clear(SkColors::kRed);

    // Draw a blue path over it. This render pass will use LoadOp::Clear because of the clear above.
    SkPaint paint;
    paint.setStrokeWidth(3);
    paint.setColor(SkColors::kBlue);
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setAntiAlias(true);

    // Note: The path is not a line, since that's optimized not to take the multisampling path.
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .lineTo(15, 15)
                  .lineTo(33, 33)
                  .detach();

    surfaceCanvas->drawPath(path, paint);

    std::unique_ptr<Recording> surfaceRecording = surfaceRecorder->snap();
    // Play back recording. This breaks the render pass.
    context->insertRecording({surfaceRecording.get()});

    // Draw another path. Because the contents of the surface need to be retained, this render pass
    // will use LoadOp::Load.
    SkPath path2 = SkPathBuilder()
                   .moveTo(33, 0)
                   .lineTo(15, 15)
                   .lineTo(0, 33)
                   .detach();

    surfaceCanvas->drawPath(path2, paint);

    std::unique_ptr<Recording> surfaceRecording2 = surfaceRecorder->snap();
    // Play back recording. Break the render pass again.
    context->insertRecording({surfaceRecording2.get()});

    // Verify results
    SkBitmap bitmap;
    bitmap.allocPixels(surfaceImageInfo);
    if (!surface->readPixels(bitmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Some points outside the paths
    REPORTER_ASSERT(reporter, bitmap.getColor4f(1, 16) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(16, 1) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(31, 16) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(16, 31) == SkColors::kRed);

    // Some points on the paths
    REPORTER_ASSERT(reporter, bitmap.getColor4f(1, 1) == SkColors::kBlue);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(31, 1) == SkColors::kBlue);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(1, 31) == SkColors::kBlue);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(31, 31) == SkColors::kBlue);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(16, 16) == SkColors::kBlue);
}

}  // namespace skgpu::graphite
