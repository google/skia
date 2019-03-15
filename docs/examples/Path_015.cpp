#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=400facce23d417bc5043c5f58404afbd
REG_FIDDLE(Path_015, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setTextSize(80);
    paint.getTextPath("ABC", 3, 20, 80, &path);
    canvas->drawPath(path, paint);
    path.toggleInverseFillType();
    paint.setColor(SK_ColorGREEN);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
