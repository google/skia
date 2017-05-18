/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"

/**
 * This GM checks that bitmap pixels are unpremultiplied before being exported
 * to other formats. If unpremultiplication is implemented properly, this
 * GM should come out completely white. If not, this GM looks like a row of two
 * greyscale gradients above a row of grey lines.
 * This tests both the ARGB4444 and ARGB8888 bitmap configurations.
 */

constexpr int SLIDE_SIZE = 256;
constexpr int PIXEL_SIZE_8888 = SLIDE_SIZE / 256;
constexpr int PIXEL_SIZE_4444 = SLIDE_SIZE / 16;

static void init_bitmap(SkColorType ct, SkBitmap* bitmap) {
    bitmap->allocPixels(SkImageInfo::Make(SLIDE_SIZE, SLIDE_SIZE, ct,
                                          kPremul_SkAlphaType));
    bitmap->eraseColor(SK_ColorWHITE);
}

static SkBitmap make_argb8888_gradient() {
    SkBitmap bitmap;
    init_bitmap(kN32_SkColorType, &bitmap);
    uint8_t rowColor = 0;
    for (int y = 0; y < SLIDE_SIZE; y++) {
        uint32_t* dst = bitmap.getAddr32(0, y);
        for (int x = 0; x < SLIDE_SIZE; x++) {
            dst[x] = SkPackARGB32(rowColor, rowColor,
                                  rowColor, rowColor);
        }
        if (y % PIXEL_SIZE_8888 == PIXEL_SIZE_8888 - 1) {
            rowColor++;
        }
    }
    return bitmap;
}

static SkBitmap make_argb4444_gradient() {
    SkBitmap bitmap;
    init_bitmap(kARGB_4444_SkColorType, &bitmap);
    uint8_t rowColor = 0;
    for (int y = 0; y < SLIDE_SIZE; y++) {
        uint16_t* dst = bitmap.getAddr16(0, y);
        for (int x = 0; x < SLIDE_SIZE; x++) {
            dst[x] = SkPackARGB4444(rowColor, rowColor,
                                    rowColor, rowColor);
        }
        if (y % PIXEL_SIZE_4444 == PIXEL_SIZE_4444 - 1) {
            rowColor++;
        }
    }
    return bitmap;
}

static SkBitmap make_argb8888_stripes() {
    SkBitmap bitmap;
    init_bitmap(kN32_SkColorType, &bitmap);
    uint8_t rowColor = 0;
    for (int y = 0; y < SLIDE_SIZE; y++) {
        uint32_t* dst = bitmap.getAddr32(0, y);
        for (int x = 0; x < SLIDE_SIZE; x++) {
            dst[x] = SkPackARGB32(rowColor, rowColor,
                                  rowColor, rowColor);
        }
        if (rowColor == 0) {
            rowColor = 255;
        } else {
            rowColor = 0;
        }
    }
    return bitmap;
}

static SkBitmap make_argb4444_stripes() {
    SkBitmap bitmap;
    init_bitmap(kARGB_4444_SkColorType, &bitmap);
    uint8_t rowColor = 0;
    for (int y = 0; y < SLIDE_SIZE; y++) {
        uint16_t* dst = bitmap.getAddr16(0, y);
        for (int x = 0; x < SLIDE_SIZE; x++) {
            dst[x] = SkPackARGB4444(rowColor, rowColor,
                                    rowColor, rowColor);
        }
        if (rowColor == 0) {
            rowColor = 15;
        } else {
            rowColor = 0;
        }
    }
    return bitmap;
}

namespace skiagm {

class BitmapPremulGM : public GM {
public:
    BitmapPremulGM() {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    SkString onShortName() override {
        return SkString("bitmap_premul");
    }

    SkISize onISize() override {
        return SkISize::Make(SLIDE_SIZE * 2, SLIDE_SIZE * 2);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar slideSize = SkIntToScalar(SLIDE_SIZE);
        canvas->drawBitmap(make_argb8888_gradient(), 0, 0);
        canvas->drawBitmap(make_argb4444_gradient(), slideSize, 0);
        canvas->drawBitmap(make_argb8888_stripes(), 0, slideSize);
        canvas->drawBitmap(make_argb4444_stripes(), slideSize, slideSize);
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new BitmapPremulGM; )
}
