// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3cf8fb639fef99993cafc064d550c739
REG_FIDDLE(Canvas_drawImageRect_3, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    // sk_sp<SkImage> image;
    for (auto i : { 20, 40, 80, 160 } ) {
        canvas->drawImageRect(image.get(), SkRect::MakeXYWH(i, i, i, i), nullptr);
    }
}
}  // END FIDDLE
