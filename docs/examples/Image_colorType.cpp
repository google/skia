// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=50396fad4a128f58e400ca00fe09711f
REG_FIDDLE(Image_colorType, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    const char* colors[] = { "Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                             "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                             "RGBA_F16" };
    SkColorType colorType = image->colorType();
    canvas->drawImage(image, 16, 0);
    canvas->drawString(colors[(int) colorType], 20, image->height() + 20, SkFont(), SkPaint());
}
}  // END FIDDLE
