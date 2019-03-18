// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=eb93d5fa66a5f7a10f4f9210494d7222
REG_FIDDLE(Canvas_translate, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint filledPaint;
    SkPaint outlinePaint;
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setColor(SK_ColorBLUE);
    canvas->save();
    canvas->translate(50, 50);
    canvas->drawCircle(28, 28, 15, outlinePaint);  // blue center: (50+28, 50+28)
    canvas->scale(2, 1/2.f);
    canvas->drawCircle(28, 28, 15, filledPaint);   // black center: (50+(28*2), 50+(28/2))
    canvas->restore();
    filledPaint.setColor(SK_ColorGRAY);
    outlinePaint.setColor(SK_ColorRED);
    canvas->scale(2, 1/2.f);
    canvas->drawCircle(28, 28, 15, outlinePaint);  // red center: (28*2, 28/2)
    canvas->translate(50, 50);
    canvas->drawCircle(28, 28, 15, filledPaint);   // gray center: ((50+28)*2, (50+28)/2)
}
}  // END FIDDLE
