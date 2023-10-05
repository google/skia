/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkFont.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

const SkISize kSurfaceSize = { 64, 64 };

constexpr SkColor4f kBackgroundColor = SkColors::kWhite;

bool run_test(skiatest::Reporter* reporter,
              Context* context,
              Recorder* recorder) {
    SkImageInfo ii = SkImageInfo::Make(kSurfaceSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap result0, result1;
    result0.allocPixels(ii);
    result1.allocPixels(ii);
    SkPixmap pm0, pm1;

    SkAssertResult(result0.peekPixels(&pm0));
    SkAssertResult(result1.peekPixels(&pm1));

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, ii);
    if (!surface) {
        ERRORF(reporter, "Surface creation failed");
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();

    // Set up a recording to clear the surface
    canvas->clear(kBackgroundColor);
    std::unique_ptr<Recording> clearRecording = recorder->snap();
    if (!clearRecording) {
        ERRORF(reporter, "Recording creation failed");
        return false;
    }

    // Draw some text and get recording
    SkPaint paint;
    paint.setAntiAlias(true);

    SkFont font(ToolUtils::create_portable_typeface("serif", SkFontStyle()));
    font.setSubpixel(true);
    font.setSize(12);

    const char* text0 = "Hamburge";
    const size_t text0Len = strlen(text0);

    canvas->drawSimpleText(text0, text0Len, SkTextEncoding::kUTF8, 3, 20, font, paint);
    std::unique_ptr<Recording> text0Recording = recorder->snap();

    // Draw some more text and get recording
    const char* text1 = "burgefons";
    const size_t text1Len = strlen(text1);

    canvas->drawSimpleText(text1, text1Len, SkTextEncoding::kUTF8, 3, 40, font, paint);
    std::unique_ptr<Recording> text1Recording = recorder->snap();

    // Playback 0, then 1, and read pixels
    InsertRecordingInfo info;
    info.fRecording = clearRecording.get();
    context->insertRecording(info);
    info.fRecording = text0Recording.get();
    context->insertRecording(info);
    info.fRecording = text1Recording.get();
    context->insertRecording(info);
    context->submit();

    if (!surface->readPixels(pm0, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return false;
    }

    // Playback 1, then 0, and read pixels
    info.fRecording = clearRecording.get();
    context->insertRecording(info);
    info.fRecording = text1Recording.get();
    context->insertRecording(info);
    info.fRecording = text0Recording.get();
    context->insertRecording(info);
    context->submit();

    if (!surface->readPixels(pm1, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return false;
    }

    // Compare and contrast
    float tol = 1.f/256;
    const float tols[4] = {tol, tol, tol, tol};
    auto error = std::function<ComparePixmapsErrorReporter>([&](int x, int y,
                                                                const float diffs[4]) {
        SkASSERT(x >= 0 && y >= 0);
        ERRORF(reporter,
               "Error at %d, %d. Diff in floats: (%f, %f, %f, %f)",
               x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
    });
    ComparePixels(pm0, pm1, tols, error);

    return true;
}

} // anonymous namespace

// This test captures two recordings A and B, plays them back as A then B, and B then A,
// and verifies that the result is the same.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecordingOrderTest_Graphite, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    (void) run_test(reporter, context, recorder.get());
}
