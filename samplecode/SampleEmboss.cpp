/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"

static void draw_emboss(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    SkEmbossMaskFilter::Light light = {{1, 1, 1}, 0, 128, 16*2};
    paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(4), light));
    paint.setShader(SkShaders::Color(SK_ColorBLUE));
    paint.setDither(true);
    canvas->drawCircle(50, 50, 30, paint);
}

struct EmbossView : public Sample {
    EmbossView() : Sample("Emboss") {}
    void onDrawContent(SkCanvas* canvas) override { draw_emboss(canvas); }
};
DEF_SAMPLE( return new EmbossView(); )
