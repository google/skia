// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_AddPathMode, 256, 180, false, 0) {
void draw(SkCanvas* canvas) {
    SkPathBuilder path;
    path.moveTo(20, 20)
        .lineTo(20, 40)
        .lineTo(40, 20);
    SkPath path2 = SkPathBuilder()
                   .moveTo(60, 60)
                   .lineTo(80, 60)
                   .lineTo(80, 40)
                   .detach();
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 2; i++) {
        for (auto addPathMode : { SkPath::kAppend_AddPathMode, SkPath::kExtend_AddPathMode } ) {
            SkPath test = SkPathBuilder(path)
                          .addPath(path2, addPathMode)
                          .detach();
            canvas->drawPath(test, paint);
            canvas->translate(100, 0);
        }
        canvas->translate(-200, 100);
        path.close();
    }
}
}  // END FIDDLE
