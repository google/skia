/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkDrawable.h"

struct MyDrawable : public SkDrawable {
    SkRect onGetBounds() override {
        return SkRect::MakeWH(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
       SkPath path;
       path.moveTo(10, 10);
       path.conicTo(10, 90, 50, 90, 0.9f);

       SkPaint paint;
       paint.setColor(SK_ColorBLUE);
       canvas->drawRect(path.getBounds(), paint);

       paint.setAntiAlias(true);
       paint.setColor(SK_ColorWHITE);
       canvas->drawPath(path, paint);
    }
};

DEF_SIMPLE_GM(Drawables, canvas, 640, 480) {
    SkAutoTUnref<SkDrawable> d(new MyDrawable);
    canvas->drawDrawable(d);
}
