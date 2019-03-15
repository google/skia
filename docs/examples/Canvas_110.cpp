#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=935c8f8b9782d297a73d7186f6ef7945
REG_FIDDLE(Canvas_110, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const int iterations = 26;
    SkRSXform transforms[iterations];
    char alphabet[iterations];
    SkScalar angle = 0;
    SkScalar scale = 1;
    for (size_t i = 0; i < SK_ARRAY_COUNT(transforms); ++i) {
        const SkScalar s = SkScalarSin(angle) * scale;
        const SkScalar c = SkScalarCos(angle) * scale;
        transforms[i] = SkRSXform::Make(-c, -s, -s * 16, c * 16);
        angle += .45;
        scale += .2;
        alphabet[i] = 'A' + i;
    }
    SkPaint paint;
    canvas->translate(110, 138);
    canvas->drawTextRSXform(alphabet, sizeof(alphabet), transforms, nullptr, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
