#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0d21e968e9a4c78c902ae3ef494941a0
REG_FIDDLE(Paint_getTextEncoding, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("SkTextEncoding::kUTF8 %c= text encoding\n",
            SkTextEncoding::kUTF8 == paint.getTextEncoding() ? '=' : '!');
    paint.setTextEncoding(SkTextEncoding::kGlyphID);
    SkDebugf("SkTextEncoding::kGlyphID %c= text encoding\n",
            SkTextEncoding::kGlyphID == paint.getTextEncoding() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
