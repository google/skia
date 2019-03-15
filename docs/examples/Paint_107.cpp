#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=59d9b8249afa1c2af6186711250ce240
REG_FIDDLE(Paint_107, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(32);
    SkScalar lineHeight = paint.getFontMetrics(nullptr);
    canvas->drawString("line 1", 10, 40, paint);
    canvas->drawString("line 2", 10, 40 + lineHeight, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
