/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkColorSpace_Base.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageEncoderPriv.h"
#include "SkJpegEncoder.h"
#include "SkPngEncoder.h"
#include "SkPM4f.h"
#include "SkSRGB.h"
#include "SkWebpEncoder.h"

namespace skiagm {

static const int imageWidth = 128;
static const int imageHeight = 128;

static inline int div_round_up(int a, int b) {
    return (a + b - 1) / b;
}

sk_sp<SkColorSpace> fix_for_colortype(sk_sp<SkColorSpace> colorSpace, SkColorType colorType) {
    if (kRGBA_F16_SkColorType == colorType) {
        if (!colorSpace) {
            return SkColorSpace::MakeSRGBLinear();
        }

        return as_CSB(colorSpace)->makeLinearGamma();
    }

    return colorSpace;
}

static void make_index8(SkBitmap* bitmap, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    const SkColor colors[] = {
            0x800000FF, 0x8000FF00, 0x80FF0000, 0x80FFFF00,
    };

    auto toPMColor = [alphaType, colorSpace](SkColor color) {
        // In the opaque/unpremul case, just convert to SkPMColor ordering.
        if (kPremul_SkAlphaType != alphaType) {
            return SkSwizzle_BGRA_to_PMColor(color);
        }

        // Linear premultiply.
        if (colorSpace) {
            uint32_t result;
            Sk4f pmFloat = SkColor4f::FromColor(color).premul().to4f_pmorder();
            SkNx_cast<uint8_t>(sk_linear_to_srgb_needs_trunc(pmFloat)).store(&result);
            result = (result & 0x00FFFFFF) | (color & 0xFF000000);
            return result;
        }

        // Legacy premultiply.
        return SkPreMultiplyColor(color);
    };

    // Note that these are not necessarily premultiplied, but they are platform byte ordering.
    SkPMColor pmColors[SK_ARRAY_COUNT(colors)];
    for (int i = 0; i < (int) SK_ARRAY_COUNT(colors); i++) {
        pmColors[i] = toPMColor(colors[i]);
    }

    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kIndex_8_SkColorType,
                                         alphaType, colorSpace);
    bitmap->allocPixels(info, SkColorTable::Make(pmColors, SK_ARRAY_COUNT(pmColors)));
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            *bitmap->getAddr8(x, y) = (x / div_round_up(imageWidth, 2)) +
                                      (y / div_round_up(imageHeight, 3));
        }
    }
}

static void make(SkBitmap* bitmap, SkColorType colorType, SkAlphaType alphaType,
                 sk_sp<SkColorSpace> colorSpace) {
    const char* resource;
    switch (colorType) {
        case kIndex_8_SkColorType:
            make_index8(bitmap, alphaType, colorSpace);
            return;
        case kGray_8_SkColorType:
            resource = "grayscale.jpg";
            alphaType = kOpaque_SkAlphaType;
            break;
        case kRGB_565_SkColorType:
            resource = "color_wheel.jpg";
            alphaType = kOpaque_SkAlphaType;
            break;
        default:
            resource = (kOpaque_SkAlphaType == alphaType) ? "color_wheel.jpg"
                                                          : "color_wheel.png";
            break;
    }

    sk_sp<SkData> data = GetResourceAsData(resource);
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    SkImageInfo dstInfo = codec->getInfo().makeColorType(colorType)
                                          .makeAlphaType(alphaType)
                                          .makeColorSpace(fix_for_colortype(colorSpace, colorType));
    bitmap->allocPixels(dstInfo);
    codec->getPixels(dstInfo, bitmap->getPixels(), bitmap->rowBytes());
}

static sk_sp<SkData> encode_data(const SkBitmap& bitmap, SkEncodedImageFormat format) {
    SkPixmap src;
    if (!bitmap.peekPixels(&src)) {
        return nullptr;
    }
    SkDynamicMemoryWStream buf;

    SkPngEncoder::Options pngOptions;
    SkWebpEncoder::Options webpOptions;
    if (bitmap.colorSpace()) {
        pngOptions.fUnpremulBehavior = SkTransferFunctionBehavior::kRespect;
        webpOptions.fUnpremulBehavior = SkTransferFunctionBehavior::kRespect;
    }

    switch (format) {
        case SkEncodedImageFormat::kPNG:
            SkAssertResult(SkPngEncoder::Encode(&buf, src, pngOptions));
            break;
        case SkEncodedImageFormat::kWEBP:
            SkAssertResult(SkWebpEncoder::Encode(&buf, src, webpOptions));
            break;
        case SkEncodedImageFormat::kJPEG:
            SkAssertResult(SkJpegEncoder::Encode(&buf, src, SkJpegEncoder::Options()));
            break;
        default:
            break;
    }
    return buf.detachAsData();
}

class EncodeSRGBGM : public GM {
public:
    EncodeSRGBGM(SkEncodedImageFormat format)
        : fEncodedFormat(format)
    {}

protected:
    SkString onShortName() override {
        const char* format = nullptr;
        switch (fEncodedFormat) {
            case SkEncodedImageFormat::kPNG:
                format = "png";
                break;
            case SkEncodedImageFormat::kWEBP:
                format = "webp";
                break;
            case SkEncodedImageFormat::kJPEG:
                format = "jpg";
                break;
            default:
                break;
        }
        return SkStringPrintf("encode-srgb-%s", format);
    }

    SkISize onISize() override {
        return SkISize::Make(imageWidth * 2, imageHeight * 15);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkColorType colorTypes[] = {
                kN32_SkColorType, kRGBA_F16_SkColorType, kIndex_8_SkColorType, kGray_8_SkColorType,
                kRGB_565_SkColorType,
        };
        const SkAlphaType alphaTypes[] = {
                kUnpremul_SkAlphaType, kPremul_SkAlphaType, kOpaque_SkAlphaType,
        };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                nullptr, SkColorSpace::MakeSRGB(),
        };

        SkBitmap bitmap;
        for (SkColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaTypes) {
                canvas->save();
                for (sk_sp<SkColorSpace> colorSpace : colorSpaces) {
                    make(&bitmap, colorType, alphaType, colorSpace);
                    auto image = SkImage::MakeFromEncoded(encode_data(bitmap, fEncodedFormat));
                    canvas->drawImage(image.get(), 0.0f, 0.0f);
                    canvas->translate((float) imageWidth, 0.0f);
                }
                canvas->restore();
                canvas->translate(0.0f, (float) imageHeight);
            }
        }
    }

private:
    SkEncodedImageFormat fEncodedFormat;

    typedef GM INHERITED;
};

DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kPNG); )
DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kWEBP); )
DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kJPEG); )
}
