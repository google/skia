/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "tools/Resources.h"

#include <memory>

namespace {

static constexpr int kImageWidth = 128;
static constexpr int kImageHeight = 128;

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

class EncodeSRGBGM : public skiagm::GM {
public:
    EncodeSRGBGM(SkEncodedImageFormat f, const char* n) : fName(n), fEncodedFormat(f) {}

private:
    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return {kImageWidth * 2, kImageHeight * 15}; }

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
                    canvas->translate((float) kImageWidth, 0.0f);
                }
                canvas->restore();
                canvas->translate(0.0f, (float) kImageHeight);
            }
        }
    }

    const char* fName = nullptr;
    SkEncodedImageFormat fEncodedFormat;
};
}  // namespace

DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kPNG,  "encode-srgb-png" ); )
DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kWEBP, "encode-srgb-webp"); )
DEF_GM( return new EncodeSRGBGM(SkEncodedImageFormat::kJPEG, "encode-srgb-jpg" ); )
