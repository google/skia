// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=82f1a9b4c2b27aa547061786d1f33dab
REG_FIDDLE(Color4f_001, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colorGray = { .5, .5, .5, 1 };
    SkColor4f colorNamedGray = SkColor4f::FromColor(SK_ColorGRAY);
    SkDebugf("colorGray %c= colorNamedGray ", colorGray != colorNamedGray ? '!' : '=');
}
}  // END FIDDLE
