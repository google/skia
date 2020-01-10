// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_path_1d_path_effect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath path;
    path.addOval(SkRect::MakeWH(16.0f, 6.0f));
    paint.setPathEffect(
            SkPath1DPathEffect::Make(path, 32.0f, 0.0f, SkPath1DPathEffect::kRotate_Style));
    paint.setAntiAlias(true);
    canvas->clear(SK_ColorWHITE);
    canvas->drawCircle(128.0f, 128.0f, 122.0f, paint);
}
}  // END FIDDLE
