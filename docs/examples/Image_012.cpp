#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9aec65fc252ffc9982fa8867433eca18
REG_FIDDLE(Image_012, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->translate(10, 10);
    canvas->drawImage(image, 0, 0);
    canvas->translate(0, image->height());
    SkPaint paint;
    canvas->drawLine(0, 10, image->width(), 10, paint);
    canvas->drawString("width", image->width() / 2 - 15, 25, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
