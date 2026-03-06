/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormat.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "src/core/SkImageInfoPriv.h"

namespace skgpu::graphite {

using TF = TextureFormat; // for brevity in this file

const char* TextureFormatName(TextureFormat format) {
    switch (format) {
        case TF::kUnsupported:    return "Unsupported";
        case TF::kR8:             return "R8";
        case TF::kR16:            return "R16";
        case TF::kR16F:           return "R16F";
        case TF::kR32F:           return "R32F";
        case TF::kA8:             return "A8";
        case TF::kRG8:            return "RG8";
        case TF::kRG16:           return "RG16";
        case TF::kRG16F:          return "RG16F";
        case TF::kRG32F:          return "RG32F";
        case TF::kRGB8:           return "RGB8";
        case TF::kBGR8:           return "BGR8";
        case TF::kB5_G6_R5:       return "B5_G6_R5";
        case TF::kR5_G6_B5:       return "R5_G6_B5";
        case TF::kRGB16:          return "RGB16";
        case TF::kRGB16F:         return "RGB16F";
        case TF::kRGB32F:         return "RGB32F";
        case TF::kRGB8_sRGB:      return "RGB8_sRGB";
        case TF::kBGR10_XR:       return "BGR10_XR";
        case TF::kRGBA8:          return "RGBA8";
        case TF::kRGBA16:         return "RBGA16";
        case TF::kRGBA16F:        return "RGBA16F";
        case TF::kRGBA32F:        return "RGBA32F";
        case TF::kRGB10_A2:       return "RGB10_A2";
        case TF::kRGBA10x6:       return "RGBA10x6";
        case TF::kRGBA8_sRGB:     return "RGBA8_sRGB";
        case TF::kBGRA8:          return "BGRA8";
        case TF::kBGR10_A2:       return "BGR10_A2";
        case TF::kBGRA8_sRGB:     return "BGRA8_sRGB";
        case TF::kABGR4:          return "ABGR4";
        case TF::kARGB4:          return "ARGB4";
        case TF::kBGRA10x6_XR:    return "BGRA10x6_XR";
        case TF::kRGB8_ETC2:      return "RGB8_ETC2";
        case TF::kRGB8_ETC2_sRGB: return "RGB8_ETC2_sRGB";
        case TF::kRGB8_BC1:       return "RGB8_BC1";
        case TF::kRGBA8_BC1:      return "RGBA8_BC1";
        case TF::kRGBA8_BC1_sRGB: return "RGBA8_BC1_sRGB";
        case TF::kYUV8_P2_420:    return "YUV8_P2_420";
        case TF::kYUV8_P3_420:    return "YUV8_P3_420";
        case TF::kYUV10x6_P2_420: return "YUV10x6_P2_420";
        case TF::kExternal:       return "External";
        case TF::kS8:             return "S8";
        case TF::kD16:            return "D16";
        case TF::kD32F:           return "D32F";
        case TF::kD24_S8:         return "D24_S8";
        case TF::kD32F_S8:        return "D32F_S8";
    }
    SkUNREACHABLE;
}

SkTextureCompressionType TextureFormatCompressionType(TextureFormat format) {
    switch (format) {
        case TF::kRGB8_ETC2:      [[fallthrough]];
        case TF::kRGB8_ETC2_sRGB: return SkTextureCompressionType::kETC2_RGB8_UNORM;
        case TF::kRGB8_BC1:       return SkTextureCompressionType::kBC1_RGB8_UNORM;
        case TF::kRGBA8_BC1:      [[fallthrough]];
        case TF::kRGBA8_BC1_sRGB: return SkTextureCompressionType::kBC1_RGBA8_UNORM;
        default:                  return SkTextureCompressionType::kNone;
    }
}

TextureFormat CompressionTypeToTextureFormat(SkTextureCompressionType type) {
    switch (type) {
        case SkTextureCompressionType::kBC1_RGB8_UNORM:  return TF::kRGB8_BC1;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: return TF::kRGBA8_BC1;
        case SkTextureCompressionType::kETC2_RGB8_UNORM: return TF::kRGB8_ETC2;
        default:                                         return TF::kUnsupported;
    }
}

size_t TextureFormatBytesPerBlock(TextureFormat format) {
    switch (format) {
        case TF::kUnsupported: return 0;
        case TF::kR8:          return 1;
        case TF::kR16:         return 2;
        case TF::kR16F:        return 2;
        case TF::kR32F:        return 4;
        case TF::kA8:          return 1;
        case TF::kRG8:         return 2;
        case TF::kRG16:        return 4;
        case TF::kRG16F:       return 4;
        case TF::kRG32F:       return 8;
        case TF::kRGB8:        return 3;
        case TF::kBGR8:        return 3;
        case TF::kB5_G6_R5:    return 2;
        case TF::kR5_G6_B5:    return 2;
        case TF::kRGB16:       return 6;
        case TF::kRGB16F:      return 6;
        case TF::kRGB32F:      return 12;
        case TF::kRGB8_sRGB:   return 3;
        case TF::kBGR10_XR:    return 4;
        case TF::kRGBA8:       return 4;
        case TF::kRGBA16:      return 8;
        case TF::kRGBA16F:     return 8;
        case TF::kRGBA32F:     return 16;
        case TF::kRGB10_A2:    return 4;
        case TF::kRGBA10x6:    return 8;
        case TF::kRGBA8_sRGB:  return 4;
        case TF::kBGRA8:       return 4;
        case TF::kBGR10_A2:    return 4;
        case TF::kBGRA8_sRGB:  return 4;
        case TF::kABGR4:       return 2;
        case TF::kARGB4:       return 2;
        case TF::kBGRA10x6_XR: return 8;
        case TF::kS8:          return 1;
        case TF::kD16:         return 2;
        case TF::kD32F:        return 4;
        case TF::kD24_S8:      return 4;
        case TF::kD32F_S8:     return 5; // Assuming it's multiplanar

        // NOTE: For compressed formats, the block size refers to an actual compressed block of
        // multiple texels, whereas with other formats the block size represents a single pixel.
        case TF::kRGB8_ETC2:
        case TF::kRGB8_ETC2_sRGB:
        case TF::kRGB8_BC1:
        case TF::kRGBA8_BC1:
        case TF::kRGBA8_BC1_sRGB:
            return 8;
        // NOTE: We don't actually know the size of external formats, so this is an arbitrary value.
        // We will see external formats only in wrapped SkImages, so this won't impact Skia's
        // internal budgeting.
        case TF::kExternal:
            return 4;
        // TODO(b/401016699): We are just over estimating this value to be used in gpu size
        // calculations even though the actually size is probably less. We should instead treat
        // planar formats similar to compressed textures that go through their own special query for
        // calculating size.
        case TF::kYUV8_P2_420:
        case TF::kYUV8_P3_420:
            return 3;
        case TF::kYUV10x6_P2_420:
            return 6;
    }
    SkUNREACHABLE;
}

uint32_t TextureFormatChannelMask(TextureFormat format) {
    switch (format) {
        case TF::kA8:             return kAlpha_SkColorChannelFlag;

        case TF::kR8:             [[fallthrough]];
        case TF::kR16:
        case TF::kR16F:
        case TF::kR32F:           return kRed_SkColorChannelFlag;

        case TF::kRG8:            [[fallthrough]];
        case TF::kRG16:
        case TF::kRG16F:
        case TF::kRG32F:          return kRG_SkColorChannelFlags;

        case TF::kRGB8:           [[fallthrough]];
        case TF::kBGR8:
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
        case TF::kRGB16:
        case TF::kRGB16F:
        case TF::kRGB32F:
        case TF::kRGB8_sRGB:
        case TF::kBGR10_XR:
        case TF::kRGB8_ETC2:
        case TF::kRGB8_ETC2_sRGB:
        case TF::kRGB8_BC1:
        case TF::kYUV8_P2_420:
        case TF::kYUV8_P3_420:
        case TF::kYUV10x6_P2_420: return kRGB_SkColorChannelFlags;

        case TF::kRGBA8:          [[fallthrough]];
        case TF::kRGBA16:
        case TF::kRGBA16F:
        case TF::kRGBA32F:
        case TF::kRGB10_A2:
        case TF::kRGBA10x6:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGR10_A2:
        case TF::kBGRA8_sRGB:
        case TF::kABGR4:
        case TF::kARGB4:
        case TF::kBGRA10x6_XR:
        case TF::kRGBA8_BC1:
        case TF::kRGBA8_BC1_sRGB:
        case TF::kExternal:       return kRGBA_SkColorChannelFlags;

        case TF::kS8:             [[fallthrough]];
        case TF::kD16:
        case TF::kD32F:
        case TF::kD24_S8:
        case TF::kD32F_S8:
        case TF::kUnsupported:    return 0;
    }
    SkUNREACHABLE;
}

bool TextureFormatAutoClamps(TextureFormat format) {
    // Floating point formats, extended range formats, and non-normalized integer formats do not
    // auto-clamp. Everything behaves like an unsigned normalized number.
    return !(TextureFormatIsFloatingPoint(format) ||
             format == TF::kBGR10_XR ||
             format == TF::kBGRA10x6_XR ||
             format == TF::kS8);
}

bool TextureFormatIsFloatingPoint(TextureFormat format) {
    switch (format) {
        // Floating point formats
        case TF::kR16F:           [[fallthrough]];
        case TF::kR32F:
        case TF::kRG16F:
        case TF::kRG32F:
        case TF::kRGB16F:
        case TF::kRGB32F:
        case TF::kRGBA16F:
        case TF::kRGBA32F:
        case TF::kD32F:
        case TF::kD32F_S8:        return true;

        // Everything else is unorm, unorm-srgb, fixed point, or integral
        case TF::kUnsupported:    [[fallthrough]];
        case TF::kR8:
        case TF::kR16:
        case TF::kA8:
        case TF::kRG8:
        case TF::kRG16:
        case TF::kRGB8:
        case TF::kBGR8:
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
        case TF::kRGB16:
        case TF::kRGB8_sRGB:
        case TF::kBGR10_XR:
        case TF::kRGBA8:
        case TF::kRGBA16:
        case TF::kRGB10_A2:
        case TF::kRGBA10x6:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGR10_A2:
        case TF::kBGRA8_sRGB:
        case TF::kABGR4:
        case TF::kARGB4:
        case TF::kBGRA10x6_XR:
        case TF::kRGB8_ETC2:
        case TF::kRGB8_ETC2_sRGB:
        case TF::kRGB8_BC1:
        case TF::kRGBA8_BC1:
        case TF::kRGBA8_BC1_sRGB:
        case TF::kYUV8_P2_420:
        case TF::kYUV8_P3_420:
        case TF::kYUV10x6_P2_420:
        case TF::kExternal:
        case TF::kS8:
        case TF::kD16:
        case TF::kD24_S8:          return false;
    }
    SkUNREACHABLE;
}

bool TextureFormatIsDepthOrStencil(TextureFormat format) {
    switch (format) {
        case TF::kS8:      [[fallthrough]];
        case TF::kD16:
        case TF::kD32F:
        case TF::kD24_S8:
        case TF::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasDepth(TextureFormat format) {
    switch (format) {
        case TF::kD16:     [[fallthrough]];
        case TF::kD32F:
        case TF::kD24_S8:
        case TF::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasStencil(TextureFormat format) {
    switch (format) {
        case TF::kS8:      [[fallthrough]];
        case TF::kD24_S8:
        case TF::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatIsMultiplanar(TextureFormat format) {
    switch (format) {
        case TF::kYUV8_P2_420:    [[fallthrough]];
        case TF::kYUV8_P3_420:
        case TF::kYUV10x6_P2_420:
            return true;
        default:
            return false;
    }
}

// Supporting implementation details for TextureFormat and SkColorType conversions
// ------------------------------------------------------------------------------------------------

Swizzle ReadSwizzleForColorType(SkColorType ct, TextureFormat format) {
    // TODO(b/390473370): When data transfers can apply an RG swizzle outside of the
    // SkColorType representation, we should instead apply the swizzle on upload and
    // preserve the expected order for any GPU use.
    if (ct == kARGB_4444_SkColorType && format == TextureFormat::kARGB4) {
        return Swizzle::BGRA();
    }

    uint32_t colorChannels = SkColorTypeChannelFlags(ct);
    uint32_t formatChannels = TextureFormatChannelMask(format);

    // Read swizzles only have to handle a few semantics around the sampled values, as any sort of
    // channel ordering for RGB vs BGR is handled by hardware. All we have to handle is mapping to
    // "gray", red-vs-alpha, and forcing to opaque.
    if (SkColorTypeIsAlphaOnly(ct)) {
        // If the format isn't just an alpha channel (e.g. TF::kA8), we need to adjust
        if (formatChannels != kAlpha_SkColorChannelFlag) {
            // If the format has an alpha channel, mask every other channel to 0
            if (formatChannels & kAlpha_SkColorChannelFlag) {
                return Swizzle("000a");
            } else {
                // Otherwise move the red channel to alpha
                SkASSERT(formatChannels & kRed_SkColorChannelFlag);
                return Swizzle("000r");
            }
        } else {
            // otherwise leave as "rgba" and let hardware do the right thing
            return Swizzle::RGBA();
        }
    } else {
        // First map gray to rrra; if this is just gray and not gray+alpha, it will also be forced
        // to opaque below and become rrr1.
        Swizzle swizzle;
        if (colorChannels & kGray_SkColorChannelFlag) {
            SkASSERT(formatChannels & kRed_SkColorChannelFlag);
            swizzle = Swizzle::RRRA();
        } else {
            swizzle = Swizzle::RGBA();
        }

        // Last, force the alpha to opaque if the color type masks it off but is present in the
        // texture format.
        if (!(colorChannels & kAlpha_SkColorChannelFlag) &&
             (formatChannels & kAlpha_SkColorChannelFlag)) {
            swizzle = Swizzle::Concat(swizzle, Swizzle::RGB1());
        }

        return swizzle;
    }
}

std::optional<skgpu::Swizzle> WriteSwizzleForColorType(SkColorType ct, TextureFormat format) {
    // D/S, compressed, external, and multiplanar formats aren't renderable with a color type.
    // Format support would mean we never really try to get here in practice, but keep consistent.
    if (format == TextureFormat::kExternal ||
        TextureFormatIsDepthOrStencil(format) ||
        TextureFormatIsMultiplanar(format) ||
        TextureFormatCompressionType(format) != SkTextureCompressionType::kNone) {
        return std::nullopt;
    }

    // TODO(b/390473370): When data transfers can apply an RG swizzle outside of the
    // SkColorType representation, we should instead apply the swizzle on upload and
    // preserve the expected order for any GPU use.
    if (ct == kARGB_4444_SkColorType && format == TextureFormat::kARGB4) {
        return Swizzle::BGRA();
    }

    uint32_t colorChannels = SkColorTypeChannelFlags(ct);
    uint32_t formatChannels = TextureFormatChannelMask(format);

    // Write swizzles only have to handle a few semantics around the sampled values, as any sort of
    // channel ordering for RGB vs BGR is handled by hardware. This reduces to just handling red
    // vs alpha. The other cases for read swizzles do not apply:
    //   - We disallow gray since computing luminance is beyond a swizzle.
    //   - We disallow forcing to opaque since in all cases where we'd do that we have no guarantee
    //     of what the dst pixel's alpha was. In the future, we could support forced-opaque
    //     rendering by always using shader-based blending or by guaranteeing a one-time initialize
    //     draw that forced any alpha channel to 1 (b/489785214).
    if (SkColorTypeIsAlphaOnly(ct)) {
        // If the format isn't just an alpha channel (e.g. TF::kA8), we need to adjust
        if (formatChannels != kAlpha_SkColorChannelFlag) {
            // If the format has an alpha channel, mask every other channel to 0
            if (formatChannels & kAlpha_SkColorChannelFlag) {
                return Swizzle("000a");
            } else {
                // Otherwise move the alpha channel to red
                SkASSERT(formatChannels & kRed_SkColorChannelFlag);
                return Swizzle("a000");
            }
        } else {
            // otherwise leave as "rgba" and let hardware do the right thing
            return Swizzle::RGBA();
        }
    } else {
        if ((colorChannels != formatChannels) || (colorChannels & kGray_SkColorChannelFlag)) {
            return std::nullopt;
        }
        return Swizzle::RGBA();
    }
}

SkSpan<const TextureFormat> PreferredTextureFormats(SkColorType ct) {
    #define N(...) std::size({__VA_ARGS__})
    #define CASE(C, ...) case C: { \
            static const std::array<TextureFormat, N(__VA_ARGS__)> kFormats{{__VA_ARGS__}}; \
            return SkSpan(kFormats); }

    switch (ct) {
        case kUnknown_SkColorType:  return {};
        // NOTE: Not all backends support all TextureFormats. Some of the more advanced formats
        // may not be supported at all and have no viable fallback. For color types that have
        // equivalent texture formats differing only in RGB vs. BGR swizzle, we allow both
        // format variations to maximize color types that have some format. For alpha-only color
        // types, we only match to red-channel formats as they have the broadest support.

        CASE(kAlpha_8_SkColorType,            TF::kR8)
        // NOTE: kRGB_565_SkColorType is misnamed and natively matches B5_G6_R5
        CASE(kRGB_565_SkColorType,            TF::kB5_G6_R5,   TF::kR5_G6_B5)
        // NOTE: kARGB_4444_SkColorType is misnamed and natively matches ABGR4
        CASE(kARGB_4444_SkColorType,          TF::kABGR4,      TF::kARGB4)
        CASE(kRGBA_8888_SkColorType,          TF::kRGBA8,      TF::kBGRA8)
        CASE(kRGB_888x_SkColorType,           TF::kRGB8,       TF::kRGBA8,      TF::kBGRA8)
        CASE(kBGRA_8888_SkColorType,          TF::kBGRA8,      TF::kRGBA8)
        CASE(kRGBA_1010102_SkColorType,       TF::kRGB10_A2,   TF::kBGR10_A2)
        CASE(kBGRA_1010102_SkColorType,       TF::kBGR10_A2,   TF::kRGB10_A2)
        CASE(kRGB_101010x_SkColorType,        TF::kRGB10_A2,   TF::kBGR10_A2)
        CASE(kBGR_101010x_SkColorType,        TF::kBGR10_A2,   TF::kRGB10_A2)
        CASE(kBGR_101010x_XR_SkColorType,     TF::kBGR10_XR)
        CASE(kBGRA_10101010_XR_SkColorType,   TF::kBGRA10x6_XR)
        CASE(kRGBA_10x6_SkColorType,          TF::kRGBA10x6)
        CASE(kGray_8_SkColorType,             TF::kR8)
        CASE(kRGBA_F16Norm_SkColorType,       TF::kRGBA16F)
        CASE(kRGBA_F16_SkColorType,           TF::kRGBA16F)
        CASE(kRGB_F16F16F16x_SkColorType,     TF::kRGBA16F)
        CASE(kRGBA_F32_SkColorType,           TF::kRGBA32F)
        CASE(kR8G8_unorm_SkColorType,         TF::kRG8)
        CASE(kA16_float_SkColorType,          TF::kR16F)
        CASE(kR16G16_float_SkColorType,       TF::kRG16F)
        CASE(kA16_unorm_SkColorType,          TF::kR16)
        CASE(kR16_unorm_SkColorType,          TF::kR16)
        CASE(kR16G16_unorm_SkColorType,       TF::kRG16)
        CASE(kR16G16B16A16_unorm_SkColorType, TF::kRGBA16)
        CASE(kSRGBA_8888_SkColorType,         TF::kRGBA8_sRGB,
                                              TF::kBGRA8_sRGB)
        CASE(kR8_unorm_SkColorType,           TF::kR8)
    }

    SkUNREACHABLE;
    #undef CASE
    #undef N
}

} // namespace skgpu::graphite
