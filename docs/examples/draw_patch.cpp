// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_patch, 350, 350, false, 6) {
// draw_patch
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint p;
    p.setAntiAlias(true);
    const SkColor colors[] = {SK_ColorRED,     SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
                              SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW};
    const SkPoint pts[] = {{100.f / 4.f, 0.f}, {3.f * 100.f / 4.f, 100.f}};
    p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                             SkTileMode::kMirror));
    const SkPoint cubics[] = {{100, 100}, {150, 50},  {250, 150}, {300, 100},
                              {250, 150}, {350, 250}, {300, 300}, {250, 250},
                              {150, 350}, {100, 300}, {50, 250},  {150, 150}};
    const SkPoint texCoords[] = {
            {0.0f, 0.0f}, {100.0f, 0.0f}, {100.0f, 100.0f}, {0.0f, 100.0f}};
    canvas->drawPatch(cubics, nullptr, texCoords, SkBlendMode::kSrcOver, p);
}
}  // END FIDDLE
