/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkImage.h"
#include "SkImagePriv.h"

sk_sp<SkImage> make_raster_image(const char* path) {
    sk_sp<SkData> resourceData = GetResourceAsData(path);
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(resourceData);

    SkBitmap bitmap;
    bitmap.allocPixels(codec->getInfo());

    codec->getPixels(codec->getInfo(), bitmap.getPixels(), bitmap.rowBytes());
    return SkImage::MakeFromBitmap(bitmap);
}

sk_sp<SkImage> make_color_space(sk_sp<SkImage> orig, sk_sp<SkColorSpace> colorSpace) {
    sk_sp<SkImage> xform = orig->makeColorSpace(colorSpace);

    // Assign an sRGB color space on the xformed image, so we can see the effects of the xform
    // when we draw.
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    if (colorSpace->gammaIsLinear()) {
        srgb = SkColorSpace::MakeSRGBLinear();
    }
    return SkImageMakeRasterCopyAndAssignColorSpace(xform.get(), srgb.get());
}

class MakeCSGM : public skiagm::GM {
public:
    MakeCSGM() {}

protected:
    SkString onShortName() override {
        return SkString("makecolorspace");
    }

    SkISize onISize() override {
        return SkISize::Make(128*3, 128*4);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkColorSpace> wideGamut = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                              SkColorSpace::kAdobeRGB_Gamut);
        sk_sp<SkColorSpace> wideGamutLinear = wideGamut->makeLinearGamma();

        // Lazy images
        sk_sp<SkImage> opaqueImage = GetResourceAsImage("images/mandrill_128.png");
        sk_sp<SkImage> premulImage = GetResourceAsImage("images/color_wheel.png");
        canvas->drawImage(opaqueImage, 0.0f, 0.0f);
        canvas->drawImage(make_color_space(opaqueImage, wideGamut), 128.0f, 0.0f);
        canvas->drawImage(make_color_space(opaqueImage, wideGamutLinear), 256.0f, 0.0f);
        canvas->drawImage(premulImage, 0.0f, 128.0f);
        canvas->drawImage(make_color_space(premulImage, wideGamut), 128.0f, 128.0f);
        canvas->drawImage(make_color_space(premulImage, wideGamutLinear), 256.0f, 128.0f);
        canvas->translate(0.0f, 256.0f);

        // Raster images
        opaqueImage = make_raster_image("images/mandrill_128.png");
        premulImage = make_raster_image("images/color_wheel.png");
        canvas->drawImage(opaqueImage, 0.0f, 0.0f);
        canvas->drawImage(make_color_space(opaqueImage, wideGamut), 128.0f, 0.0f);
        canvas->drawImage(make_color_space(opaqueImage, wideGamutLinear), 256.0f, 0.0f);
        canvas->drawImage(premulImage, 0.0f, 128.0f);
        canvas->drawImage(make_color_space(premulImage, wideGamut), 128.0f, 128.0f);
        canvas->drawImage(make_color_space(premulImage, wideGamutLinear), 256.0f, 128.0f);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new MakeCSGM;)
