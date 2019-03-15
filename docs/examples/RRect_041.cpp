#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=68a5d24f22e2d798608fce8a20e47fd0
REG_FIDDLE(RRect_041, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkVector radii[] = {{5, 5},  {10, 10}, {15, 15}, {5, 5}};
    SkRRect rrect;
    rrect.setRectRadii({10, 10, 110, 80}, radii);
    SkRRect transformed;
    SkMatrix matrix = SkMatrix::MakeRectToRect(rrect.rect(), {140, 30, 220, 80},
                                               SkMatrix::kCenter_ScaleToFit);
    bool success = rrect.transform(matrix, &transformed);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString("rrect", 55, 100, paint);
    canvas->drawString(success ? "transformed" : "transform failed", 185, 100, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, paint);
    canvas->drawRRect(transformed, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
