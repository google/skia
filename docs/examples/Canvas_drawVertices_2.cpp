// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_drawVertices_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };
    SkPoint texs[] = { { 0, 0 }, { 0, 250 }, { 250, 250 }, { 250, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };
    SkColor4f c4[4];
    std::transform(colors, colors + 4, c4, [](SkColor c) {
        return SkColor4f::FromColor(c);
    });
    paint.setShader(SkShaders::LinearGradient(points, {{c4, {}, SkTileMode::kClamp}, {}}));
    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
            std::size(points), points, texs, colors);
    canvas->drawVertices(vertices, SkBlendMode::kDarken, paint);
}
}  // END FIDDLE
