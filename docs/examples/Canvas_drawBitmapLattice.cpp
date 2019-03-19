// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c5bfa944e17ba4a4400dc799f032069c
REG_FIDDLE(Canvas_drawBitmapLattice, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkIRect center = { 20, 10, 50, 40 };
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));
    SkCanvas bitCanvas(bitmap);
    SkPaint paint;
    SkColor gray = 0xFF000000;
    int left = 0;
    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {
        int top = 0;
        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {
            paint.setColor(gray);
            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);
            gray += 0x001f1f1f;
            top = bottom;
        }
        left = right;
    }
    const int xDivs[] = { center.fLeft, center.fRight };
    const int yDivs[] = { center.fTop, center.fBottom };
    SkCanvas::Lattice::RectType fillTypes[3][3];
    memset(fillTypes, 0, sizeof(fillTypes));
    fillTypes[1][1] = SkCanvas::Lattice::kTransparent;
    SkColor dummy[9];  // temporary pending bug fix
    SkCanvas::Lattice lattice = { xDivs, yDivs, fillTypes[0], SK_ARRAY_COUNT(xDivs),
         SK_ARRAY_COUNT(yDivs), nullptr, dummy };
    for (auto dest: { 20, 30, 40, 60, 90 } ) {
        canvas->drawBitmapLattice(bitmap, lattice, SkRect::MakeWH(dest, 110 - dest), nullptr);
        canvas->translate(dest + 4, 0);
    }
}
}  // END FIDDLE
