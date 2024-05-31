// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_getContext, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    if (auto context = canvas->recordingContext()) {
         if (context->asDirectContext()) {
             canvas->clear(SK_ColorRED);
         } else {
             canvas->clear(SK_ColorGREEN);
         }
    } else {
         canvas->clear(SK_ColorBLUE);
    }
}
}  // END FIDDLE
