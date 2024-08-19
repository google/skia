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
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/private/base/SkMalloc.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <stdint.h>
#include <vector>

static const char* kPng          = "png";
static const char* kJpeg         = "jpeg";
static const char* kWebpLossless = "webp_lossless";
static const char* kWebpLossy    = "webp_lossy";

namespace {
static const struct {
    SkEncodedImageFormat format;
    int                  quality;
    const char*          name;
} gRecs[] = {
    { SkEncodedImageFormat::kPNG,  100, kPng},
    { SkEncodedImageFormat::kJPEG, 100, kJpeg},
    { SkEncodedImageFormat::kWEBP, 100, kWebpLossless},
    { SkEncodedImageFormat::kWEBP,  80, kWebpLossy},
};
}

static sk_sp<SkData> encode_ndk(const SkPixmap& pmap, SkEncodedImageFormat format, int quality) {
    SkDynamicMemoryWStream stream;
    SkDynamicMemoryWStream buf;
    switch (format) {
        case SkEncodedImageFormat::kPNG: {
            bool success = SkPngEncoder::Encode(&buf, pmap, {});
            return success ? buf.detachAsData() : nullptr;
        }
        case SkEncodedImageFormat::kJPEG: {
            SkJpegEncoder::Options opts;
            opts.fQuality = quality;
            bool success = SkJpegEncoder::Encode(&buf, pmap, opts);
            return success ? buf.detachAsData() : nullptr;
        }
        case SkEncodedImageFormat::kWEBP: {
            SkWebpEncoder::Options opts;
            opts.fQuality = quality;
            bool success = SkWebpEncoder::Encode(&buf, pmap, opts);
            return success ? buf.detachAsData() : nullptr;
        }
        default:
            SkUNREACHABLE;
    }
}

DEF_TEST(NdkEncode, r) {
    for (auto ct : { kRGBA_8888_SkColorType,
                     kRGB_565_SkColorType,
                     kRGBA_F16_SkColorType }) {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(10, 10, ct, kOpaque_SkAlphaType));
        bm.eraseColor(SK_ColorBLUE);
        for (const auto& rec : gRecs) {
            auto encoded = encode_ndk(bm.pixmap(), rec.format, rec.quality);
            if (!encoded) {
                ERRORF(r, "Failed to encode %s to %s\n", ToolUtils::colortype_name(ct), rec.name);
                continue;
            }
            auto gen = SkImageGenerators::MakeFromEncoded(std::move(encoded));
            if (!gen) {
                ERRORF(r, "Failed to decode from %s as %s\n", ToolUtils::colortype_name(ct),
                       rec.name);
                continue;
            }

            if (rec.name == kPng && bm.colorType() == kRGB_565_SkColorType) {
                REPORTER_ASSERT(r, gen->getInfo().colorType() == kRGB_565_SkColorType);
            } else {
                REPORTER_ASSERT(r, gen->getInfo().colorType() == kN32_SkColorType);
            }

            SkBitmap bm2;
            bm2.allocPixels(bm.info());
            REPORTER_ASSERT(r, gen->getPixels(bm2.pixmap()));

            for (int x = 0; x < bm.width();  x++)
            for (int y = 0; y < bm.height(); y++) {
                SkColor orig   = bm .getColor(x, y);
                SkColor actual = bm2.getColor(x, y);

                REPORTER_ASSERT(r, SkColorGetA(orig) == SkColorGetA(actual));
                REPORTER_ASSERT(r, SkColorGetA(orig) == 0xFF);

                if (rec.name == kPng || rec.name == kWebpLossless) {
                    REPORTER_ASSERT(r, orig == actual);
                } else {
                    int diffR = std::abs((int) SkColorGetR(orig) - (int) SkColorGetR(actual));
                    int diffG = std::abs((int) SkColorGetG(orig) - (int) SkColorGetG(actual));
                    int diffB = std::abs((int) SkColorGetB(orig) - (int) SkColorGetB(actual));
                    REPORTER_ASSERT(r, diffR <= 2 && diffG <= 1 && diffB <= 1);
                }
            }
        }
    }
}

DEF_TEST(NdkEncode_unsupportedFormats, r) {
    for (auto ct : { kRGBA_8888_SkColorType,
                     kRGB_565_SkColorType,
                     kRGBA_F16_SkColorType }) {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(10, 10, ct, kOpaque_SkAlphaType));
        bm.eraseColor(SK_ColorBLUE);
        for (auto format : { SkEncodedImageFormat::kBMP,
                             SkEncodedImageFormat::kGIF,
                             SkEncodedImageFormat::kICO,
                             SkEncodedImageFormat::kWBMP,
                             SkEncodedImageFormat::kPKM,
                             SkEncodedImageFormat::kKTX,
                             SkEncodedImageFormat::kASTC,
                             SkEncodedImageFormat::kDNG,
                             SkEncodedImageFormat::kHEIF }) {
            REPORTER_ASSERT(r, !encode_ndk(bm.pixmap(), format, 100));
        }
    }
}

DEF_TEST(NdkEncode_badQuality, r) {
    for (auto ct : { kRGBA_8888_SkColorType,
                     kRGB_565_SkColorType,
                     kRGBA_F16_SkColorType }) {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(10, 10, ct, kOpaque_SkAlphaType));
        bm.eraseColor(SK_ColorBLUE);
        for (auto format : { SkEncodedImageFormat::kJPEG,
                             SkEncodedImageFormat::kPNG,
                             SkEncodedImageFormat::kWEBP }) {
            for (int quality : {-1, -100, 101, 200}) {
                REPORTER_ASSERT(r, !encode_ndk(bm.pixmap(), format, quality));
            }
        }
    }
}

DEF_TEST(NdkEncode_nullPixels, r) {
    for (auto info : { SkImageInfo::MakeUnknown(),
                       SkImageInfo::MakeN32Premul(10, 10),
                       SkImageInfo::MakeN32Premul(0, 0)}) {
        SkPixmap pm(info, nullptr, info.minRowBytes());
        for (const auto& rec : gRecs) {
            REPORTER_ASSERT(r, !encode_ndk(pm, rec.format, rec.quality));
        }
    }
}

DEF_TEST(NdkEncode_badInfo, r) {
    // Allocate an arbitrary amount of memory. These infos don't have a natural
    // amount to allocate, and the encoder shouldn't touch the memory anyway.
    // But this allows us to verify that the bad info fails, even when the pixel
    // pointer is not null.
    void* pixels = sk_malloc_throw(1024);
    std::vector<SkPixmap> pixmaps{ SkPixmap(SkImageInfo::MakeN32Premul(-10, 10), pixels, 1000),
                                   SkPixmap(SkImageInfo::MakeN32Premul(10, -10), pixels, 200),
                                   SkPixmap(SkImageInfo::MakeN32Premul(10,  10), pixels, 20),
                                   SkPixmap(SkImageInfo::MakeN32Premul(10,  10), pixels, 41),
                                   SkPixmap(SkImageInfo::MakeN32Premul(10,  10), pixels, 0),
                                   SkPixmap(SkImageInfo::MakeN32Premul( 0,   0), pixels, 40)};
    if (sizeof(size_t) > sizeof(uint32_t)) {
        pixmaps.emplace_back(SkImageInfo::MakeN32Premul(10, 10),  pixels,
                             static_cast<size_t>(UINT32_MAX) + 1);
    }
    for (const auto& pm : pixmaps) {
        for (const auto& rec : gRecs) {
            REPORTER_ASSERT(r, !encode_ndk(pm, rec.format, rec.quality));
        }
    }
    free(pixels);
}

DEF_TEST(NdkEncode_unsupportedColorTypes, r) {
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
        kGray_8_SkColorType,
        kRGBA_F16Norm_SkColorType,
        kRGBA_F32_SkColorType,
        kR8G8_unorm_SkColorType,
        kA16_float_SkColorType,
        kR16G16_float_SkColorType,
        kA16_unorm_SkColorType,
        kR16G16_unorm_SkColorType,
        kR16G16B16A16_unorm_SkColorType,
    }) {
        auto info = SkImageInfo::Make(7, 13, ct, kOpaque_SkAlphaType, SkColorSpace::MakeSRGB());
        SkBitmap bm;
        bm.allocPixels(info);
        bm.eraseColor(SK_ColorGREEN);
        for (const auto& rec : gRecs) {
            REPORTER_ASSERT(r, !encode_ndk(bm.pixmap(), rec.format, rec.quality));
        }
        if (!SkColorTypeIsAlwaysOpaque(ct)) {
            for (auto at : { kPremul_SkAlphaType, kUnpremul_SkAlphaType}) {
                info = info.makeAlphaType(at);
                bm.allocPixels(info);
                bm.eraseARGB(0x7F, 0xFF, 0xFF, 0xFF);
            }
            for (const auto& rec : gRecs) {
                REPORTER_ASSERT(r, !encode_ndk(bm.pixmap(), rec.format, rec.quality));
            }
        }
    }
}

DEF_TEST(NdkEncode_unsupportedAlphaTypes, r) {
    for (auto ct : { kRGBA_8888_SkColorType,
                     kRGB_565_SkColorType,
                     kRGBA_F16_SkColorType }) {
        for (auto at : { kUnknown_SkAlphaType, (SkAlphaType) -1}) {
            auto info = SkImageInfo::Make(10, 10, ct, at);
            size_t rowBytes = info.minRowBytes();
            void* pixels = sk_malloc_throw(info.computeByteSize(rowBytes));
            SkPixmap pm(info, pixels, rowBytes);
            for (const auto& rec : gRecs) {
                REPORTER_ASSERT(r, !encode_ndk(pm, rec.format, rec.quality));
            }
            free(pixels);
        }
    }
}

static constexpr skcms_TransferFunction k2Dot6 = {2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static constexpr skcms_Matrix3x3 kDCIP3 = {{
        {0.486143, 0.323835, 0.154234},
        {0.226676, 0.710327, 0.0629966},
        {0.000800549, 0.0432385, 0.78275},
}};


static bool nearly_equal(float a, float b) {
    return fabs(a - b) < .002f;
}

static bool nearly_equal(const skcms_TransferFunction& x, const skcms_TransferFunction& y) {
    return nearly_equal(x.g, y.g)
        && nearly_equal(x.a, y.a)
        && nearly_equal(x.b, y.b)
        && nearly_equal(x.c, y.c)
        && nearly_equal(x.d, y.d)
        && nearly_equal(x.e, y.e)
        && nearly_equal(x.f, y.f);
}

static bool nearly_equal(const skcms_Matrix3x3& a, const skcms_Matrix3x3& b) {
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        if (!nearly_equal(a.vals[i][j], b.vals[i][j])) return false;
    }
    return true;
}

static bool nearly_equal(SkColorSpace* a, SkColorSpace* b) {
    skcms_TransferFunction fnA,     fnB;
    skcms_Matrix3x3        gamutA,  gamutB;
    return a && b && a->isNumericalTransferFn(&fnA) && a->toXYZD50(&gamutA)
                  && b->isNumericalTransferFn(&fnB) && b->toXYZD50(&gamutB)
             && nearly_equal(fnA, fnB) && nearly_equal(gamutA, gamutB);
}

DEF_TEST(NdkEncode_ColorSpace, r) {
    const struct {
        sk_sp<SkColorSpace> cs;
        const char*         name;
    } colorSpaces[] = {
        { sk_sp<SkColorSpace>(nullptr),                                                 "null"    },
        { SkColorSpace::MakeSRGB(),                                                     "srgb"    },
        { SkColorSpace::MakeSRGBLinear(),                                            "srgb-linear"},
        { SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB),      "bt709"   },
        { SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020),   "rec2020" },
        { SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,    SkNamedGamut::kDisplayP3), "p3"      },
        { SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,   SkNamedGamut::kAdobeRGB),  "adobeRGB"},
        { SkColorSpace::MakeRGB(k2Dot6,                      kDCIP3),                   "dci-p3"  },
    };
    for (const auto& colorSpace : colorSpaces) {
        for (auto ct : { kRGBA_8888_SkColorType, kRGB_565_SkColorType, kRGBA_F16_SkColorType }) {
            SkBitmap bm;
            bm.allocPixels(SkImageInfo::Make(10, 10, ct, kOpaque_SkAlphaType, colorSpace.cs));
            bm.eraseColor(SK_ColorRED);

            for (const auto& rec : gRecs) {
                auto encoded = encode_ndk(bm.pixmap(), rec.format, rec.quality);
                REPORTER_ASSERT(r, encoded);
                auto gen = SkImageGenerators::MakeFromEncoded(std::move(encoded));
                REPORTER_ASSERT(r, gen);

                auto  expected = colorSpace.cs ? colorSpace.cs : SkColorSpace::MakeSRGB();
                auto* actual   = gen->getInfo().colorSpace();
                if (!nearly_equal(actual, expected.get())) {
                    const char* name = "unknown";
                    for (auto named : colorSpaces) {
                        if (nearly_equal(actual, named.cs.get())) {
                            name = named.name;
                            break;
                        }
                    }

                    ERRORF(r, "Mismatch: expected: %s\tactual:%s", colorSpace.name, name);
                }
            }
        }
    }
}

DEF_TEST(NdkEncode_unsupportedColorSpace, r) {
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
        for (auto ct : { kRGBA_8888_SkColorType, kRGB_565_SkColorType, kRGBA_F16_SkColorType }) {
            SkBitmap bm;
            bm.allocPixels(SkImageInfo::Make(10, 10, ct, kOpaque_SkAlphaType, unsupported));
            bm.eraseColor(SK_ColorBLUE);

            for (const auto& rec : gRecs) {
                REPORTER_ASSERT(r, !encode_ndk(bm.pixmap(), rec.format, rec.quality));
            }
        }
    }
}

#endif // SK_ENABLE_NDK_IMAGES
