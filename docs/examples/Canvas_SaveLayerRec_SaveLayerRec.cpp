#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b5cea1eed80a0eb04ddbab3f36dff73f
REG_FIDDLE(Canvas_SaveLayerRec_SaveLayerRec, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas::SaveLayerRec rec1;
    rec1.fSaveLayerFlags = SkCanvas::kPreserveLCDText_SaveLayerFlag;
    SkCanvas::SaveLayerRec rec2(nullptr, nullptr, SkCanvas::kPreserveLCDText_SaveLayerFlag);
    SkDebugf("rec1 %c= rec2\n", rec1.fBounds == rec2.fBounds
            && rec1.fPaint == rec2.fPaint
            && rec1.fBackdrop == rec2.fBackdrop
            && rec1.fSaveLayerFlags == rec2.fSaveLayerFlags ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
