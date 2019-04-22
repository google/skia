#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=91c9a3e498bb9412e4522a95d076ed5f
REG_FIDDLE(Text_Size, 256, 135, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    canvas->drawString("12 point", 10, 20, paint);
    paint.setTextSize(24);
    canvas->drawString("24 point", 10, 60, paint);
    paint.setTextSize(48);
    canvas->drawString("48 point", 10, 120, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
