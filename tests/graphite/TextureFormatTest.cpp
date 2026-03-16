/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorType.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

// Types for defining color type and format behavior expectations
enum ChannelDataType {
    UNorm, Signed, Float, FNorm, sRGB, XR, Pad
};

struct Channel {
    char fName; // rgbayuv01 and G (for gray)
    int fBits;
    ChannelDataType fType;
};

struct ColorTypeExpectation {
    SkColorType fColorType;
    Swizzle fReadSwizzle;
    std::optional<Swizzle> fWriteSwizzle; // not set implies not renderable
};

struct FormatExpectation {
    TextureFormat fFormat;
    skia_private::TArray<Channel> fChannels;
    SkTextureCompressionType fCompressionType = SkTextureCompressionType::kNone;

    // Not set implies transfers are disabled; it is composed with a ColorTypeExpectation's
    // read or write swizzle to produce the expected readback/upload swizzle.
    std::optional<Swizzle> fXferSwizzle;
    // The first color type expectation is assumed to be the best fit.
    skia_private::TArray<ColorTypeExpectation> fCompatibleColorTypes;

    // All of the expectations for the fixed properties of a TextureFormat are derived from its
    // fChannels definition.
    bool isFloatingPoint() const { return this->hasType(Float) || this->hasType(FNorm); }
    bool hasDepth() const { return this->hasChannel('d'); }
    bool hasStencil() const { return this->hasChannel('s'); }
    bool hasDepthOrStencil() const { return this->hasDepth() || this->hasStencil(); }
    bool isMultiplanar() const {
        return this->hasChannel('y') && this->hasChannel('u') && this->hasChannel('v');
    }

    bool autoClamps() const {
        // Auto clamping is derived from the type of the channel with the most bits
        bool autoClamp = true;
        int maxBitSize = 0;
        for (const Channel& c : fChannels) {
            if (c.fBits > maxBitSize) {
                autoClamp = c.fType == UNorm || c.fType == sRGB;
                maxBitSize = c.fBits;
            }
        }
        return autoClamp;
    }

    int bytesPerBlock() const {
        if (fCompressionType != SkTextureCompressionType::kNone) {
            // At the moment, all supported compression types have the same bytes per block
            return 8;
        }

        int bitCount = 0;
        for (const Channel& c : fChannels) {
            bitCount += c.fBits;
        }
        return bitCount / 8;
    }

    uint32_t channelMask() const {
        uint32_t mask = 0;
        if (this->hasChannel('r') || this->hasChannel('y')) {
            mask |= kRed_SkColorChannelFlag;
        }
        if (this->hasChannel('g') || this->hasChannel('u')) {
            mask |= kGreen_SkColorChannelFlag;
        }
        if (this->hasChannel('b') || this->hasChannel('v')) {
            mask |= kBlue_SkColorChannelFlag;
        }
        if (this->hasChannel('a')) {
            mask |= kAlpha_SkColorChannelFlag;
        }
        // Other channels do not contribute to SkColorChannel mask
        return mask;
    }

private:
    bool hasChannel(char channel) const {
        for (const Channel& c : fChannels) {
            if (c.fName == channel) {
                return true;
            }
        }
        return false;
    }
    bool hasType(ChannelDataType type) const {
        for (const Channel& c : fChannels) {
            if (c.fType == type) {
                return true;
            }
        }
        return false;
    }
};


// Define the channel layout for every SkColorType for use in generating and validating the
// result of transferring data to or from a texture format.
[[maybe_unused]] static const struct ColorTypeChannels {
    SkColorType fColorType;
    Swizzle fEffectiveSwizzle; // Derivable from channel mask
    skia_private::TArray<Channel> fChannels;
} kColorTypeChannels[] {
    {kAlpha_8_SkColorType,            Swizzle("000a"), {{'a', 8, UNorm}}},
    // NOTE: 565 and 4444 are misnamed and are BGR and ABGR respectively.
    {kRGB_565_SkColorType,            Swizzle("rgb1"), {{'b', 5, UNorm},
                                                        {'g', 6, UNorm},
                                                        {'r', 5, UNorm}}},
    {kARGB_4444_SkColorType,          Swizzle("rgba"), {{'a', 4, UNorm},
                                                        {'b', 4, UNorm},
                                                        {'g', 4, UNorm},
                                                        {'r', 4, UNorm}}},
    {kRGBA_8888_SkColorType,          Swizzle("rgba"), {{'r', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'b', 8, UNorm},
                                                        {'a', 8, UNorm}}},
    {kRGB_888x_SkColorType,           Swizzle("rgb1"), {{'r', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'b', 8, UNorm},
                                                        {'x', 8, Pad}}},
    {kBGRA_8888_SkColorType,          Swizzle("rgba"), {{'b', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'r', 8, UNorm},
                                                        {'a', 8, UNorm}}},
    {kRGBA_1010102_SkColorType,       Swizzle("rgba"), {{'r', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'b', 10, UNorm},
                                                        {'a', 2, UNorm}}},
    {kBGRA_1010102_SkColorType,       Swizzle("rgba"), {{'b', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'r', 10, UNorm},
                                                        {'a', 2, UNorm}}},
    {kRGB_101010x_SkColorType,        Swizzle("rgb1"), {{'r', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'b', 10, UNorm},
                                                        {'x', 2, Pad}}},
    {kBGR_101010x_SkColorType,        Swizzle("rgb1"), {{'b', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'r', 10, UNorm},
                                                        {'x', 2, Pad}}},
    {kBGR_101010x_XR_SkColorType,     Swizzle("rgb1"), {{'b', 10, XR},
                                                        {'g', 10, XR},
                                                        {'r', 10, XR},
                                                        {'x', 2, Pad}}},
    {kBGRA_10101010_XR_SkColorType,   Swizzle("rgba"), {{'x', 6, Pad}, {'b', 10, XR},
                                                        {'x', 6, Pad}, {'g', 10, XR},
                                                        {'x', 6, Pad}, {'r', 10, XR},
                                                        {'x', 6, Pad}, {'a', 10, XR}}},
    {kRGBA_10x6_SkColorType,          Swizzle("rgba"), {{'x', 6, Pad}, {'r', 10, UNorm},
                                                        {'x', 6, Pad}, {'g', 10, UNorm},
                                                        {'x', 6, Pad}, {'b', 10, UNorm},
                                                        {'x', 6, Pad}, {'a', 10, UNorm}}},
    // NOTE: The swizzle is rrr1 since we store gray in the red channel of the texture, but we use
    // 'G' as the channel to force generating gray/luminance values in the tests instead of just 'r'
    {kGray_8_SkColorType,             Swizzle("rrr1"), {{'G', 8, UNorm}}},
    {kRGBA_F16Norm_SkColorType,       Swizzle("rgba"), {{'r', 16, FNorm},
                                                        {'g', 16, FNorm},
                                                        {'b', 16, FNorm},
                                                        {'a', 16, FNorm}}},
    {kRGBA_F16_SkColorType,           Swizzle("rgba"), {{'r', 16, Float},
                                                        {'g', 16, Float},
                                                        {'b', 16, Float},
                                                        {'a', 16, Float}}},
    {kRGB_F16F16F16x_SkColorType,     Swizzle("rgb1"), {{'r', 16, Float},
                                                        {'g', 16, Float},
                                                        {'b', 16, Float},
                                                        {'x', 16, Pad}}},
    {kRGBA_F32_SkColorType,           Swizzle("rgba"), {{'r', 32, Float},
                                                        {'g', 32, Float},
                                                        {'b', 32, Float},
                                                        {'a', 32, Float}}},
    {kR8G8_unorm_SkColorType,         Swizzle("rg01"), {{'r', 8, UNorm}, {'g', 8, UNorm}}},
    {kA16_float_SkColorType,          Swizzle("000a"), {{'a', 16, Float}}},
    {kR16G16_float_SkColorType,       Swizzle("rg01"), {{'r', 16, Float}, {'g', 16, Float}}},
    {kA16_unorm_SkColorType,          Swizzle("000a"), {{'a', 16, UNorm}}},
    {kR16_unorm_SkColorType,          Swizzle("r001"), {{'r', 16, UNorm}}},
    {kR16G16_unorm_SkColorType,       Swizzle("rg01"), {{'r', 16, UNorm}, {'g', 16, UNorm}}},
    {kR16G16B16A16_unorm_SkColorType, Swizzle("rgba"), {{'r', 16, UNorm},
                                                        {'g', 16, UNorm},
                                                        {'b', 16, UNorm},
                                                        {'a', 16, UNorm}}},
    {kSRGBA_8888_SkColorType,         Swizzle("rgba"), {{'r', 8, sRGB},
                                                        {'g', 8, sRGB},
                                                        {'b', 8, sRGB},
                                                        {'a', 8, UNorm}}},
    {kR8_unorm_SkColorType,           Swizzle("r001"), {{'r', 8, UNorm}}},
};
// Must include one per SkColorType except for kUnknown
static_assert(std::size(kColorTypeChannels) == kSkColorTypeCnt - 1,
              "Missing channel definition for SkColorType");

// Full definition of compatibility and behavior between SkColorType and TextureFormat:
static const FormatExpectation kExpectations[] {
    {.fFormat=TextureFormat::kUnsupported,
     .fChannels={},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kR8,
     .fChannels={{'r', 8, UNorm}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kR8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kAlpha_8_SkColorType,  Swizzle("000r"), Swizzle("a000")},
                             {kGray_8_SkColorType,   Swizzle("rrra"), std::nullopt}}},

    {.fFormat=TextureFormat::kR16,
     .fChannels={{'r', 16, UNorm}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kR16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kA16_unorm_SkColorType, Swizzle("000r"), Swizzle("a000")}}},

    {.fFormat=TextureFormat::kR16F,
     .fChannels={{'r', 16, Float}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kA16_float_SkColorType, Swizzle("000r"), Swizzle("a000")}}},

    {.fFormat=TextureFormat::kR32F,
     .fChannels={{'r', 32, Float}},
     .fXferSwizzle=std::nullopt,
     // TODO(michaelludwig): Use kR16_float_SkColorType once
     // https://skia-review.git.corp.google.com/c/skia/+/1165337 is landed.
     .fCompatibleColorTypes={{kA16_float_SkColorType, Swizzle("000r"), Swizzle("a000")}}},

    {.fFormat=TextureFormat::kA8,
     .fChannels={{'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("000a"),
     .fCompatibleColorTypes={{kAlpha_8_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR8G8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR16G16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGR8,
     .fChannels={{'b', 8, UNorm}, {'g', 8, UNorm}, {'r', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kB5_G6_R5,
     .fChannels={{'b', 5, UNorm}, {'g', 6, UNorm}, {'r', 5, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kR5_G6_B5,
     .fChannels={{'r', 5, UNorm}, {'g', 6, UNorm}, {'b', 5, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}, {'b', 16, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}, {'b', 16, Float}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_F16F16F16x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}, {'b', 32, Float}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB8_sRGB,
     .fChannels={{'r', 8, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGR10_XR,
     .fChannels={{'b', 10, XR}, {'g', 10, XR}, {'r', 10, XR}, {'x', 2, Pad}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kBGR_101010x_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}, {'b', 16, UNorm}, {'a', 16, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}, {'b', 16, Float}, {'a', 16, Float}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_F16_SkColorType,       Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGBA_F16Norm_SkColorType,   Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_F16F16F16x_SkColorType, Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}, {'b', 32, Float}, {'a', 32, Float}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB10_A2,
     .fChannels={{'r', 10, UNorm}, {'g', 10, UNorm}, {'b', 10, UNorm}, {'a', 2, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt},
                             {kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGR_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA10x6,
     .fChannels={{'x', 6, Pad}, {'r', 10, UNorm}, {'x', 6, Pad}, {'g', 10, UNorm},
                 {'x', 6, Pad}, {'b', 10, UNorm}, {'x', 6, Pad}, {'a', 10, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_10x6_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA8_sRGB,
     .fChannels={{'r', 8, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGRA8,
     .fChannels={{'b', 8, UNorm}, {'g', 8, UNorm}, {'r', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kBGR10_A2,
     .fChannels={{'b', 10, UNorm}, {'g', 10, UNorm}, {'r', 10, UNorm}, {'a', 2, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGR_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt},
                             {kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kBGRA8_sRGB,
     .fChannels={{'b', 8, sRGB}, {'g', 8, sRGB}, {'r', 8, sRGB}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kABGR4,
     .fChannels={{'a', 4, UNorm}, {'b', 4, UNorm}, {'g', 4, UNorm}, {'r', 4, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kARGB_4444_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kARGB4,
     // TODO(michaelludwig): kARGB_4444 color type is actually BGRA order. Historically, we
     // configured kARGB4 format to swizzle the channels on read and write in the shader so that the
     // CPU data could be uploaded directly. When we can perform a RB channel swap as part of
     // upload/readback, then this can change to RGBA swizzles.
     .fChannels={{'a', 4, UNorm}, {'r', 4, UNorm}, {'g', 4, UNorm}, {'b', 4, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kARGB_4444_SkColorType, Swizzle::BGRA(), Swizzle::BGRA()}}},

    {.fFormat=TextureFormat::kBGRA10x6_XR,
     .fChannels={{'x', 6, Pad}, {'b', 10, XR}, {'x', 6, Pad}, {'g', 10, XR},
                 {'x', 6, Pad}, {'r', 10, XR}, {'x', 6, Pad}, {'a', 10, XR}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_10101010_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    // For compressed formats, the bytes per block represents actual compressed block size, not
    // just the size of a pixel.
    {.fFormat=TextureFormat::kRGB8_ETC2,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kETC2_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGB8_ETC2_sRGB,
     .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}},
     .fCompressionType=SkTextureCompressionType::kETC2_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGB8_BC1,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA8_BC1,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGBA8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA8_BC1_sRGB,
     .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGBA8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    // For these multiplanar formats, we set the bytes per block assuming the UV planes are the
    // same size as the Y plane, which is an overestimate of the total texture memory.
    {.fFormat=TextureFormat::kYUV8_P2_420,
     .fChannels={{'y', 8, UNorm}, {'u', 8, UNorm}, {'v', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kYUV8_P3_420,
     .fChannels={{'y', 8, UNorm}, {'u', 8, UNorm}, {'v', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kYUV10x6_P2_420,
     .fChannels={{'y', 10, UNorm}, {'x', 6, Pad}, {'u', 10, UNorm}, {'x', 6, Pad},
                 {'v', 10, UNorm}, {'x', 6, Pad}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_10x6_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kExternal,
     // We don't really know this, but most Skia behavior defaults to assuming 8-bit color
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kS8,
     .fChannels={{'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD16,
     .fChannels={{'d', 16, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD32F,
     .fChannels={{'d', 32, Float}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD24_S8,
     .fChannels={{'d', 24, UNorm}, {'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD32F_S8,
     .fChannels={{'d', 32, Float}, {'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},
};

void run_texture_format_test(skiatest::Reporter* r, const Caps* caps, TextureFormat format) {
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
        REPORTER_ASSERT(r, e.bytesPerBlock() == TextureFormatBytesPerBlock(format));
        REPORTER_ASSERT(r, e.channelMask() == TextureFormatChannelMask(format));
        REPORTER_ASSERT(r, e.hasDepthOrStencil() == TextureFormatIsDepthOrStencil(format));
        REPORTER_ASSERT(r, e.hasDepth() == TextureFormatHasDepth(format));
        REPORTER_ASSERT(r, e.hasStencil() == TextureFormatHasStencil(format));
        REPORTER_ASSERT(r, e.isMultiplanar() == TextureFormatIsMultiplanar(format));
        REPORTER_ASSERT(r, e.autoClamps() == TextureFormatAutoClamps(format));
        REPORTER_ASSERT(r, e.isFloatingPoint() == TextureFormatIsFloatingPoint(format));

        // Verify compatible color types
        auto [baseColorType, _] = TextureFormatColorTypeInfo(format);
        if (baseColorType == kUnknown_SkColorType) {
            REPORTER_ASSERT(r, e.fCompatibleColorTypes.empty());
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
            const bool actualCompatible = AreColorTypeAndFormatCompatible(ct, format);
            REPORTER_ASSERT(r, foundColorExpectation == actualCompatible,
                            "actual (%d) vs expected (%d)",
                            actualCompatible, foundColorExpectation);
        }
    }

    // All formats should have expectations
    REPORTER_ASSERT(r, foundExpectation, "Missing expectation for %s", TextureFormatName(format));
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFormatTest, r, ctx, CtsEnforcement::kNextRelease) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        run_texture_format_test(r, ctx->priv().caps(), static_cast<TextureFormat>(i));
    }
}

} // namespace skgpu::graphite
