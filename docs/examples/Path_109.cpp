#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c0216b3f7ebd80b9589ae5728f08fc80
REG_FIDDLE(Path_109, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkPaint paint;
    paint.setTextSize(256);
    paint.getTextPath("&", 1, 30, 220, &path);
    for (int y = 2; y < 256; y += 9) {
       for (int x = 2; x < 256; x += 9) {
           int coverage = 0;
           for (int iy = -4; iy <= 4; iy += 2) {
               for (int ix = -4; ix <= 4; ix += 2) {
                   coverage += path.contains(x + ix, y + iy);
               }
           }
           paint.setColor(SkColorSetARGB(0x5f, 0xff * coverage / 25, 0, 0xff * (25 - coverage) / 25));
           canvas->drawCircle(x, y, 8, paint);
       }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
