// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_drawOval, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(0xFF3f5f9f);
    SkColor4f  kColor1 = SkColor4f::FromColor(SkColorSetARGB(0xff, 0xff, 0x7f, 0));
    SkColor4f  g1Colors[] = { kColor1, kColor1.withAlpha(0x20/255.f) };
    SkPoint  g1Points[] = { { 0, 0 }, { 0, 100 } };
    SkScalar pos[] = { 0.2f, 1.0f };
    SkRect bounds = SkRect::MakeWH(80, 70);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkShaders::LinearGradient(g1Points, {{g1Colors, pos, SkTileMode::kClamp}, {}}));
    canvas->drawOval(bounds , paint);
}
}  // END FIDDLE
