/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPath.h"

struct MyDrawable : public SkDrawable {
    SkRect onGetBounds() override { return SkRect::MakeWH(50, 100);  }

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

/*
 *  Test calling drawables w/ translate and matrices
 */
DEF_SIMPLE_GM(drawable, canvas, 180, 275) {
    sk_sp<SkDrawable> drawable(new MyDrawable);

    canvas->translate(10, 10);
    canvas->drawDrawable(drawable.get());
    canvas->drawDrawable(drawable.get(), 0, 150);

    SkMatrix m = SkMatrix::MakeScale(1.5f, 0.8f);
    m.postTranslate(70, 0);
    canvas->drawDrawable(drawable.get(), &m);

    m.postTranslate(0, 150);
    canvas->drawDrawable(drawable.get(), &m);
}
