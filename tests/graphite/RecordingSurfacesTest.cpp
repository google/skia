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
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

// Tests that a Recording can be replayed to a provided surface.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTest, reporter, context) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            16, 16, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(surfaceRecorder.get(), surfaceImageInfo);
    Surface* graphiteSurface = static_cast<Surface*>(surface.get());
    const TextureInfo& textureInfo = graphiteSurface->backingTextureProxy()->textureInfo();

    // Flush the initial clear added by MakeGraphite.
    std::unique_ptr<skgpu::graphite::Recording> surfaceRecording = surfaceRecorder->snap();
    context->insertRecording({surfaceRecording.get()});

    // Snap a recording without a bound target.
    const SkImageInfo recordingImageInfo = surfaceImageInfo.makeDimensions(SkISize::Make(8, 16));
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkCanvas* canvas = recorder->makeDeferredCanvas(recordingImageInfo, textureInfo);
    canvas->clear(SkColors::kRed);
    // Can't make another canvas before snapping.
    REPORTER_ASSERT(reporter,
                    recorder->makeDeferredCanvas(recordingImageInfo, textureInfo) == nullptr);
    std::unique_ptr<Recording> recording = recorder->snap();

    // Play back recording.
    context->insertRecording({recording.get(), surface.get()});

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surfaceImageInfo);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Verify recording was replayed and is now uninstantiated.
    REPORTER_ASSERT(reporter, pixmap.getColor4f(0, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, pixmap.getColor4f(8, 0) == SkColors::kTransparent);
    REPORTER_ASSERT(reporter, !recording->isTargetProxyInstantiated());
}

}  // namespace skgpu::graphite
