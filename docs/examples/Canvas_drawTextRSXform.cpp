// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=935c8f8b9782d297a73d7186f6ef7945
REG_FIDDLE(Canvas_drawTextRSXform, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const int iterations = 26;
    SkRSXform transforms[iterations];
    char alphabet[iterations];
    SkScalar angle = 0;
    SkScalar scale = 1;
    for (size_t i = 0; i < std::size(transforms); ++i) {
        const SkScalar s = SkScalarSin(angle) * scale;
        const SkScalar c = SkScalarCos(angle) * scale;
        transforms[i] = SkRSXform::Make(-c, -s, -s * 16, c * 16);
        angle += .45f;
        scale += .2f;
        alphabet[i] = 'A' + i;
    }
    SkPaint paint;
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 20);
    auto spiral = SkTextBlob::MakeFromRSXform(alphabet, sizeof(alphabet), transforms, font);
    canvas->drawTextBlob(spiral, 110, 138, paint);
}
}  // END FIDDLE
