/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorType.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

struct ColorTypeExpectation {
    SkColorType fColorType;
    Swizzle fReadSwizzle;
    std::optional<Swizzle> fWriteSwizzle; // not set implies not renderable
};

struct FormatExpectation {
    TextureFormat fFormat;

    // Fixed properties
    SkTextureCompressionType fCompressionType;
    size_t fBytesPerBlock;
    uint32_t fChannelMask;
    bool fHasDepth;
    bool fHasStencil;
    bool fMultiplanar;
    bool fAutoClamps;
    bool fIsFloatingPoint;

    // The first color type expectation is assumed to be the best fit.
    skia_private::TArray<ColorTypeExpectation> fCompatibleColorTypes;
};

inline
FormatExpectation MakeColor(TextureFormat format,
                            size_t bytesPerBlock,
                            uint32_t channels,
                            bool multiplanar,
                            bool autoClamps,
                            bool isFloatingPoint,
                            std::initializer_list<ColorTypeExpectation> compatibleColorTypes) {
    return {format, SkTextureCompressionType::kNone, bytesPerBlock, channels, /*fHasDepth=*/false,
            /*fHasStencil=*/false, multiplanar, autoClamps, isFloatingPoint, compatibleColorTypes};
}

inline
FormatExpectation MakeCompressed(TextureFormat format,
                                 SkTextureCompressionType compressionType,
                                 size_t bytesPerBlock,
                                 uint32_t channels,
                                 std::initializer_list<ColorTypeExpectation> compatibleColorTypes) {
    return {format, compressionType, bytesPerBlock, channels, /*fHasDepth=*/false,
            /*fHasStencil=*/false, /*fMultiplanar=*/false, /*fAutoClamps=*/true,
            /*fIsFloatingPoint=*/false, compatibleColorTypes};
}

inline
FormatExpectation MakeDepthStencil(TextureFormat format, size_t
                                   bytesPerBlock,
                                   bool hasDepth,
                                   bool hasStencil,
                                   bool autoClamps,
                                   bool isFloatingPoint) {
    return {format, SkTextureCompressionType::kNone, bytesPerBlock, /*fChannelMask=*/0, hasDepth,
            hasStencil, /*fMultiplanar=*/false,  autoClamps, isFloatingPoint,
            /*compatibleColorTypes=*/{}};
}

static const FormatExpectation kExpectations[] {
    MakeColor(TextureFormat::kUnsupported,
              /*bytesPerBlock=*/0,
              /*channels=*/0,
              /*multiplanar=*/false,
              /*autoClamps=*/true, // true is the "default" for auto-clamping behavior
              /*isFloatingPoint=*/false,
              {}),

    // TODO(michaelludwig): Right now the alpha-only color types are the expected defaults for
    // red formats. In the future this will be changed to red-only color type.
    MakeColor(TextureFormat::kR8,
              /*bytesPerBlock=*/1,
              kRed_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kAlpha_8_SkColorType, Swizzle("000r"), Swizzle("a000")},
               {kR8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kGray_8_SkColorType, Swizzle("rrra"), std::nullopt}}),

    MakeColor(TextureFormat::kR16,
              /*bytesPerBlock=*/2,
              kRed_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kA16_unorm_SkColorType, Swizzle("000r"), Swizzle("a000")},
               {kR16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kR16F,
              /*bytesPerBlock=*/2,
              kRed_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kA16_float_SkColorType, Swizzle("000r"), Swizzle("a000")}}),

    MakeColor(TextureFormat::kR32F,
              /*bytesPerBlock=*/4,
              kRed_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              // TODO(michaelludwig): Use kR16_float_SkColorType once
              // https://skia-review.git.corp.google.com/c/skia/+/1165337 is landed.
              {{kA16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kA8,
              /*bytesPerBlock=*/1,
              kAlpha_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kAlpha_8_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRG8,
              /*bytesPerBlock=*/2,
              kRed_SkColorChannelFlag | kGreen_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kR8G8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRG16,
              /*bytesPerBlock=*/4,
              kRed_SkColorChannelFlag | kGreen_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kR16G16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRG16F,
              /*bytesPerBlock=*/4,
              kRed_SkColorChannelFlag | kGreen_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRG32F,
              /*bytesPerBlock=*/8,
              kRed_SkColorChannelFlag | kGreen_SkColorChannelFlag,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB8,
              /*bytesPerBlock=*/3,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kBGR8,
              /*bytesPerBlock=*/3,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kB5_G6_R5,
              /*bytesPerBlock=*/2,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kR5_G6_B5,
              /*bytesPerBlock=*/2,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB16,
              /*bytesPerBlock=*/6,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB16F,
              /*bytesPerBlock=*/6,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kRGB_F16F16F16x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB32F,
              /*bytesPerBlock=*/12,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB8_sRGB,
              /*bytesPerBlock=*/3,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kBGR10_XR,
              /*bytesPerBlock=*/4,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/false,
              {{kBGR_101010x_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGBA8,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGB_888x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeColor(TextureFormat::kRGBA16,
              /*bytesPerBlock=*/8,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGBA16F,
              /*bytesPerBlock=*/8,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kRGBA_F16_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGBA_F16Norm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGB_F16F16F16x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeColor(TextureFormat::kRGBA32F,
              /*bytesPerBlock=*/16,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/true,
              {{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGB10_A2,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGB_101010x_SkColorType, Swizzle::RGB1(), std::nullopt},
               {kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kBGR_101010x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeColor(TextureFormat::kRGBA10x6,
              /*bytesPerBlock=*/8,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGBA_10x6_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kRGBA8_sRGB,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kBGRA8,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGB_888x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeColor(TextureFormat::kBGR10_A2,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kBGR_101010x_SkColorType, Swizzle::RGB1(), std::nullopt},
               {kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
               {kRGB_101010x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeColor(TextureFormat::kBGRA8_sRGB,
              /*bytesPerBlock=*/4,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kABGR4,
              /*bytesPerBlock=*/2,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kARGB_4444_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    MakeColor(TextureFormat::kARGB4,
              /*bytesPerBlock=*/2,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              // TODO(michaelludwig): kARGB_4444 color type is actually BGRA order. Historically,
              // we configured kARGB4 format to swizzle the channels on read and write in the
              // shader so that the CPU data could be uploaded directly. When we can perform a RB
              // channel swap as part of upload/readback, then this can change to RGBA swizzles.
              {{kARGB_4444_SkColorType, Swizzle::BGRA(), Swizzle::BGRA()}}),

    MakeColor(TextureFormat::kBGRA10x6_XR,
              /*bytesPerBlock=*/8,
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false,
              /*autoClamps=*/false,
              /*isFloatingPoint=*/false,
              {{kBGRA_10101010_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}),

    // For compressed formats, the bytes per block represents actual compressed block size, not
    // just the size of a pixel.
    MakeCompressed(TextureFormat::kRGB8_ETC2,
                   SkTextureCompressionType::kETC2_RGB8_UNORM,
                   /*bytesPerBlock=*/8,
                   kRGB_SkColorChannelFlags,
                   {{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeCompressed(TextureFormat::kRGB8_ETC2_sRGB,
                   SkTextureCompressionType::kETC2_RGB8_UNORM,
                   /*bytesPerBlock=*/8,
                   kRGB_SkColorChannelFlags,
                   {{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeCompressed(TextureFormat::kRGB8_BC1,
                   SkTextureCompressionType::kBC1_RGB8_UNORM,
                   /*bytesPerBlock=*/8,
                   kRGB_SkColorChannelFlags,
                   {{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeCompressed(TextureFormat::kRGBA8_BC1,
                   SkTextureCompressionType::kBC1_RGBA8_UNORM,
                   /*bytesPerBlock=*/8,
                   kRGBA_SkColorChannelFlags,
                   {{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeCompressed(TextureFormat::kRGBA8_BC1_sRGB,
                   SkTextureCompressionType::kBC1_RGBA8_UNORM,
                   /*bytesPerBlock=*/8,
                   kRGBA_SkColorChannelFlags,
                   {{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    // For these multiplanar formats, we set the bytes per block assuming the UV planes are the
    // same size as the Y plane, which is an overestimate of the total texture memory.
    MakeColor(TextureFormat::kYUV8_P2_420,
              /*bytesPerBlock=*/3,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/true,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeColor(TextureFormat::kYUV8_P3_420,
              /*bytesPerBlock=*/3,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/true,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeColor(TextureFormat::kYUV10x6_P2_420,
              /*bytesPerBlock=*/6,
              kRGB_SkColorChannelFlags,
              /*multiplanar=*/true,
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGBA_10x6_SkColorType, Swizzle::RGBA(), std::nullopt}}),

    MakeColor(TextureFormat::kExternal,
              /*bytesPerBlock=*/4,   // this is meaningless but is what we report
              kRGBA_SkColorChannelFlags,
              /*multiplanar=*/false, // probably actually true, but hidden from us
              /*autoClamps=*/true,
              /*isFloatingPoint=*/false,
              {{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt},
               {kRGB_888x_SkColorType, Swizzle::RGB1(), std::nullopt}}),

    MakeDepthStencil(TextureFormat::kS8,
                     /*bytesPerBlock=*/1,
                     /*hasDepth=*/false,
                     /*hasStencil=*/true,
                     /*autoClamps=*/false,
                     /*isFloatingPoint=*/false),

    MakeDepthStencil(TextureFormat::kD16,
                     /*bytesPerBlock=*/2,
                     /*hasDepth=*/true,
                     /*hasStencil=*/false,
                     /*autoClamps=*/true,
                     /*isFloatingPoint=*/false),

    MakeDepthStencil(TextureFormat::kD32F,
                     /*bytesPerBlock=*/4,
                     /*hasDepth=*/true,
                     /*hasStencil=*/false,
                     /*autoClamps=*/false,
                     /*isFloatingPoint=*/true),

    MakeDepthStencil(TextureFormat::kD24_S8,
                     /*bytesPerBlock=*/4,
                     /*hasDepth=*/true,
                     /*hasStencil=*/true,
                     /*autoClamps=*/true, // referring to main depth channel
                     /*isFloatingPoint=*/false),

    MakeDepthStencil(TextureFormat::kD32F_S8,
                     /*bytesPerBlock=*/5,
                     /*hasDepth=*/true,
                     /*hasStencil=*/true,
                     /*autoClamps=*/false,
                     /*isFloatingPoint=*/true),
};

// TODO(michaelludwig): For now format-colortype validation relies on Caps APIs that require a full
// TextureInfo, but they will be moved to fixed rules by TextureFormat. At that point, the
// backend-specific test files can also go away.
using TextureInfoFactoryFn = TextureInfo(*)(TextureFormat);

inline
void RunTextureFormatTest(skiatest::Reporter* r,
                          const Caps* caps,
                          TextureFormat format,
                          TextureInfoFactoryFn texInfoFactory) {
    bool foundExpectation = false;
    for (auto&& e : kExpectations) {
        if (e.fFormat != format) {
            continue;
        }

        // Should only find it once
        REPORTER_ASSERT(r, !foundExpectation, "Format expectation listed multiple times");
        foundExpectation = true;

        skiatest::ReporterContext scope(r, SkStringPrintf("Format %s", TextureFormatName(format)));

        // Found the expectation for the requested format. Check fixed properties first.
        REPORTER_ASSERT(r, e.fCompressionType == TextureFormatCompressionType(format));
        REPORTER_ASSERT(r, e.fBytesPerBlock == TextureFormatBytesPerBlock(format));
        REPORTER_ASSERT(r, e.fChannelMask == TextureFormatChannelMask(format));
        REPORTER_ASSERT(r, (e.fHasDepth || e.fHasStencil) == TextureFormatIsDepthOrStencil(format));
        REPORTER_ASSERT(r, e.fHasDepth == TextureFormatHasDepth(format));
        REPORTER_ASSERT(r, e.fHasStencil == TextureFormatHasStencil(format));
        REPORTER_ASSERT(r, e.fMultiplanar == TextureFormatIsMultiplanar(format));
        REPORTER_ASSERT(r, e.fAutoClamps == TextureFormatAutoClamps(format));
        REPORTER_ASSERT(r, e.fIsFloatingPoint == TextureFormatIsFloatingPoint(format));

        // Verify compatible color types
        TextureInfo texInfo = texInfoFactory(format);

        SkColorType baseColorType = caps->getDefaultColorType(texInfo);
        if (baseColorType == kUnknown_SkColorType) {
            // TODO(michaelludwig): Caps excludes color type infos for formats that aren't
            // supported, so we could see kUnknown on a given device. When compatibility is
            // separated from format support, we can instead assert this only happens when there
            // really are no compatible color types.
            // REPORTER_ASSERT(r, e.fCompatibleColorTypes.empty());
            continue;
        } else {
            // Should be the first listed compatible color type
            REPORTER_ASSERT(r, !e.fCompatibleColorTypes.empty());
            REPORTER_ASSERT(r, e.fCompatibleColorTypes[0].fColorType == baseColorType);
        }

        for (int c = 0; c <= kLastEnum_SkColorType; ++c) {
            SkColorType ct = static_cast<SkColorType>(c);

            skiatest::ReporterContext ctScope{r, SkStringPrintf("color type %d\n", c)};

            bool foundColorExpectation = false;
            for (auto&& ec : e.fCompatibleColorTypes) {
                if (ec.fColorType == ct) {
                    // Expected to be compatible (and should only find it once)
                    REPORTER_ASSERT(r, !foundColorExpectation,
                                    "Color type listed multiple times: %d",
                                    (int) ec.fColorType);
                    foundColorExpectation = true;

                    // Check swizzles here, the rest of the color type checks happen outside the
                    // loop based on `foundColorExpectation`.
                    Swizzle actualReadSwizzle = ReadSwizzleForColorType(ct, format);
                    REPORTER_ASSERT(r, ec.fReadSwizzle == actualReadSwizzle,
                                    "actual %s vs. expected %s",
                                    actualReadSwizzle.asString().c_str(),
                                    ec.fReadSwizzle.asString().c_str());

                    auto actualWriteSwizzle = WriteSwizzleForColorType(ct, format);
                    if (ec.fWriteSwizzle.has_value()) {
                        REPORTER_ASSERT(r, actualWriteSwizzle.has_value());
                        REPORTER_ASSERT(r, ec.fWriteSwizzle == actualWriteSwizzle,
                                        "actual %s vs. expected %s",
                                        actualWriteSwizzle ? actualWriteSwizzle->asString().c_str()
                                                           : "null",
                                        ec.fWriteSwizzle->asString().c_str());
                    } else {
                        REPORTER_ASSERT(r, !actualWriteSwizzle.has_value());
                        // This is a proxy for "the format can represent CT, and there are some
                        // formats that can render CT, but this format does not render w/ CT".
                        TextureInfo renderableInfo = caps->getDefaultSampledTextureInfo(
                                ct, Mipmapped::kNo, Protected::kNo, Renderable::kYes);
                        REPORTER_ASSERT(r, format != TextureInfoPriv::ViewFormat(renderableInfo));
                    }
                }
            }

            // If we found an expectation, it should be detected as compatible (and false otherwise)
            const bool actualCompatible = caps->areColorTypeAndTextureInfoCompatible(ct, texInfo);
            REPORTER_ASSERT(r, foundColorExpectation == actualCompatible,
                            "actual (%d) vs expected (%d)", actualCompatible, foundExpectation);
        }
    }

    // All formats should have expectations
    REPORTER_ASSERT(r, foundExpectation, "Missing expectation for %s", TextureFormatName(format));
}

} // namespace skgpu::graphite
