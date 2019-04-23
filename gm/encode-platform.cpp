/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "src/images/SkImageEncoderPriv.h"
#include "tools/Resources.h"

namespace skiagm {

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
static SkEncodedImageFormat kTypes[] {
        SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG, SkEncodedImageFormat::kGIF,
        SkEncodedImageFormat::kBMP, SkEncodedImageFormat::kICO,
};
#elif defined(SK_BUILD_FOR_WIN)
// Use PNG multiple times because our WIC encoder does not support GIF, BMP, or ICO.
static SkEncodedImageFormat kTypes[] {
        SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG, SkEncodedImageFormat::kPNG,
        SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kPNG,
};
#else
// Use WEBP in place of GIF.  Use PNG two extra times.  We don't support GIF, BMP, or ICO.
static SkEncodedImageFormat kTypes[] {
        SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG, SkEncodedImageFormat::kWEBP,
        SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kPNG,
};
#endif

static sk_sp<SkData> encode_data(SkEncodedImageFormat type, const SkBitmap& bitmap) {
    SkPixmap src;
    if (!bitmap.peekPixels(&src)) {
        return nullptr;
    }
    SkDynamicMemoryWStream buf;
    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        return SkEncodeImageWithCG(&buf, src, type) ? buf.detachAsData() : nullptr;
    #elif defined(SK_BUILD_FOR_WIN)
        return SkEncodeImageWithWIC(&buf, src, type, 100) ? buf.detachAsData() : nullptr;
    #else
        switch (type) {
            case SkEncodedImageFormat::kPNG: {
                bool success = SkPngEncoder::Encode(&buf, src, SkPngEncoder::Options());
                return success ? buf.detachAsData() : nullptr;
            }
            case SkEncodedImageFormat::kJPEG: {
                bool success = SkJpegEncoder::Encode(&buf, src, SkJpegEncoder::Options());
                return success ? buf.detachAsData() : nullptr;
            }
            case SkEncodedImageFormat::kWEBP: {
                bool success = SkWebpEncoder::Encode(&buf, src, SkWebpEncoder::Options());
                return success ? buf.detachAsData() : nullptr;
            }
            default:
                SkASSERT(false);
                return nullptr;
        }
    #endif
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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkBitmap opaqueBm, premulBm, unpremulBm;

        if (!GetResourceAsBitmap("images/mandrill_256.png", &opaqueBm)) {
            *errorMsg = "Could not load images/mandrill_256.png.png. "
                        "Did you forget to set the resourcePath?";
            return DrawResult::kFail;
        }
        SkBitmap tmp;
        if (!GetResourceAsBitmap("images/yellow_rose.png", &tmp)) {
            *errorMsg = "Could not load images/yellow_rose.png. "
                        "Did you forget to set the resourcePath?";
            return DrawResult::kFail;
        }
        tmp.extractSubset(&premulBm, SkIRect::MakeWH(256, 256));
        tmp.reset();
        unpremulBm.allocPixels(premulBm.info().makeAlphaType(kUnpremul_SkAlphaType));
        SkAssertResult(premulBm.readPixels(unpremulBm.pixmap()));

        for (SkEncodedImageFormat type : kTypes) {
            auto opaqueImage = SkImage::MakeFromEncoded(encode_data(type, opaqueBm));
            auto premulImage = SkImage::MakeFromEncoded(encode_data(type, premulBm));
            auto unpremulImage = SkImage::MakeFromEncoded(encode_data(type, unpremulBm));

            canvas->drawImage(opaqueImage.get(), 0.0f, 0.0f);
            canvas->drawImage(premulImage.get(), 0.0f, 256.0f);
            canvas->drawImage(unpremulImage.get(), 0.0f, 512.0f);

            canvas->translate(256.0f, 0.0f);
        }
        return DrawResult::kOk;
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new EncodePlatformGM; )
}
