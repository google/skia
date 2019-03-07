// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=82621c4df8da1e589d9e627494067826
REG_FIDDLE(Path_054, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    SkPath path;
    SkPoint pts[] = {{128, 10}, {10, 214}, {236, 214}};
    path.moveTo(pts[1]);
    for (int i = 0; i < 3; ++i) {
        path.quadTo(pts[i % 3],  pts[(i + 2) % 3]);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
