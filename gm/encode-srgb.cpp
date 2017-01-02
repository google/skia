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

namespace skiagm {

static const int imageWidth = 128;
static const int imageHeight = 128;

static void make(SkBitmap* bitmap, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    sk_sp<SkData> data = GetResourceAsData("color_wheel.png");
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    SkImageInfo dstInfo = codec->getInfo().makeColorType(kN32_SkColorType)
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
    SkAssertResult(SkEncodeImageAsPNG(&buf, src));
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
        return SkISize::Make(imageWidth * 2, imageHeight * 2);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkAlphaType alphaTypes[] = { kUnpremul_SkAlphaType, kPremul_SkAlphaType, };
        const sk_sp<SkColorSpace> colorSpaces[] = {
                nullptr, SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named),
        };

        SkBitmap bitmap;
        for (SkAlphaType alphaType: alphaTypes) {
            canvas->save();
            for (sk_sp<SkColorSpace> colorSpace : colorSpaces) {
                make(&bitmap, alphaType, colorSpace);
                auto image = SkImage::MakeFromEncoded(encode_data(bitmap));
                canvas->drawImage(image.get(), 0.0f, 0.0f);
                canvas->translate((float) imageWidth, 0.0f);
            }
            canvas->restore();
            canvas->translate(0.0f, (float) imageHeight);
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodeSRGBGM; )
}
