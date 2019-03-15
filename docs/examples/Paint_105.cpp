#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0d21e968e9a4c78c902ae3ef494941a0
REG_FIDDLE(Paint_105, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("kUTF8_SkTextEncoding %c= text encoding\n",
            kUTF8_SkTextEncoding == paint.getTextEncoding() ? '=' : '!');
    paint.setTextEncoding(kGlyphID_SkTextEncoding);
    SkDebugf("kGlyphID_SkTextEncoding %c= text encoding\n",
            kGlyphID_SkTextEncoding == paint.getTextEncoding() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
