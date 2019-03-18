// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e8bdae9bea3227758989028424fcac3d
REG_FIDDLE(Canvas_drawVertices_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };
    SkPoint texs[] = { { 0, 0 }, { 0, 250 }, { 250, 250 }, { 250, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };
    paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 4,
            SkShader::kClamp_TileMode));
    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
            SK_ARRAY_COUNT(points), points, texs, colors);
    canvas->drawVertices(vertices, SkBlendMode::kDarken, paint);
}
}  // END FIDDLE
