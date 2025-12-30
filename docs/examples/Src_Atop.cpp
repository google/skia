// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Src_Atop, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colors[] = { SkColors::kRed, SkColors::kBlue };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(horz, {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkColor4f alphas[] = { SkColors::kBlack, SkColors::kTransparent };
    SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
    paint.setShader(SkShaders::LinearGradient(vert, {{alphas, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
    canvas->clipRect( { 30, 30, 226, 226 } );
    canvas->drawColor(SkColorSetA(SK_ColorGREEN, 128), SkBlendMode::kSrcATop);
}
}  // END FIDDLE
