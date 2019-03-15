// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=50396fad4a128f58e400ca00fe09711f
REG_FIDDLE(Image_colorType, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    SkColorType colorType = image->colorType();
    canvas->drawImage(image, 16, 0);
    canvas->drawString(SkColorTypeToString(colorType), 20, image->height() + 20, SkFont(), SkPaint());
}
}  // END FIDDLE
