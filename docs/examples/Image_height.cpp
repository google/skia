#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a4f53a0b6ac85e7bc3887245b728530d
REG_FIDDLE(Image_height, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->translate(10, 10);
    canvas->drawImage(image, 0, 0);
    canvas->translate(image->width(), 0);
    SkPaint paint;
    canvas->drawLine(10, 0, 10, image->height(), paint);
    canvas->drawString("height", 34, image->height() / 2, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
