/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageEncoderPriv.h"
#include "SkUnPreMultiply.h"

namespace skiagm {

static void make_opaque_256(SkBitmap* bitmap) {
    GetResourceAsBitmap("mandrill_256.png", bitmap);
}

static void make_premul_256(SkBitmap* bitmap) {
    SkBitmap tmp;
    GetResourceAsBitmap("yellow_rose.png", &tmp);
    tmp.extractSubset(bitmap, SkIRect::MakeWH(256, 256));
    bitmap->lockPixels();
}

static void make_unpremul_256(SkBitmap* bitmap) {
    make_premul_256(bitmap);
    for (int y = 0; y < bitmap->height(); y++) {
        for (int x = 0; x < bitmap->width(); x++) {
            SkPMColor* pixel = bitmap->getAddr32(x, y);
            *pixel = SkUnPreMultiply::UnPreMultiplyPreservingByteOrder(*pixel);
        }
    }
    bitmap->setAlphaType(kUnpremul_SkAlphaType);
}

static SkImageEncoder* make_encoder(SkEncodedFormat type) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    return CreateImageEncoder_CG((SkImageEncoder::Type)type);
#elif defined(SK_BUILD_FOR_WIN)
    return CreateImageEncoder_WIC((SkImageEncoder::Type)type);
#else
    switch (type) {
        case kPNG_SkEncodedFormat:
            return CreatePNGImageEncoder();
        case kJPEG_SkEncodedFormat:
            return CreateJPEGImageEncoder();
        case kWEBP_SkEncodedFormat:
            return CreateWEBPImageEncoder();
        default:
            SkASSERT(false);
            return nullptr;
    }
#endif
}

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
static SkEncodedFormat kTypes[] {
        kPNG_SkEncodedFormat, kJPEG_SkEncodedFormat, kGIF_SkEncodedFormat,
        kBMP_SkEncodedFormat, kICO_SkEncodedFormat,
};
#elif defined(SK_BUILD_FOR_WIN)
// Use PNG multiple times because our WIC encoder does not support GIF, BMP, or ICO.
static SkEncodedFormat kTypes[] {
        kPNG_SkEncodedFormat, kJPEG_SkEncodedFormat, kPNG_SkEncodedFormat,
        kPNG_SkEncodedFormat, kPNG_SkEncodedFormat,
};
#else
// Use WEBP in place of GIF.  Use PNG two extra times.  We don't support GIF, BMP, or ICO.
static SkEncodedFormat kTypes[] {
        kPNG_SkEncodedFormat, kJPEG_SkEncodedFormat, kWEBP_SkEncodedFormat,
        kPNG_SkEncodedFormat, kPNG_SkEncodedFormat,
};
#endif

static sk_sp<SkData> encode_data(std::unique_ptr<SkImageEncoder>& encoder, const SkBitmap& src) {
    SkDynamicMemoryWStream buffer;
    return encoder->encodeStream(&buffer, src, 100) ? buffer.detachAsData() : nullptr;
}

class EncodePlatformGM : public GM {
public:
    EncodePlatformGM() {}

protected:
    SkString onShortName() override {
        return SkString("encode-platform");
    }

    SkISize onISize() override {
        return SkISize::Make(256 * SK_ARRAY_COUNT(kTypes), 256 * 3);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap opaqueBm, premulBm, unpremulBm;
        make_opaque_256(&opaqueBm);
        make_premul_256(&premulBm);
        make_unpremul_256(&unpremulBm);

        for (SkEncodedFormat type : kTypes) {
            std::unique_ptr<SkImageEncoder> encoder(make_encoder(type));
            sk_sp<SkData> opaqueData = encode_data(encoder, opaqueBm);
            sk_sp<SkData> premulData = encode_data(encoder, premulBm);
            sk_sp<SkData> unpremulData = encode_data(encoder, unpremulBm);

            sk_sp<SkImage> opaqueImage = SkImage::MakeFromEncoded(opaqueData);
            sk_sp<SkImage> premulImage = SkImage::MakeFromEncoded(premulData);
            sk_sp<SkImage> unpremulImage = SkImage::MakeFromEncoded(unpremulData);

            canvas->drawImage(opaqueImage.get(), 0.0f, 0.0f);
            canvas->drawImage(premulImage.get(), 0.0f, 256.0f);
            canvas->drawImage(unpremulImage.get(), 0.0f, 512.0f);

            canvas->translate(256.0f, 0.0f);
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodePlatformGM; )
}
