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
#include "SkData.h"
#include "SkImageEncoderPriv.h"
#include "SkPM4f.h"
#include "SkSRGB.h"

namespace skiagm {

static const int imageWidth = 128;
static const int imageHeight = 128;

static inline int div_round_up(int a, int b) {
    return (a + b - 1) / b;
}

static void make_index8(SkBitmap* bitmap, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    const SkColor colors[] = {
            0x800000FF, 0x8000FF00, 0x80FF0000, 0x80FFFF00,
    };

    auto toPMColor = [alphaType, colorSpace](SkColor color) {
        // In the unpremul case, just convert to SkPMColor ordering.
        if (kUnpremul_SkAlphaType == alphaType) {
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

    sk_sp<SkColorTable> colorTable(new SkColorTable(pmColors, SK_ARRAY_COUNT(pmColors)));
    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kIndex_8_SkColorType,
                                         alphaType, colorSpace);
    bitmap->allocPixels(info, nullptr, colorTable.get());
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            *bitmap->getAddr8(x, y) = (x / div_round_up(imageWidth, 2)) +
                                      (y / div_round_up(imageHeight, 3));
        }
    }
}

static void make(SkBitmap* bitmap, SkColorType colorType, SkAlphaType alphaType,
                 sk_sp<SkColorSpace> colorSpace) {
    if (kIndex_8_SkColorType == colorType) {
        make_index8(bitmap, alphaType, colorSpace);
        return;
    }

    sk_sp<SkData> data = GetResourceAsData("color_wheel.png");
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    SkImageInfo dstInfo = codec->getInfo().makeColorType(colorType)
                                          .makeAlphaType(alphaType)
                                          .makeColorSpace(colorSpace);
    bitmap->allocPixels(dstInfo);
    codec->getPixels(dstInfo, bitmap->getPixels(), bitmap->rowBytes());
}

static sk_sp<SkData> encode_data(const SkBitmap& bitmap) {
    SkAutoLockPixels autoLockPixels(bitmap);
    SkPixmap src;
    if (!bitmap.peekPixels(&src)) {
        return nullptr;
    }
    SkDynamicMemoryWStream buf;
    SkEncodeOptions options;
    if (bitmap.colorSpace()) {
        options.fPremulBehavior = SkEncodeOptions::PremulBehavior::kGammaCorrect;
    }
    SkAssertResult(SkEncodeImageAsPNG(&buf, src, options));
    return buf.detachAsData();
}

class EncodeSRGBGM : public GM {
public:
    EncodeSRGBGM() {}

protected:
    SkString onShortName() override {
        return SkString("encode-srgb");
    }

    SkISize onISize() override {
        return SkISize::Make(imageWidth * 2, imageHeight * 4);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkColorType colorTypes[] = { kN32_SkColorType, kIndex_8_SkColorType, };
        const SkAlphaType alphaTypes[] = { kUnpremul_SkAlphaType, kPremul_SkAlphaType, };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                nullptr, SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named),
        };

        SkBitmap bitmap;
        for (SkColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaTypes) {
                canvas->save();
                for (sk_sp<SkColorSpace> colorSpace : colorSpaces) {
                    make(&bitmap, colorType, alphaType, colorSpace);
                    auto image = SkImage::MakeFromEncoded(encode_data(bitmap));
                    canvas->drawImage(image.get(), 0.0f, 0.0f);
                    canvas->translate((float) imageWidth, 0.0f);
                }
                canvas->restore();
                canvas->translate(0.0f, (float) imageHeight);
            }
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodeSRGBGM; )
}
