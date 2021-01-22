// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(backdrop_blur_with_rrect_clip, 512, 512, false, 3) {
void draw(SkCanvas* canvas) {
    const SkRect r = SkRect::MakeXYWH(128, 128, 256, 256);
    const SkRRect rr = SkRRect::MakeRectXY(r, 128, 128);

    canvas->drawImage(image, 0, 0);

    canvas->save();
    canvas->clipRRect(rr, true);

    sk_sp<SkImageFilter> filter = SkImageFilters::Blur(10, 10, nullptr);
    SkPaint p;
    p.setImageFilter(std::move(filter));

    SkCanvas::SaveLayerRec slr(&r, &p, SkCanvas::kInitWithPrevious_SaveLayerFlag);
    canvas->saveLayer(slr);
    canvas->drawColor(0x40FFFFFF);
    canvas->restore();
    canvas->restore();
}
}  // END FIDDLE
