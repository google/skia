// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=de83fbd848a4625345b4b87a6e55d98a
REG_FIDDLE(Paint_setStrokeCap_a, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeCap((SkPaint::Cap) SkPaint::kCapCount);
    SkDebugf("kRound_Cap %c= paint.getStrokeCap()\n",
            SkPaint::kRound_Cap == paint.getStrokeCap() ? '=' : '!');
}
}  // END FIDDLE
