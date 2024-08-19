/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#ifdef SK_ENABLE_NDK_IMAGES
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/ports/SkImageGeneratorNDK.h"

#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <vector>

static std::unique_ptr<SkImageGenerator> make_generator(const char* path, skiatest::Reporter* r) {
    auto data = GetResourceAsData(path);
    if (data) {
        auto gen = SkImageGeneratorNDK::MakeFromEncodedNDK(std::move(data));
        if (gen) {
            return gen;
        }
        ERRORF(r, "Failed to create NDK generator from %s\n", path);
    } else {
        // Silently fail so developers can skip using --resources
    }
    return nullptr;
}

DEF_TEST(NdkDecode, r) {
    static const struct {
        const char* fPath;
        SkISize     fSize;
    } recs[] = {
        {"images/CMYK.jpg", {642, 516}},
        {"images/arrow.png", {187, 312}},
        {"images/baby_tux.webp", {386, 395}},
        {"images/color_wheel.gif", {128, 128}},
        {"images/rle.bmp", {320, 240}},
        {"images/color_wheel.ico", {128, 128}},
        {"images/google_chrome.ico", {256, 256}},
        {"images/mandrill.wbmp", {512, 512}},
    };
    for (auto& rec : recs) {
        auto gen = make_generator(rec.fPath, r);
        if (!gen) continue;

        const auto& info = gen->getInfo();
        REPORTER_ASSERT(r, info.dimensions() == rec.fSize);

        SkBitmap bm;
        bm.allocPixels(info);
        REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));

        REPORTER_ASSERT(r, info.alphaType() != kUnpremul_SkAlphaType);
        auto unpremulInfo = info.makeAlphaType(kUnpremul_SkAlphaType);
        bm.allocPixels(unpremulInfo);
        REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));
    }
}

DEF_TEST(NdkDecode_nullData, r) {
    auto gen = SkImageGeneratorNDK::MakeFromEncodedNDK(nullptr);
    REPORTER_ASSERT(r, !gen);
}

static constexpr skcms_TransferFunction k2Dot6 = {2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static constexpr skcms_Matrix3x3 kDCIP3 = {{
        {0.486143, 0.323835, 0.154234},
        {0.226676, 0.710327, 0.0629966},
        {0.000800549, 0.0432385, 0.78275},
}};

DEF_TEST(NdkDecode_reportedColorSpace, r) {
    for (sk_sp<SkColorSpace> cs : {
        sk_sp<SkColorSpace>(nullptr),
        SkColorSpace::MakeSRGB(),
        SkColorSpace::MakeSRGBLinear(),
        SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB),
        SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020),
        SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3),
        SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB),
        SkColorSpace::MakeRGB(k2Dot6, kDCIP3),
    }) {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_F16_SkColorType, kOpaque_SkAlphaType, cs));
        bm.eraseColor(SK_ColorBLUE);

        for (auto format : { SkEncodedImageFormat::kPNG,
                             SkEncodedImageFormat::kJPEG,
                             SkEncodedImageFormat::kWEBP }) {
            SkDynamicMemoryWStream stream;
            if (format == SkEncodedImageFormat::kJPEG) {
                SkJpegEncoder::Options opts;
                opts.fQuality = 80;
                REPORTER_ASSERT(r, SkJpegEncoder::Encode(&stream, bm.pixmap(), opts));
            } else if (format == SkEncodedImageFormat::kPNG) {
                REPORTER_ASSERT(r, SkPngEncoder::Encode(&stream, bm.pixmap(), {}));
            } else {
                SkWebpEncoder::Options opts;
                opts.fQuality = 80;
                REPORTER_ASSERT(r, SkWebpEncoder::Encode(&stream, bm.pixmap(), opts));
            }

            auto gen = SkImageGeneratorNDK::MakeFromEncodedNDK(stream.detachAsData());
            if (!gen) {
                ERRORF(r, "Failed to encode!");
                return;
            }

            if (!cs) cs = SkColorSpace::MakeSRGB();
            REPORTER_ASSERT(r, SkColorSpace::Equals(gen->getInfo().colorSpace(), cs.get()));
        }
    }
}

DEF_TEST(NdkDecode_ColorSpace, r) {
    for (const char* path: {
        "images/CMYK.jpg",
        "images/arrow.png",
        "images/baby_tux.webp",
        "images/color_wheel.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        for (sk_sp<SkColorSpace> cs : {
            sk_sp<SkColorSpace>(nullptr),
            SkColorSpace::MakeSRGB(),
            SkColorSpace::MakeSRGBLinear(),
            SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB),
            SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020),
            SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3),
            SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB),
            SkColorSpace::MakeRGB(k2Dot6, kDCIP3),
        }) {
            auto info = gen->getInfo().makeColorSpace(cs);

            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));
        }

        std::vector<sk_sp<SkColorSpace>> unsupportedCs;
        for (auto gamut : { SkNamedGamut::kSRGB, SkNamedGamut::kAdobeRGB, SkNamedGamut::kDisplayP3,
                            SkNamedGamut::kRec2020, SkNamedGamut::kXYZ }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ, gamut));
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::kHLG, gamut));
            unsupportedCs.push_back(SkColorSpace::MakeRGB(k2Dot6, gamut));
        }

        for (auto gamut : { SkNamedGamut::kSRGB, SkNamedGamut::kDisplayP3,
                            SkNamedGamut::kRec2020, SkNamedGamut::kXYZ }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, gamut));
        }

        for (auto gamut : { SkNamedGamut::kAdobeRGB, SkNamedGamut::kDisplayP3,
                            SkNamedGamut::kXYZ }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, gamut));
        }

        for (auto gamut : { SkNamedGamut::kAdobeRGB, SkNamedGamut::kDisplayP3,
                            SkNamedGamut::kRec2020, SkNamedGamut::kXYZ }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, gamut));
        }

        for (auto gamut : { SkNamedGamut::kAdobeRGB,
                            SkNamedGamut::kRec2020, SkNamedGamut::kXYZ }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, gamut));
        }

        for (auto fn : { SkNamedTransferFn::kSRGB, SkNamedTransferFn::k2Dot2,
                         SkNamedTransferFn::kLinear, SkNamedTransferFn::kRec2020 }) {
            unsupportedCs.push_back(SkColorSpace::MakeRGB(fn, kDCIP3));
        }

        for (auto unsupported : unsupportedCs) {
            auto info = gen->getInfo().makeColorSpace(unsupported);

            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r, !gen->getPixels(bm.pixmap()));
        }
    }
}

DEF_TEST(NdkDecode_reuseNoColorSpace, r) {
    static const struct {
        const char*         fPath;
        sk_sp<SkColorSpace> fCorrectedColorSpace;
        bool                fIsOpaque;
    } recs[] = {
        // AImageDecoder defaults to ADATASPACE_UNKNOWN for this image.
        {"images/wide_gamut_yellow_224_224_64.jpeg", SkColorSpace::MakeSRGB(), true},
        // This image is SRGB, so convert to a different color space.
        {"images/example_1.png", SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,
                                                       SkNamedGamut::kAdobeRGB), false},
    };
    for (auto& rec : recs) {
        auto gen = make_generator(rec.fPath, r);
        if (!gen) continue;

        REPORTER_ASSERT(r, gen->getInfo().colorSpace()->isSRGB());
        REPORTER_ASSERT(r, gen->getInfo().isOpaque() == rec.fIsOpaque);

        auto noColorCorrection = gen->getInfo().makeColorSpace(nullptr);
        if (rec.fIsOpaque) {
            // Use something other than the default color type to verify that the modified color
            // type is used even when the color space is reset.
            noColorCorrection = noColorCorrection.makeColorType(kRGB_565_SkColorType);
        }

        SkBitmap orig;
        orig.allocPixels(noColorCorrection);
        REPORTER_ASSERT(r, gen->getPixels(orig.pixmap()));

        SkBitmap corrected;
        corrected.allocPixels(noColorCorrection.makeColorSpace(rec.fCorrectedColorSpace));
        REPORTER_ASSERT(r, gen->getPixels(corrected.pixmap()));

        REPORTER_ASSERT(r, !ToolUtils::equal_pixels(orig, corrected));

        SkBitmap reuse;
        reuse.allocPixels(noColorCorrection);
        REPORTER_ASSERT(r, gen->getPixels(reuse.pixmap()));

        REPORTER_ASSERT(r, ToolUtils::equal_pixels(orig, reuse));
    }
}

// The NDK supports scaling up to arbitrary dimensions. Skia forces clients to do this in a
// separate step, so the client is in charge of how to do the upscale.
DEF_TEST(NdkDecode_noUpscale, r) {
    for (const char* path: {
        "images/CMYK.jpg",
        "images/arrow.png",
        "images/baby_tux.webp",
        "images/color_wheel.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        const auto actualDimensions = gen->getInfo().dimensions();
        const int width = actualDimensions.width();
        const int height = actualDimensions.height();
        for (SkISize dims : {
            SkISize{width*2, height*2},
            SkISize{width + 1, height + 1},
        }) {
            auto info = gen->getInfo().makeDimensions(dims);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r, !gen->getPixels(bm.pixmap()));
        }
    }
}

// libwebp supports downscaling to an arbitrary scale factor, and this is supported by the NDK.
DEF_TEST(NdkDecode_webpArbitraryDownscale, r) {
    for (const char* path: {
        "images/baby_tux.webp",
        "images/yellow_rose.webp",
        "images/webp-color-profile-lossless.webp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        const auto actualDimensions = gen->getInfo().dimensions();
        const int width = actualDimensions.width();
        const int height = actualDimensions.height();
        for (SkISize dims : {
            SkISize{width/2, height/2},
            SkISize{width/4, height/4},
            SkISize{width/7, height/7},
            SkISize{width - 1, height - 1},
            SkISize{1, 1},
            SkISize{5, 20}
        }) {
            auto info = gen->getInfo().makeDimensions(dims);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));

            REPORTER_ASSERT(r, info.alphaType() != kUnpremul_SkAlphaType);
            auto unpremulInfo = info.makeAlphaType(kUnpremul_SkAlphaType);
            bm.allocPixels(unpremulInfo);
            REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));
        }
    }
}

// libjpeg-turbo supports downscaling to some scale factors.
DEF_TEST(NdkDecode_jpegDownscale, r) {
    static const struct {
        const char* fPath;
        SkISize     fSupportedSizes[4];
    } recs[] = {
        {"images/CMYK.jpg", {{642,516},{321,258},{161,129},{81,65}}},
        {"images/dog.jpg", {{180,180},{90,90},{45,45},{23,23}}},
        {"images/grayscale.jpg", {{128,128},{64,64},{32,32},{16,16}}},
        {"images/brickwork-texture.jpg", {{512,512},{256,256},{128,128},{64,64}}},
        {"images/mandrill_h2v1.jpg", {{512,512},{256,256},{128,128},{64,64}}},
        {"images/ducky.jpg", {{489,537},{245,269},{123,135},{62,68}}},
    };
    for (auto& rec : recs) {
        auto gen = make_generator(rec.fPath, r);
        if (!gen) continue;

        for (SkISize dims : rec.fSupportedSizes) {
            auto info = gen->getInfo().makeDimensions(dims);
            SkBitmap bm;
            bm.allocPixels(info);
            if (!gen->getPixels(bm.pixmap())) {
                ERRORF(r, "failed to decode %s to {%i,%i}\n", rec.fPath, dims.width(),
                          dims.height());
            }

            REPORTER_ASSERT(r, info.alphaType() != kUnpremul_SkAlphaType);
            auto unpremulInfo = info.makeAlphaType(kUnpremul_SkAlphaType);
            bm.allocPixels(unpremulInfo);
            REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));
        }
    }
}

DEF_TEST(NdkDecode_reuseJpeg, r) {
    auto gen = make_generator("images/CMYK.jpg", r);
    if (!gen) return;

    SkImageInfo info = gen->getInfo();
    SkBitmap orig;
    orig.allocPixels(info);
    REPORTER_ASSERT(r, gen->getPixels(orig.pixmap()));

    info = info.makeWH(321, 258);
    SkBitmap downscaled;
    downscaled.allocPixels(info);
    REPORTER_ASSERT(r, gen->getPixels(downscaled.pixmap()));

    SkBitmap reuse;
    reuse.allocPixels(gen->getInfo());
    REPORTER_ASSERT(r, gen->getPixels(reuse.pixmap()));

    REPORTER_ASSERT(r, ToolUtils::equal_pixels(orig, reuse));
}

// The NDK supports scaling down to arbitrary dimensions. Skia forces clients to do this in a
// separate step, so the client is in charge of how to do the downscale.
DEF_TEST(NdkDecode_noDownscale, r) {
    for (const char* path: {
        "images/arrow.png",
        "images/color_wheel.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        const auto actualDimensions = gen->getInfo().dimensions();
        const int width = actualDimensions.width();
        const int height = actualDimensions.height();
        for (SkISize dims : {
            SkISize{width/2, height/2},
            SkISize{width/3, height/3},
            SkISize{width/4, height/4},
            SkISize{width/8, height/8},
            SkISize{width - 1, height - 1},
        }) {
            auto info = gen->getInfo().makeDimensions(dims);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r, !gen->getPixels(bm.pixmap()));
        }
    }
}

DEF_TEST(NdkDecode_Gray8, r) {
    static const struct {
        const char* fPath;
        bool        fGrayscale;
    } recs[] = {
        {"images/CMYK.jpg", false},
        {"images/arrow.png", false},
        {"images/baby_tux.webp", false},
        {"images/color_wheel.gif", false},
        {"images/rle.bmp", false},
        {"images/color_wheel.ico", false},
        {"images/google_chrome.ico", false},
        {"images/mandrill.wbmp", true},
        {"images/grayscale.jpg", true},
        {"images/grayscale.png", true},
    };
    for (auto& rec : recs) {
        auto gen = make_generator(rec.fPath, r);
        if (!gen) continue;

        SkImageInfo info = gen->getInfo();
        if (rec.fGrayscale) {
            REPORTER_ASSERT(r, info.colorType() == kGray_8_SkColorType);
            REPORTER_ASSERT(r, info.alphaType() == kOpaque_SkAlphaType);
        } else {
            info = info.makeColorType(kGray_8_SkColorType);
        }
        SkBitmap bm;
        bm.allocPixels(info);
        bool success = gen->getPixels(bm.pixmap());
        if (success != rec.fGrayscale) {
            ERRORF(r, "Expected decoding %s to Gray8 to %s. Actual: %s\n", rec.fPath,
                      (rec.fGrayscale ? "succeed" : "fail"), (success ? "succeed" : "fail"));
        }
    }
}

DEF_TEST(NdkDecode_Opaque_and_565, r) {
    for (const char* path: {
        "images/CMYK.jpg",
        "images/dog.jpg",
        "images/ducky.jpg",
        "images/arrow.png",
        "images/example_1.png",
        "images/explosion_sprites.png",
        "images/lut_identity.png",
        "images/grayscale.png",
        "images/baby_tux.webp",
        "images/yellow_rose.webp",
        "images/webp-color-profile-lossless.webp",
        "images/colorTables.gif",
        "images/color_wheel.gif",
        "images/flightAnim.gif",
        "images/randPixels.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        auto info = gen->getInfo().makeAlphaType(kOpaque_SkAlphaType);
        SkBitmap bm;
        bm.allocPixels(info);
        bool success = gen->getPixels(bm.pixmap());
        REPORTER_ASSERT(r, success == gen->getInfo().isOpaque());

        info = info.makeColorType(kRGB_565_SkColorType);
        bm.allocPixels(info);
        success = gen->getPixels(bm.pixmap());
        REPORTER_ASSERT(r, success == gen->getInfo().isOpaque());
    }
}

DEF_TEST(NdkDecode_AlwaysSupportedColorTypes, r) {
    for (const char* path: {
        "images/CMYK.jpg",
        "images/dog.jpg",
        "images/ducky.jpg",
        "images/arrow.png",
        "images/example_1.png",
        "images/explosion_sprites.png",
        "images/lut_identity.png",
        "images/grayscale.png",
        "images/baby_tux.webp",
        "images/yellow_rose.webp",
        "images/webp-color-profile-lossless.webp",
        "images/colorTables.gif",
        "images/color_wheel.gif",
        "images/flightAnim.gif",
        "images/randPixels.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        auto info = gen->getInfo().makeColorType(kRGBA_F16_SkColorType);
        SkBitmap bm;
        bm.allocPixels(info);
        REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));

        // This also tests that we can reuse the same generator for a different
        // color type.
        info = info.makeColorType(kRGBA_8888_SkColorType);
        bm.allocPixels(info);
        REPORTER_ASSERT(r, gen->getPixels(bm.pixmap()));
    }
}

DEF_TEST(NdkDecode_UnsupportedColorTypes, r) {
    for (const char* path: {
        "images/CMYK.jpg",
        "images/dog.jpg",
        "images/ducky.jpg",
        "images/arrow.png",
        "images/example_1.png",
        "images/explosion_sprites.png",
        "images/lut_identity.png",
        "images/grayscale.png",
        "images/baby_tux.webp",
        "images/yellow_rose.webp",
        "images/webp-color-profile-lossless.webp",
        "images/colorTables.gif",
        "images/color_wheel.gif",
        "images/flightAnim.gif",
        "images/randPixels.gif",
        "images/rle.bmp",
        "images/color_wheel.ico",
        "images/google_chrome.ico",
        "images/mandrill.wbmp",
    }) {
        auto gen = make_generator(path, r);
        if (!gen) continue;

        for (SkColorType ct : {
            kUnknown_SkColorType,
            kAlpha_8_SkColorType,
            kARGB_4444_SkColorType,
            kRGB_888x_SkColorType,
            kBGRA_8888_SkColorType,
            kRGBA_1010102_SkColorType,
            kBGRA_1010102_SkColorType,
            kRGB_101010x_SkColorType,
            kBGR_101010x_SkColorType,
            kRGBA_F16Norm_SkColorType,
            kRGBA_F32_SkColorType,
            kR8G8_unorm_SkColorType,
            kA16_float_SkColorType,
            kR16G16_float_SkColorType,
            kA16_unorm_SkColorType,
            kR16G16_unorm_SkColorType,
            kR16G16B16A16_unorm_SkColorType,
        }) {
            auto info = gen->getInfo().makeColorType(ct);
            SkBitmap bm;
            bm.allocPixels(info);
            if (gen->getPixels(bm.pixmap())) {
                ERRORF(r, "Expected decoding %s to %i to fail!", path, ct);
            }
        }
    }
}
#endif // SK_ENABLE_NDK_IMAGES
