// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=027f920259888fc19591ea9a90d92873
REG_FIDDLE(Canvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas::SaveLayerRec rec1;
    SkCanvas::SaveLayerRec rec2(nullptr, nullptr);
    SkDebugf("rec1 %c= rec2\n", rec1.fBounds == rec2.fBounds
            && rec1.fPaint == rec2.fPaint
            && rec1.fBackdrop == rec2.fBackdrop
            && rec1.fSaveLayerFlags == rec2.fSaveLayerFlags ? '=' : '!');
}
}  // END FIDDLE
