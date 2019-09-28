// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5e8513f073db09acde3ff616f6426e3d
REG_FIDDLE(Path_reverseAddPath, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(20, 40);
    path.lineTo(40, 20);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 2; i++) {
        SkPath path2;
        path2.moveTo(60, 60);
        path2.lineTo(80, 60);
        path2.lineTo(80, 40);
        for (int j = 0; j < 2; j++) {
            SkPath test(path);
            test.reverseAddPath(path2);
            canvas->drawPath(test, paint);
            canvas->translate(100, 0);
            path2.close();
        }
        canvas->translate(-200, 100);
        path.close();
    }
}
}  // END FIDDLE
