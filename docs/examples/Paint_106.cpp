#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a5d1ba0dbf42afb797ffdb07647b5cb9
REG_FIDDLE(Paint_106, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextEncoding((SkTextEncoding) 4);
    SkDebugf("4 %c= text encoding\n", (SkTextEncoding) 4 == paint.getTextEncoding() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
