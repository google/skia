/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/Test.h"

// Draws a dashed circle with circumference 100, with an on-interval of 90 and an off-interval of
// 10, offset into the intervals by 25. This should draw a dash clockwise from 3:00 ending around
// 11:00, then the start of a second dash from 12:00 back to 3:00. In https://crbug.com/1495670, the
// math for intervals ending very near 2*pi introduced floating point error that prevented drawing
// the second dash.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DashPathEffectTest_2PiInterval,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    const float r = 50.0f / SK_ScalarPI;
    const float centerX = ceilf(0.5f * r);
    const float centerY = ceilf(r);
    const float dashWidth = 10.0f;

    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(16, 16),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    GrDirectContext* context = contextInfo.directContext();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, ii);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(dashWidth);

    constexpr float intervals[2] = {90.0f, 10.0f};
    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 25.0f));
    canvas->drawCircle(centerX, centerY, r, paint);

    // Check that we drew the second dash, which starts at the top of the circle.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }
    SkColor topColor = pixmap.getColor(centerX + 1.0f, centerY - r);
    REPORTER_ASSERT(reporter, topColor == SK_ColorRED);
}
