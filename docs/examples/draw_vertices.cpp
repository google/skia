// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_vertices, 256, 256, false, 6) {
// draw_vertices
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);

    SkPoint pts[3] = {{64, 32}, {0, 224}, {128, 224}};
    SkColor colors[3] = {SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN};
    canvas->drawVertices(
            SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 3, pts, nullptr, colors),
            SkBlendMode::kSrcOver, p);

    canvas->translate(120, 0);

    p.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions()));
    SkPoint texs[3] = {{0, 0}, {0, 128}, {64, 256}};
    canvas->drawVertices(
            SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 3, pts, texs, nullptr),
            SkBlendMode::kSrcOver, p);
}
}  // END FIDDLE
