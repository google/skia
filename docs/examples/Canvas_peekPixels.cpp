// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e9411d676d1fa13b46331abe9e14ad3e
REG_FIDDLE(Canvas_peekPixels, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap;
    if (canvas->peekPixels(&pixmap)) {
        SkDebugf("width=%d height=%d\n", pixmap.bounds().width(), pixmap.bounds().height());
    }
}
}  // END FIDDLE
