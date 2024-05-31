// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Bitmap_readyToDraw, 256, 160, false, 5) {
void draw(SkCanvas* canvas) {
    if (source.readyToDraw()) {
        canvas->drawImage(source.asImage(), 10, 10);
    }
}
}  // END FIDDLE
