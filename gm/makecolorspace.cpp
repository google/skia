/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkImagePriv.h"
#include "tools/Resources.h"

#include <initializer_list>
#include <memory>

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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        sk_sp<SkColorSpace> wideGamut = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                              SkNamedGamut::kAdobeRGB);
        sk_sp<SkColorSpace> wideGamutLinear = wideGamut->makeLinearGamma();

        // Lazy images
        sk_sp<SkImage> opaqueImage = GetResourceAsImage("images/mandrill_128.png");
        sk_sp<SkImage> premulImage = GetResourceAsImage("images/color_wheel.png");
        if (!opaqueImage || !premulImage) {
            *errorMsg = "Failed to load images. Did you forget to set the resourcePath?";
            return DrawResult::kFail;
        }
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
        return DrawResult::kOk;
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new MakeCSGM;)

DEF_SIMPLE_GM_BG(makecolortypeandspace, canvas, 128 * 3, 128 * 4, SK_ColorWHITE) {
    sk_sp<SkImage> images[] = {
        GetResourceAsImage("images/mandrill_128.png"),
        GetResourceAsImage("images/color_wheel.png"),
    };
    auto rec2020 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kRec2020);

    // Use the lazy images on the first iteration, and concrete (raster/GPU) images on the second
    for (bool lazy : {true, false}) {
        for (int j = 0; j < 2; ++j) {
            const SkImage* image = images[j].get();
            if (!image) {
                // Can happen on bots that abandon the GPU context
                continue;
            }

            // Unmodified
            canvas->drawImage(image, 0, 0);

            // Change the color type/space of the image in a couple ways. In both cases, codec
            // may fail, because we refude to decode transparent sources to opaque color types.
            // Guard against that, to avoid cascading failures in DDL.

            // 565 in a wide color space (should be visibly quantized). Fails with the color_wheel,
            // because of the codec issues mentioned above.
            auto image565 = image->makeColorTypeAndColorSpace(kRGB_565_SkColorType, rec2020);
            if (!lazy || image565->makeRasterImage()) {
                canvas->drawImage(image565, 128, 0);
            }

            // Grayscale in the original color space. This fails in even more cases, due to the
            // above opaque issue, and because Ganesh doesn't support drawing to gray, at all.
            auto imageGray = image->makeColorTypeAndColorSpace(kGray_8_SkColorType,
                                                               image->refColorSpace());
            if (!lazy || imageGray->makeRasterImage()) {
                canvas->drawImage(imageGray, 256, 0);
            }

            images[j] = canvas->getGrContext()
                    ? image->makeTextureImage(canvas->getGrContext(), nullptr)
                    : image->makeRasterImage();

            canvas->translate(0, 128);
        }
    }
}
