/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

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
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        paint.setColorFilter(SkColorFilters::Matrix(opaqueGrayMatrix));

        canvas->drawImage(bitmap.asImage(), 100.0f, 100.0f, SkSamplingOptions(), &paint);
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ColorFilterAlpha8; )
