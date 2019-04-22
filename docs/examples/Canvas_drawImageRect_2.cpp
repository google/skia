// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=7f92cd5c9b9f4b1ac3cd933b08037bfe
REG_FIDDLE(Canvas_drawImageRect_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    // sk_sp<SkImage> image;
    for (auto i : { 1, 2, 4, 8 } ) {
        canvas->drawImageRect(image.get(), SkIRect::MakeLTRB(0, 0, 100, 100),
                SkRect::MakeXYWH(i * 20, i * 20, i * 20, i * 20), nullptr);
    }
}
}  // END FIDDLE
