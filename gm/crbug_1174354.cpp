/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

static void draw_bg_blur(SkCanvas* canvas, SkIRect rect, float sigma) {
    // First create an intermediate layer that has an opaque area that we blur with transparency
    // all around it. We want to make sure the transparency doesn't affect the blur of the opaque
    // content.
    auto outsetRect = SkRect::Make(rect).makeOutset(10, 10);
    canvas->saveLayer(outsetRect, nullptr);
    SkColor colors[] = {SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN};
    float cx = (rect.left() + rect.right() )/2.f;
    float cy = (rect.top()  + rect.bottom())/2.f;
    auto g = SkGradientShader::MakeSweep(cx,
                                         cy,
                                         colors,
                                         nullptr,
                                         3,
                                         SkTileMode::kMirror,
                                         0,
                                         45,
                                         0,
                                         nullptr);
    SkPaint paint;
    paint.setShader(std::move(g));
    canvas->drawRect(SkRect::Make(rect), paint);
    // Now do the bg-blur into another save-layer that should only read the opaque region.
    SkCanvas::SaveLayerRec rec;
    auto blur = SkImageFilters::Blur(sigma, sigma, SkTileMode::kClamp, nullptr, rect);
    rec.fBounds = &outsetRect;
    rec.fBackdrop = blur.get();
    canvas->saveLayer(rec);
    canvas->restore();
    canvas->restore();
}

DEF_SIMPLE_GM(crbug_1174354, canvas, 70, 250) {
    // The initial fix for crbug.com/1156805 had an issue where the border added to the downsampled
    // texture that was used as input to the blur could actually bring in transparency when there
    // wasn't any within the original src bounds. It was populated the border using a filtering draw
    // from the full res source that could read from outside the pixels surrounding the original src
    // bounds.
    draw_bg_blur(canvas, SkIRect::MakeXYWH(10,  10,  50, 50),  5);
    draw_bg_blur(canvas, SkIRect::MakeXYWH(10,  70,  50, 50), 15);
    draw_bg_blur(canvas, SkIRect::MakeXYWH(10, 130,  50, 50), 30);
    draw_bg_blur(canvas, SkIRect::MakeXYWH(10, 190, 50, 50),  70);
}
