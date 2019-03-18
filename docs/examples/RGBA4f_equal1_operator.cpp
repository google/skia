// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e5b34bcb7f80f2ed890cdacaa059db0d
REG_FIDDLE(RGBA4f_equal1_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colorRed = { 1, 0, 0, 1 };
    SkColor4f colorNamedRed = SkColor4f::FromColor(SK_ColorRED);
    SkDebugf("colorRed %c= colorNamedRed", colorRed == colorNamedRed ? '=' : '!');
}
}  // END FIDDLE
