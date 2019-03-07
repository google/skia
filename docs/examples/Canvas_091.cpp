// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Canvas_091, 256, 256, false, 4) {
// HASH=7f92cd5c9b9f4b1ac3cd933b08037bfe
void draw(SkCanvas* canvas) {
    // sk_sp<SkImage> image;
    for (auto i : { 1, 2, 4, 8 } ) {
        canvas->drawImageRect(image.get(), SkIRect::MakeLTRB(0, 0, 100, 100),
                SkRect::MakeXYWH(i * 20, i * 20, i * 20, i * 20), nullptr);
    }
}

}
