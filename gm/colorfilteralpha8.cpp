/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"

class ColorFilterAlpha8 : public skiagm::GM {
public:
    ColorFilterAlpha8() {}

protected:
    SkString onShortName() override {
        return SkString("colorfilteralpha8");
    }

    SkISize onISize() override {
        return SkISize::Make(400, 400);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorRED);

        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::MakeA8(200, 200);
        bitmap.allocPixels(info);
        bitmap.eraseColor(0x88FFFFFF);

        SkPaint paint;
        float opaqueGrayMatrix[20] = {
                0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 255.0f
        };
        paint.setColorFilter(SkColorFilters::MatrixRowMajor255(opaqueGrayMatrix));

        canvas->drawBitmap(bitmap, 100.0f, 100.0f, &paint);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ColorFilterAlpha8; )
