// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(drawarcs, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(8);

    SkPath path;
    SkRandom rand;

    for (int i = 0; i < 100; ++i) {
        SkScalar x = rand.nextUScalar1() * 200;
        SkScalar y = rand.nextUScalar1() * 200;

        path.rewind();
        path.addArc(SkRect::MakeXYWH(x, y, 70, 70), rand.nextUScalar1() * 360,
                    rand.nextUScalar1() * 360);
        paint.setColor(rand.nextU() | 0xFF000000);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
