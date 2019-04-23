#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1b9f1f05026ceb14ccb6926a13cdaa83
REG_FIDDLE(Image_alphaType, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    const char* alphaTypeStr[] = { "Unknown", "Opaque", "Premul", "Unpremul" };
    SkAlphaType alphaType = image->alphaType();
    canvas->drawImage(image, 16, 0);
    canvas->drawString(alphaTypeStr[(int) alphaType], 20, image->height() + 20, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
