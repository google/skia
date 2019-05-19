// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1bdc07ad3b154c89b771722c2fcaee3f
REG_FIDDLE(Canvas_drawDrawable_2, 256, 100, false, 0) {
struct MyDrawable : public SkDrawable {
    SkRect onGetBounds() override { return SkRect::MakeWH(50, 100);  }
    void onDraw(SkCanvas* canvas) override {
       SkPath path;
       path.conicTo(10, 90, 50, 90, 0.9f);
       SkPaint paint;
       paint.setColor(SK_ColorBLUE);
       canvas->drawRect(path.getBounds(), paint);
       paint.setAntiAlias(true);
       paint.setColor(SK_ColorWHITE);
       canvas->drawPath(path, paint);
    }
};

void draw(SkCanvas* canvas) {
    sk_sp<SkDrawable> drawable(new MyDrawable);
  canvas->drawDrawable(drawable.get(), 10, 10);
}
}  // END FIDDLE
