#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e311cdd451edacec33b50cc22a4dd5dc
REG_FIDDLE(Path_048, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(72);
    canvas->drawString("#", 120, 80, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    SkPath path;
    SkPoint hash[] = {{58, 28}, {43, 80}, {37, 45}, {85, 45}};
    SkVector offsets[] = {{0, 0}, {17, 0}, {0, 0}, {-5, 17}};
    unsigned o = 0;
    for (unsigned i = 0; i < SK_ARRAY_COUNT(hash); i += 2) {
        for (unsigned j = 0; j < 2; o++, j++) {
            path.moveTo(hash[i].fX + offsets[o].fX, hash[i].fY + offsets[o].fY);
            path.lineTo(hash[i + 1].fX + offsets[o].fX, hash[i + 1].fY + offsets[o].fY);
        }
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
