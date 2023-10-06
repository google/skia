/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkWebpEncoder.h"
#include "tools/DecodeUtils.h"
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
                                                          : "images/rainbow-gradient.png";
            break;
    }

    auto image = ToolUtils::GetResourceAsImage(resource);
    if (!image) {
        return nullptr;
    }

    auto surface = SkSurfaces::Raster(SkImageInfo::Make(
            image->width(), image->height(), colorType, alphaType, image->refColorSpace()));
    surface->getCanvas()->drawImage(image, 0, 0);
    return surface->makeImageSnapshot();
}

// This GM looks different depending on the colortype used, so we use different
// names to treat them as different GMs.
// All Variants draw images in pairs:
// - First as an image of the SkColorType of the destination
// - Next as the result of encoding and decoding the first image
enum class Variant {
    // One pair, using an opaque image.
    kOpaque,
    // One pair, using a grayscale image.
    kGray,
    // An opaque pair followed by two more for premul and unpremul.
    kNormal,
};

class EncodeColorTypesGM : public GM {
public:
    EncodeColorTypesGM(SkEncodedImageFormat format, int quality, Variant variant, const char* name)
        : fFormat(format)
        , fQuality(quality)
        , fVariant(variant)
        , fName(name)
    {}

protected:
    SkString getName() const override {
        const char* variant = fVariant == Variant::kOpaque ? "opaque-":
                              fVariant == Variant::kGray   ? "gray-"  :
                                                             ""       ;
        return SkStringPrintf("encode-%scolor-types-%s", variant, fName);
    }

    SkISize getISize() override {
        const int width = fVariant == Variant::kNormal ? imageWidth * 7 : imageWidth * 2;
        return SkISize::Make(width, imageHeight);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        const auto colorType = canvas->imageInfo().colorType();
        switch (fVariant) {
            case Variant::kGray:
                if (colorType != kGray_8_SkColorType) {
                    return DrawResult::kSkip;
                }
                break;
            case Variant::kOpaque:
                if (colorType != kRGB_565_SkColorType     &&
                    colorType != kRGB_888x_SkColorType    &&
                    colorType != kRGB_101010x_SkColorType &&
                    colorType != kBGR_101010x_SkColorType)
                {
                    return DrawResult::kSkip;
                }
                break;
            case Variant::kNormal:
                if (colorType != kARGB_4444_SkColorType    &&
                    colorType != kRGBA_8888_SkColorType    &&
                    colorType != kBGRA_8888_SkColorType    &&
                    colorType != kRGBA_1010102_SkColorType &&
                    colorType != kBGRA_1010102_SkColorType &&
                    colorType != kRGBA_F16Norm_SkColorType &&
                    colorType != kRGBA_F16_SkColorType     &&
                    colorType != kRGBA_F32_SkColorType)
                {
                    return DrawResult::kSkip;
                }
            break;
        }
        const SkAlphaType alphaTypes[] = {
            kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType,
        };

        for (SkAlphaType alphaType : alphaTypes) {
            auto src = make_image(colorType, alphaType);
            if (!src) {
                break;
            }
            SkASSERT_RELEASE(fFormat == SkEncodedImageFormat::kWEBP);
            SkWebpEncoder::Options options;
            if (fQuality < 100) {
                options.fCompression = SkWebpEncoder::Compression::kLossy;
                options.fQuality = fQuality;
            } else {
                options.fCompression = SkWebpEncoder::Compression::kLossless;
                // in lossless mode, this is effort. 70 is the default effort in SkImageEncoder,
                // which follows Blink and WebPConfigInit.
                options.fQuality = 70;
            }
            auto data = SkWebpEncoder::Encode(nullptr, src.get(), options);
            SkASSERT(data);
            auto decoded = SkImages::DeferredFromEncodedData(data);
            if (!decoded) {
                break;
            }

            canvas->drawImage(src, 0.0f, 0.0f);
            canvas->translate((float) imageWidth, 0.0f);

            canvas->drawImage(decoded, 0.0f, 0.0f);
            canvas->translate((float) imageWidth * 1.5, 0.0f);
        }
        return DrawResult::kOk;
    }

private:
    const SkEncodedImageFormat fFormat;
    const int                  fQuality;
    const Variant              fVariant;
    const char*                fName;

    using INHERITED = GM;
};


#define DEF_ENCODE_GM(format, quality, variant, name) \
    static skiagm::GMRegistry SK_MACRO_CONCAT(SK_MACRO_APPEND_LINE(REG_), variant)(\
            [](){return std::unique_ptr<skiagm::GM>([](){\
                    return new EncodeColorTypesGM(format, quality, Variant::variant, name);\
                }());});

#define DEF_VARIANTS(format, quality, name)         \
    DEF_ENCODE_GM(format, quality, kNormal, name);  \
    DEF_ENCODE_GM(format, quality, kOpaque, name);  \
    DEF_ENCODE_GM(format, quality, kGray, name);

DEF_VARIANTS(SkEncodedImageFormat::kWEBP, 100, "webp-lossless")
DEF_VARIANTS(SkEncodedImageFormat::kWEBP,  80, "webp-lossy")
}  // namespace skiagm
