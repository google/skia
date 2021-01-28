// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(blur_alpha_img, 256, 256, false, 0) {
sk_sp<SkImage> foo() {
    int w = 240, h = 240;
    SkScalar scale = 10.0f;
    SkPath path;
    static const int8_t pts[] = {2, 2, 1, 3, 0, 3, 2, 1, 3, 1, 4, 0, 4, 1,
                                 5, 1, 4, 2, 4, 3, 2, 5, 2, 4, 3, 3, 2, 3};
    path.moveTo(2 * scale, 3 * scale);
    for (size_t i = 0; i < sizeof(pts) / sizeof(pts[0]); i += 2) {
        path.lineTo(pts[i] * scale, pts[i + 1] * scale);
    }
    path.close();
    SkMatrix matrix = SkMatrix::Scale(4 * scale, 4 * scale);
    SkPaint paint;
    paint.setPathEffect(SkPath2DPathEffect::Make(matrix, path));
    paint.setAntiAlias(true);
    SkRect bounds{-4 * scale, -4 * scale, (float)w, (float)h};
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeA8(w, h));
    SkCanvas c(bm);
    c.clear(0);
    c.drawRect(bounds, paint);
    return bm.asImage();
}

void draw(SkCanvas* canvas) {
    auto a8img = foo();
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 2.0f, 0));
    paint.setColor(0xFF008000);
    canvas->drawImage(a8img, 8, 8, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
