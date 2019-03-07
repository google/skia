// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Bitmap_033, 256, 256, false, 3);
// HASH=0c45da35172bc0a529b2faecddae62a2
void draw(SkCanvas* canvas) {
    SkIRect bounds;
    source.getBounds(&bounds);
    bounds.inset(100, 100);
    SkBitmap bitmap;
    source.extractSubset(&bitmap, bounds);
    canvas->scale(.5f, .5f);
    canvas->drawBitmap(bitmap, 10, 10);
}
}
