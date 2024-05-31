// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Src, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->clipRect({50, 50, 200, 200});
    SkPaint srcBlend;
    srcBlend.setBlendMode(SkBlendMode::kSrc);
    canvas->saveLayer(nullptr, &srcBlend);
    canvas->drawColor(0);
    SkPaint transRed;
    transRed.setColor(SkColorSetA(SK_ColorRED, 127));
    canvas->drawCircle(125, 125, 75, transRed);
    canvas->restore();
}
}  // END FIDDLE
