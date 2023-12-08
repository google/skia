// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(UnpremulBugs, 400, 400, false, 6) {
void fill(SkBitmap & b, SkColor c, int n) {
    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            *(b.getAddr32(x, y)) = c;
        }
    }
}

void doCell(SkCanvas* canvas, float x, float y, SkAlphaType at, SkColor c, int n) {
    SkImageInfo info = SkImageInfo::MakeS32(n, n, at);
    SkBitmap bmp;
    bmp.allocPixels(info);
    fill(bmp, c, n);
    sk_sp<SkImage> img = bmp.asImage();

    SkPaint paint;
    const SkTileMode tile = SkTileMode::kRepeat;

    paint.setShader(img->makeShader(tile, tile, SkSamplingOptions()));
    canvas->drawRect(SkRect::MakeXYWH(x, y, 50, 50), paint);
}

void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    doCell(canvas, 0, 0, kPremul_SkAlphaType, 0x80404040, 2);
    doCell(canvas, 0, 50, kUnpremul_SkAlphaType, 0x80808080, 2);
    doCell(canvas, 50, 0, kPremul_SkAlphaType, 0x80808080, 2);
    doCell(canvas, 50, 50, kUnpremul_SkAlphaType, 0x80FFFFFF, 2);

    doCell(canvas, 110, 0, kPremul_SkAlphaType, 0x80404040, 1);
    doCell(canvas, 110, 50, kUnpremul_SkAlphaType, 0x80808080, 1);
    doCell(canvas, 160, 0, kPremul_SkAlphaType, 0x80808080, 1);
    doCell(canvas, 160, 50, kUnpremul_SkAlphaType, 0x80FFFFFF, 1);

    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 12);
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    canvas->drawString("Premul", 220, 20, font, paint);
    canvas->drawString("Unpremul", 220, 70, font, paint);
    canvas->drawString("Gray", 5, 120, font, paint);
    canvas->drawString("White", 55, 120, font, paint);
    canvas->drawString("Gray", 115, 120, font, paint);
    canvas->drawString("White", 165, 120, font, paint);

    canvas->drawString("2x2", 40, 150, font, paint);
    canvas->drawString("1x1", 150, 150, font, paint);
}
}  // END FIDDLE
