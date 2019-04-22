// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bb1dbfdca3aedf716beb6f07e2aab065
REG_FIDDLE(State_Stack_a, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    canvas->save();                             // records stack depth to restore
    canvas->clipRect(SkRect::MakeWH(100, 100)); // constrains drawing to clip
    canvas->clear(SK_ColorRED);                 // draws to limit of clip
    canvas->save();                             // records stack depth to restore
    canvas->clipRect(SkRect::MakeWH(50, 150));  // Rect below 100 is ignored
    canvas->clear(SK_ColorBLUE);                // draws to smaller clip
    canvas->restore();                          // enlarges clip
    canvas->drawLine(20, 20, 150, 150, paint);  // line below 100 is not drawn
    canvas->restore();                          // enlarges clip
    canvas->drawLine(150, 20, 50, 120, paint);  // line below 100 is drawn
}
}  // END FIDDLE
