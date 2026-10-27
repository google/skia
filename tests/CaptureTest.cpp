/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/capture/SkCapture.h"

#include <memory>

namespace {
void enable_capture(skgpu::graphite::ContextOptions* options) { options->fEnableCapture = true; }
}  // namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkCapture_DoNotCaptureEmptyAssets,
                                           /* filter= */ nullptr,
                                           reporter,
                                           context,
                                           /* test_ctx= */,
                                           /* options_name= */,
                                           enable_capture,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    // TODO(b/573568909): Replace this setup code with generalized APIs down the road
    // (e.g., replace the Graphite-specific recorder with SkRecorder, and replace
    // SkSurfaces::RenderTarget with recorder.makeSurface()) to support non-graphite backends.
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    context->startCapture();

    // Draw something to be captured.
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeWH(50, 50), paint);

    // Snapping and inserting should capture the recorded draw.
    std::unique_ptr<skgpu::graphite::Recording> recording1 = recorder->snap();
    REPORTER_ASSERT(reporter, recording1);
    REPORTER_ASSERT(reporter, context->insertRecording({recording1.get()}));

    // A subsequent snap with no intervening draws should not capture an empty asset.
    std::unique_ptr<skgpu::graphite::Recording> recording2 = recorder->snap();
    REPORTER_ASSERT(reporter, recording2);
    REPORTER_ASSERT(reporter, context->insertRecording({recording2.get()}));

    // Ending capture without any new draws should not add an empty asset.
    sk_sp<SkCapture> capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 1);
    REPORTER_ASSERT(reporter, capture->getMetadata().numRecordingCaptures == 2);

    // Start a new capture and verify that subsequent draws are captured without adding empty
    // assets.
    context->startCapture();
    canvas->drawRect(SkRect::MakeWH(25, 25), paint);
    std::unique_ptr<skgpu::graphite::Recording> recording3 = recorder->snap();
    REPORTER_ASSERT(reporter, recording3);
    REPORTER_ASSERT(reporter, context->insertRecording({recording3.get()}));

    // Draw again after snapping; the draw should be captured on endCapture() without adding
    // empty assets.
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), paint);

    capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 2);
    REPORTER_ASSERT(reporter, capture->getMetadata().numRecordingCaptures == 1);
}
#endif  // defined(SK_GRAPHITE)
