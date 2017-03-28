/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRandom.h"
#include "SkRRect.h"

namespace skiagm {

class BitmapColorSpace : public GM {
public:
    BitmapColorSpace() {}

    SkString onShortName() override { return SkString("bitmapcolorspace"); }
    SkISize onISize() override { return SkISize::Make(50, 50); }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bitmap;
        sk_sp<SkColorSpace> adobe = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                          SkColorSpace::kAdobeRGB_Gamut);
        SkImageInfo info = SkImageInfo::Make(25, 50, kN32_SkColorType, kOpaque_SkAlphaType, adobe);
        bitmap.allocPixels(info);
        bitmap.eraseColor(0xFFE00000);
        canvas->drawBitmap(bitmap, 0.0f, 0.0f, nullptr);

        canvas->translate(25.0f, 0.0f);

        info = SkImageInfo::MakeS32(25, 50, kOpaque_SkAlphaType);
        bitmap.allocPixels(info);
        bitmap.eraseColor(0xFFFF0000);
        canvas->drawBitmap(bitmap, 0.0f, 0.0f, nullptr);
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new BitmapColorSpace(); )

}
