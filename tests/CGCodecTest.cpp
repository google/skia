/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include "include/core/SkData.h"
#include "include/codec/SkCodec.h"
#include "include/ports/SkImageGeneratorCG.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/ports/SkCGCodec.h"
#include "tests/Test.h"
#include "tools/Resources.h"

static std::unique_ptr<SkCodec> make_codec(const char* path, skiatest::Reporter* r) {
    auto data = GetResourceAsData(path);
    if (data) {
        auto codec = SkCGCodec::MakeFromEncoded(std::move(data));
        if (codec) {
            return codec;
        }
        ERRORF(r, "Failed to create CG codec from %s\n", path);
    } else {
        // Silently fail so developers can skip using --resources
    }
    return nullptr;
}

DEF_TEST(CGCodec_encodedFormat, r) {
    const struct {
        const char*          path;
        SkEncodedImageFormat format;
    } recs[] = {
        { "images/arrow.png",             SkEncodedImageFormat::kPNG  },
        { "images/CMYK.jpg",              SkEncodedImageFormat::kJPEG },
        { "images/box.gif",               SkEncodedImageFormat::kGIF  },
        { "images/google_chrome.ico",     SkEncodedImageFormat::kICO  },
        { "images/randPixels.bmp",        SkEncodedImageFormat::kBMP  },
        { "images/heifwriter_input.heic", SkEncodedImageFormat::kHEIF },
    };
    for (const auto& rec : recs) {
        auto codec = make_codec(rec.path, r);
        if (!codec) continue;

        REPORTER_ASSERT(r, rec.format == codec->getEncodedFormat());
    }
}

DEF_TEST(CGCodec_nullData, r) {
    REPORTER_ASSERT(r, !SkImageGeneratorCG::MakeFromEncodedCG(nullptr));
}

DEF_TEST(CGCodec_unsupported, r) {
    for (const char* path : { "images/mandrill.wbmp",
                              "images/dng_with_preview.dng",
                              "images/flower-bc1.dds",
                              "images/flower-etc1.ktx",
                              "Cowboy.svg",
                              "skottie/skottie_sample_1.json" }) {
        auto data = GetResourceAsData(path);
        if (!data) {
            return;
        }

        auto gen = SkImageGeneratorCG::MakeFromEncodedCG(std::move(data));
        REPORTER_ASSERT(r, !gen);
    }
}

DEF_TEST(CGCodec_getFrameInfo, r) {
    const char* path = "images/flightAnim.gif";
    auto codec = make_codec(path, r);
    if (codec) return;

    const int frameCount = codec->getFrameCount();
    SkCodec::FrameInfo frameInfo;
    for (int i : { -1, -10, frameCount, frameCount + 7 }) {
        REPORTER_ASSERT(r, !codec->getFrameInfo(i, &frameInfo));
        REPORTER_ASSERT(r, !codec->getFrameInfo(i, nullptr));
    }

    for (int i = 0; i < frameCount; i++) {
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));
        REPORTER_ASSERT(r, codec->getFrameInfo(i, nullptr));
    }
}
#endif // SK_BUILD_FOR_(MAC || IOS)
