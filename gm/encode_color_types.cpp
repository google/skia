/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/Resources.h"

namespace skiagm {

static const int imageWidth = 128;
static const int imageHeight = 128;

static sk_sp<SkImage> make_image(SkColorType colorType, SkAlphaType alphaType) {
    const char* resource;
    switch (colorType) {
        case kGray_8_SkColorType:
            if (alphaType != kOpaque_SkAlphaType) {
                return nullptr;
            }
            resource = "images/grayscale.jpg";
            break;
        case kRGB_565_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
            if (alphaType != kOpaque_SkAlphaType) {
                return nullptr;
            }
            resource = "images/color_wheel.jpg";
            break;
        default:
            resource = (kOpaque_SkAlphaType == alphaType) ? "images/color_wheel.jpg"
                                                          : "images/color_wheel.png";
            break;
    }

    auto image = GetResourceAsImage(resource);
    if (!image) {
        return nullptr;
    }

    auto surface = SkSurface::MakeRaster(SkImageInfo::Make(image->width(), image->height(),
            colorType, alphaType, image->refColorSpace()));
    surface->getCanvas()->drawImage(image, 0, 0);
    return surface->makeImageSnapshot();
}

class EncodeColorTypesGM : public GM {
public:
    EncodeColorTypesGM(SkEncodedImageFormat format, int quality, const char* name)
        : fFormat(format)
        , fQuality(quality)
        , fName(name)
    {}

protected:
    SkString onShortName() override {
        return SkStringPrintf("encode-color-types-%s", fName);
    }

    SkISize onISize() override {
        return SkISize::Make(imageWidth * 7, imageHeight * 13);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkColorType colorTypes[] = {
            kARGB_4444_SkColorType,
            kRGBA_8888_SkColorType,
            kBGRA_8888_SkColorType,
            kRGBA_1010102_SkColorType,
            kBGRA_1010102_SkColorType,
            kRGBA_F16Norm_SkColorType,
            kRGBA_F16_SkColorType,
            kRGBA_F32_SkColorType,

            // Opaque types
            kGray_8_SkColorType,
            kRGB_565_SkColorType,
            kRGB_888x_SkColorType,
            kRGB_101010x_SkColorType,
            kBGR_101010x_SkColorType,
        };
        const SkAlphaType alphaTypes[] = {
            kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType,
        };

        for (SkColorType colorType : colorTypes) {
            canvas->save();
            for (SkAlphaType alphaType : alphaTypes) {
                auto src = make_image(colorType, alphaType);
                auto decoded = src ? SkImage::MakeFromEncoded(src->encodeToData(fFormat, fQuality))
                                   : nullptr;
                if (!src || !decoded) {
                    break;
                }

                canvas->drawImage(src, 0.0f, 0.0f);
                canvas->translate((float) imageWidth, 0.0f);

                canvas->drawImage(decoded, 0.0f, 0.0f);
                canvas->translate((float) imageWidth * 1.5, 0.0f);
            }
            canvas->restore();
            canvas->translate(0.0f, (float) imageHeight);
        }
    }

private:
    const SkEncodedImageFormat fFormat;
    const int fQuality;
    const char* fName;

    typedef GM INHERITED;
};

DEF_GM( return new EncodeColorTypesGM(SkEncodedImageFormat::kWEBP, 100, "webp-lossless"); )
DEF_GM( return new EncodeColorTypesGM(SkEncodedImageFormat::kWEBP, 80,  "webp-lossy"); )
}
