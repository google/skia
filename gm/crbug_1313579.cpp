/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"

// SkiaRenderer can wind up specifying near-integer scale-and-translate matrices on SkCanvas before
// applying a backdrop blur image filter via saveLayer() with an integer clip, crop rect, and
// SaveLayerRec bounds. Round-out is used to determine the bounds of the input image needed in IFs.
// This could cause an extra row/column of pixels to be included in the blur. When that row/column
// is significantly different in color than the intended blur content and the radius is large then
// clamp mode blur creates a very noticeable color bleed artifact.
DEF_SIMPLE_GM(crbug_1313579, canvas, 110, 110) {
    static constexpr auto kBGRect = SkIRect{0, 0, 100, 100};

   sk_sp<SkImageFilter> backdrop_filter =
            SkImageFilters::Blur(50.f, 50.f, SkTileMode::kClamp, nullptr, kBGRect);

    SkMatrix m;

    canvas->clear(SK_ColorGREEN);

    m.setAll(0.999999f, 0,         4.99999f,
             0,         0.999999f, 4.99999f,
             0,         0,         1);
    canvas->concat(m);
    canvas->clipIRect(kBGRect);
    canvas->clear(SK_ColorWHITE);
    canvas->saveLayer(SkCanvas::SaveLayerRec(nullptr, nullptr, backdrop_filter.get(), 0));
    canvas->restore();
}
