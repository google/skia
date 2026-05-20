/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(crbug_513836996, reporter, context, CtsEnforcement::kNever) {
    if (context->supportsProtectedContent()) {
        // Require readback, so exit if not possible
        return;
    }

    constexpr int kW = 200;
    constexpr int kH = 100;
    const SkImageInfo ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);

    auto src_ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap src_bm;
    src_bm.allocPixels(src_ii);
    src_bm.eraseColor(SK_ColorGREEN);

    sk_sp<SkImage> rasterImage = SkImages::RasterFromBitmap(src_bm);
    sk_sp<SkImage> green = SkImages::TextureFromImage(recorder.get(), rasterImage);

    const SkPoint dstClips[8] = {
        {10, 10}, {60, 10}, {60, 60}, {10, 60},     // LEFT
        {110, 10}, {190, 10}, {190, 90}, {110, 90}, // RIGHT
    };

    SkCanvas::ImageSetEntry entries[2];
    entries[0].fImage = green;
    entries[0].fSrcRect = SkRect::MakeLTRB(-100, -100, -50, -50);
    entries[0].fDstRect = SkRect::MakeWH(kW, kH);
    entries[0].fMatrixIndex = -1;
    entries[0].fAlpha = 1.f;
    entries[0].fAAFlags = SkCanvas::kNone_QuadAAFlags;
    entries[0].fHasClip = true;

    entries[1].fImage = green;
    entries[1].fSrcRect = SkRect::MakeWH(32, 32);
    entries[1].fDstRect = SkRect::MakeWH(kW, kH);
    entries[1].fMatrixIndex = -1;
    entries[1].fAlpha = 1.f;
    entries[1].fAAFlags = SkCanvas::kNone_QuadAAFlags;
    entries[1].fHasClip = true;

    canvas->experimental_DrawEdgeAAImageSet(
        entries, 2, dstClips, nullptr, SkSamplingOptions(),
        nullptr, SkCanvas::kFast_SrcRectConstraint);

    std::unique_ptr<Recording> recording = recorder->snap();
    context->insertRecording({recording.get()});

    SkBitmap bm;
    bm.allocPixels(ii);
    REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0), "Failed to read pixels");

    SkColor left_px  = bm.getColor(35, 35);
    SkColor right_px = bm.getColor(150, 50);

    REPORTER_ASSERT(reporter, left_px == SK_ColorBLACK,
        "Left pixel should be BLACK, got 0x%08x", left_px);
    REPORTER_ASSERT(reporter, right_px == SK_ColorGREEN,
        "Right pixel should be GREEN, got 0x%08x", right_px);
}
