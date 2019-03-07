// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Bitmap_057, 256, 160, false, 5);
// HASH=e89c78ca992e2e789ed50944fe68f920
void draw(SkCanvas* canvas) {
    if (source.readyToDraw()) {
        canvas->drawBitmap(source, 10, 10);
    }
}
}
