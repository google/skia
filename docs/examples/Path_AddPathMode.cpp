// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=801b02e74c64aafdb734f2e5cf3e5ab0
REG_FIDDLE(Path_AddPathMode, 256, 180, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, path2;
    path.moveTo(20, 20);
    path.lineTo(20, 40);
    path.lineTo(40, 20);
    path2.moveTo(60, 60);
    path2.lineTo(80, 60);
    path2.lineTo(80, 40);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 2; i++) {
        for (auto addPathMode : { SkPath::kAppend_AddPathMode, SkPath::kExtend_AddPathMode } ) {
            SkPath test(path);
            test.addPath(path2, addPathMode);
            canvas->drawPath(test, paint);
            canvas->translate(100, 0);
        }
        canvas->translate(-200, 100);
        path.close();
    }
}
}  // END FIDDLE
