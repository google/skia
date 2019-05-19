// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f48b22eaad1bb7adcc3faaa321754af6
REG_FIDDLE(Canvas_drawVertices, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };
    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
            SK_ARRAY_COUNT(points), points, nullptr, colors);
    canvas->drawVertices(vertices.get(), SkBlendMode::kSrc, paint);
}
}  // END FIDDLE
