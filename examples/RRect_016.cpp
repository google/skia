// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(RRect_016, 256, 90, false, 0);
// HASH=3afc3ac9bebd1d7387822cc608571e82
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    rrect.setRect({60, 30, 120, 80});
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(rrect, paint);
}
}
