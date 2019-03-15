#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=50969745cf2b23544362f4cff5592b75
REG_FIDDLE(RRect_040, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkVector radii[] = {{5, 5},  {10, 10}, {15, 15}, {5, 5}};
    SkRRect rrect;
    rrect.setRectRadii({10, 10, 110, 80}, radii);
    char storage[SkRRect::kSizeInMemory];
    rrect.writeToMemory(storage);
    SkRRect copy;
    copy.readFromMemory(storage, sizeof(storage));
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString("rrect", 55, 100, paint);
    canvas->drawString("copy", 185, 100, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, paint);
    canvas->translate(120, 0);
    canvas->drawRRect(copy, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
