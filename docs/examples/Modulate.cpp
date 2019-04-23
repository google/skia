// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3fdac2b2f48bd227d2e74234c260bc8e
REG_FIDDLE(Modulate, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    auto drawSquare = [=](int dx, int dy, SkBlendMode mode, const char* label) -> void {
        const SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
        const SkPoint horz[] = { { 0, 0 }, { 128, 0 } };
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, SK_ARRAY_COUNT(colors),
                SkTileMode::kClamp));
        paint.setBlendMode(mode);
        canvas->translate(dx, dy);
        canvas->drawRect({0, 0, 128, 128}, paint);
        paint.setBlendMode(SkBlendMode::kXor);
        SkFont font;
        canvas->drawString(label, 40, 100, font, paint);
    };
    drawSquare(0, 0, SkBlendMode::kSrc, "destination");
    drawSquare(128, 0, SkBlendMode::kSrc, "");
    drawSquare(0, 128, SkBlendMode::kSrc, "");
    canvas->translate(-128, -128);
    canvas->rotate(90, 0, 128);
    drawSquare(0, 0, SkBlendMode::kSrc, "source");
    drawSquare(0, -128, SkBlendMode::kModulate, "modulate");
    drawSquare(-128, 0, SkBlendMode::kMultiply, "multiply");
}
}  // END FIDDLE
