/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "src/images/SkImageEncoderPriv.h"
#include "tools/Resources.h"

namespace skiagm {

static const int imageWidth = 128;
static const int imageHeight = 128;

static void make(SkBitmap* bitmap, SkColorType colorType, SkAlphaType alphaType,
                 sk_sp<SkColorSpace> colorSpace) {
    const char* resource;
    switch (colorType) {
        case kGray_8_SkColorType:
            resource = "images/grayscale.jpg";
            alphaType = kOpaque_SkAlphaType;
            break;
        case kRGB_565_SkColorType:
            resource = "images/color_wheel.jpg";
            alphaType = kOpaque_SkAlphaType;
            break;
        default:
            resource = (kOpaque_SkAlphaType == alphaType) ? "images/color_wheel.jpg"
                                                          : "images/color_wheel.png";
            break;
    }

    sk_sp<SkData> data = GetResourceAsData(resource);
    if (!data) {
        return;
    }
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    SkImageInfo dstInfo = codec->getInfo().makeColorType(colorType)
                                          .makeAlphaType(alphaType)
                                          .makeColorSpace(colorSpace);
    bitmap->allocPixels(dstInfo);
    codec->getPixels(dstInfo, bitmap->getPixels(), bitmap->rowBytes());
}

static sk_sp<SkData> encode_data(const SkBitmap& bitmap, SkEncodedImageFormat format) {
    SkPixmap src;
    if (!bitmap.peekPixels(&src)) {
        return nullptr;
    }
    SkDynamicMemoryWStream buf;

    switch (format) {
        case SkEncodedImageFormat::kPNG:
            SkAssertResult(SkPngEncoder::Encode(&buf, src, SkPngEncoder::Options()));
            break;
        case SkEncodedImageFormat::kWEBP:
            SkAssertResult(SkWebpEncoder::Encode(&buf, src, SkWebpEncoder::Options()));
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
            kN32_SkColorType, kRGBA_F16_SkColorType, kGray_8_SkColorType, kRGB_565_SkColorType,
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
