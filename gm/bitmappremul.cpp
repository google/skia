/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"

/**
 * This GM checks that bitmap pixels are unpremultiplied before being exported
 * to other formats. If unpremultiplication is implemented properly, this
 * GM should come out completely white. If not, this GM looks like a row of two
 * greyscale gradients above a row of grey lines.
 * This tests both the ARGB4444 and ARGB8888 bitmap configurations.
 */

constexpr int SLIDE_SIZE = 256;

static void init_bitmap(SkColorType ct, SkBitmap* bitmap) {
    bitmap->allocPixels(SkImageInfo::Make(SLIDE_SIZE, SLIDE_SIZE, ct,
                                          kPremul_SkAlphaType));
    bitmap->eraseColor(SK_ColorWHITE);
}

static sk_sp<SkImage> make_argb8888_gradient() {
    SkBitmap bitmap;
    init_bitmap(kN32_SkColorType, &bitmap);
    for (int y = 0; y < SLIDE_SIZE; y++) {
        uint32_t* dst = bitmap.getAddr32(0, y);
        for (int x = 0; x < SLIDE_SIZE; x++) {
            dst[x] = SkPackARGB32(y, y, y, y);
        }
    }
    return bitmap.asImage();
}

static sk_sp<SkImage> make_argb4444_gradient() {
    SkBitmap bitmap;
    init_bitmap(kARGB_4444_SkColorType, &bitmap);
    // Using draw rather than readPixels to suppress dither
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    SkCanvas{ bitmap }.drawImage(make_argb8888_gradient(), 0, 0, SkSamplingOptions(), &paint);
    return bitmap.asImage();
}

static sk_sp<SkImage> make_argb8888_stripes() {
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
    return bitmap.asImage();
}

static sk_sp<SkImage> make_argb4444_stripes() {
    SkBitmap bitmap;
    init_bitmap(kARGB_4444_SkColorType, &bitmap);
    // Using draw rather than readPixels to suppress dither
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    SkCanvas{ bitmap }.drawImage(make_argb8888_stripes(), 0, 0, SkSamplingOptions(), &paint);
    return bitmap.asImage();
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
        canvas->drawImage(make_argb8888_gradient(), 0, 0);
        canvas->drawImage(make_argb4444_gradient(), slideSize, 0);
        canvas->drawImage(make_argb8888_stripes(), 0, slideSize);
        canvas->drawImage(make_argb4444_stripes(), slideSize, slideSize);
    }

private:
    using INHERITED = GM;
};

DEF_GM( return new BitmapPremulGM; )
}  // namespace skiagm
