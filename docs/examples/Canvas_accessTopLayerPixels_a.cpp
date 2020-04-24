// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=38d0d6ca9bea146d31bcbec197856359
REG_FIDDLE(Canvas_accessTopLayerPixels_a, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    if (canvas->accessTopLayerPixels(nullptr, nullptr)) {
         canvas->clear(SK_ColorRED);
    } else {
         canvas->clear(SK_ColorBLUE);
    }
}
}  // END FIDDLE
