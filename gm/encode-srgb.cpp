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

static void make(SkBitmap* bitmap, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    sk_sp<SkData> data = GetResourceAsData("color_wheel.png");
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    SkImageInfo dstInfo = codec->getInfo().makeAlphaType(alphaType).makeColorSpace(colorSpace);
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
    return SkEncodeImageAsPNG(&buf, src) ? buf.detachAsData() : nullptr;
}

class EncodeSRGBGM : public GM {
public:
    EncodeSRGBGM() {}

protected:
    SkString onShortName() override {
        return SkString("encode-srgb");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bitmap;
        make(&bitmap, kPremul_SkAlphaType, nullptr);
        auto image0 = SkImage::MakeFromEncoded(encode_data(bitmap));
        make(&bitmap, kUnpremul_SkAlphaType, nullptr);
        auto image1 = SkImage::MakeFromEncoded(encode_data(bitmap));
        make(&bitmap, kPremul_SkAlphaType, SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named));
        auto image2 = SkImage::MakeFromEncoded(encode_data(bitmap));
        make(&bitmap, kUnpremul_SkAlphaType, SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named));
        auto image3 = SkImage::MakeFromEncoded(encode_data(bitmap));
        canvas->drawImage(image0.get(), 0.0f, 0.0f);
        canvas->drawImage(image1.get(), 0.0f, 128.0f);
        canvas->drawImage(image2.get(), 128.0f, 0.0f);
        canvas->drawImage(image3.get(), 128.0f, 128.0f);
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodeSRGBGM; )
}
