/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorSpaceXformImageGenerator.h"

class ColorXformImageGenGM : public skiagm::GM {
public:
    ColorXformImageGenGM() {}

protected:

    SkString onShortName() override {
        return SkString("color_xform_image_gen");
    }

    SkISize onISize() override {
        return SkISize::Make(100, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bitmap;
        SkImageInfo info =
                SkImageInfo::MakeN32(100, 100, kOpaque_SkAlphaType, SkColorSpace::MakeSRGB());
        bitmap.allocPixels(info);
        bitmap.eraseColor(SK_ColorRED);
        bitmap.eraseArea(SkIRect::MakeWH(25, 25), SK_ColorBLUE); // We should not see any blue.

        std::unique_ptr<SkImageGenerator> gen = SkColorSpaceXformImageGenerator::Make(
                bitmap,
                SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                      SkColorSpace::kRec2020_Gamut),
                kNever_SkCopyPixelsMode);

        SkIRect subset = SkIRect::MakeXYWH(25, 25, 50, 50);
        sk_sp<SkImage> image = SkImage::MakeFromGenerator(std::move(gen), &subset);
        canvas->drawImage(image, 25, 25);
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new ColorXformImageGenGM(); )
