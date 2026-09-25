/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/utils/SkOSPath.h"
#include "tests/CodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
#include "include/codec/SkPngDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "include/codec/SkPngRustDecoder.h"
#endif

DEF_TEST(BadImage, reporter) {
    static constexpr auto badImages = std::to_array<const char*>({
            "sigabort_favicon.ico",
            "sigsegv_favicon.ico",
            "sigsegv_favicon_2.ico",
            "ico_leak01.ico",
            "ico_fuzz0.ico",
            "ico_fuzz1.ico",
            "skbug3442.webp",
            "skbug3429.webp",
            "b38116746.ico",
            "skbug5883.gif",
    });

    const char* badImagesFolder = "invalid_images";

    for (size_t i = 0; i < std::size(badImages); ++i) {
        SkString resourcePath = SkOSPath::Join(badImagesFolder, badImages[i]);
        std::unique_ptr<SkStream> stream(GetResourceAsStream(resourcePath.c_str()));
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));

        // These images are corrupt.  It's not important whether we succeed/fail in codec
        // creation or decoding.  We just want to make sure that we don't crash.
        if (codec) {
            SkBitmap bm;
            bm.allocPixels(codec->getInfo());
            codec->getPixels(codec->getInfo(), bm.getPixels(),
                    bm.rowBytes());
        }
    }
}

#if defined(SK_CODEC_DECODES_ICO)
static void test_bad_png_in_ico_does_not_fallback(skiatest::Reporter* r,
                                                  const SkCodecs::Decoder& pngDecoder) {
    ScopedCodecDecoders scopedDecoders;

    // 1. When the PNG decoder is registered, decoding an ICO with a malformed PNG fails cleanly
    // without falling back to libpng.
    SkCodecs::Register(pngDecoder);
    constexpr uint8_t kMalformedPng[] = {
            0x89,
            'P',
            'N',
            'G',
            '\r',
            '\n',
            0x1A,
            '\n',
            0x00,
            0x00,
            0x00,
            0x00,
    };
    sk_sp<SkData> badIcoData = make_ico_with_png(kMalformedPng, sizeof(kMalformedPng));
    REPORTER_ASSERT(r, badIcoData);
    std::unique_ptr<SkCodec> badCodec = SkCodec::MakeFromStream(SkMemoryStream::Make(badIcoData));
    REPORTER_ASSERT(
            r, badCodec == nullptr, "Malformed PNG in ICO should fail without fallback to libpng");

    // 2. Verify that when a registered PNG decoder rejects a valid PNG that libpng could decode,
    // SkIcoCodec does not fall back to libpng (or attempt a redundant second decode).
    SkCodecs::Register({
            "png",
            pngDecoder.isFormat,
            [](std::unique_ptr<SkStream>, SkCodec::Result* result, SkCodecs::DecodeContext)
                    -> std::unique_ptr<SkCodec> {
                if (result) {
                    *result = SkCodec::kErrorInInput;
                }
                return nullptr;
            },
    });
    sk_sp<SkData> icoWithValidPng = make_ico_from_png_resource(r, "images/mandrill_128.png");
    std::unique_ptr<SkCodec> rejectedCodec =
            SkCodec::MakeFromStream(SkMemoryStream::Make(icoWithValidPng));
    REPORTER_ASSERT(r,
                    rejectedCodec == nullptr,
                    "SkIcoCodec must not fall back to libpng when a PNG decoder is registered");
}

#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
DEF_SERIAL_TEST(Ico_malformedPngWithRustDecoderDoesNotFallback, r) {
    test_bad_png_in_ico_does_not_fallback(r, SkPngRustDecoder::Decoder());
}
#endif

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
DEF_SERIAL_TEST(Ico_malformedPngWithLibpngNoRedundantFallback, r) {
    test_bad_png_in_ico_does_not_fallback(r, SkPngDecoder::Decoder());
}
#endif
#endif  // SK_CODEC_DECODES_ICO
