/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "src/gpu/graphite/TextureFormat.h"

 #include "include/core/SkColor.h"

namespace skgpu::graphite {

const char* TextureFormatName(TextureFormat format) {
    switch (format) {
        case TextureFormat::kUnsupported:    return "Unsupported";
        case TextureFormat::kR8:             return "R8";
        case TextureFormat::kR16:            return "R16";
        case TextureFormat::kR16F:           return "R16F";
        case TextureFormat::kR32F:           return "R32F";
        case TextureFormat::kA8:             return "A8";
        case TextureFormat::kRG8:            return "RG8";
        case TextureFormat::kRG16:           return "RG16";
        case TextureFormat::kRG16F:          return "RG16F";
        case TextureFormat::kRG32F:          return "RG32F";
        case TextureFormat::kRGB8:           return "RGB8";
        case TextureFormat::kBGR8:           return "BGR8";
        case TextureFormat::kB5_G6_R5:       return "B5_G6_R5";
        case TextureFormat::kR5_G6_B5:       return "R5_G6_B5";
        case TextureFormat::kRGB16:          return "RGB16";
        case TextureFormat::kRGB16F:         return "RGB16F";
        case TextureFormat::kRGB32F:         return "RGB32F";
        case TextureFormat::kRGB8_sRGB:      return "RGB8_sRGB";
        case TextureFormat::kBGR10_XR:       return "BGR10_XR";
        case TextureFormat::kRGBA8:          return "RGBA8";
        case TextureFormat::kRGBA16:         return "RBGA16";
        case TextureFormat::kRGBA16F:        return "RGBA16F";
        case TextureFormat::kRGBA32F:        return "RGBA32F";
        case TextureFormat::kRGB10_A2:       return "RGB10_A2";
        case TextureFormat::kRGBA8_sRGB:     return "RGBA8_sRGB";
        case TextureFormat::kBGRA8:          return "BGRA8";
        case TextureFormat::kBGR10_A2:       return "BGR10_A2";
        case TextureFormat::kBGRA8_sRGB:     return "BGRA8_sRGB";
        case TextureFormat::kABGR4:          return "ABGR4";
        case TextureFormat::kARGB4:          return "ARGB4";
        case TextureFormat::kBGRA10x6_XR:    return "BGRA10x6_XR";
        case TextureFormat::kRGB8_ETC2:      return "RGB8_ETC2";
        case TextureFormat::kRGB8_ETC2_sRGB: return "RGB8_ETC2_sRGB";
        case TextureFormat::kRGB8_BC1:       return "RGB8_BC1";
        case TextureFormat::kRGBA8_BC1:      return "RGBA8_BC1";
        case TextureFormat::kRGBA8_BC1_sRGB: return "RGBA8_BC1_sRGB";
        case TextureFormat::kYUV8_P2_420:    return "YUV8_P2_420";
        case TextureFormat::kYUV8_P3_420:    return "YUV8_P3_420";
        case TextureFormat::kYUV10x6_P2_420: return "YUV10x6_P2_420";
        case TextureFormat::kExternal:       return "External";
        case TextureFormat::kS8:             return "S8";
        case TextureFormat::kD16:            return "D16";
        case TextureFormat::kD32F:           return "D32F";
        case TextureFormat::kD24_S8:         return "D24_S8";
        case TextureFormat::kD32F_S8:        return "D32F_S8";
    }
    SkUNREACHABLE;
}

SkTextureCompressionType TextureFormatCompressionType(TextureFormat format) {
    switch (format) {
        case TextureFormat::kRGB8_ETC2:      [[fallthrough]];
        case TextureFormat::kRGB8_ETC2_sRGB: return SkTextureCompressionType::kETC2_RGB8_UNORM;
        case TextureFormat::kRGB8_BC1:       return SkTextureCompressionType::kBC1_RGB8_UNORM;
        case TextureFormat::kRGBA8_BC1:      [[fallthrough]];
        case TextureFormat::kRGBA8_BC1_sRGB: return SkTextureCompressionType::kBC1_RGBA8_UNORM;
        default:                             return SkTextureCompressionType::kNone;
    }
}

size_t TextureFormatBytesPerBlock(TextureFormat format) {
    switch (format) {
        case TextureFormat::kUnsupported: return 0;
        case TextureFormat::kR8:          return 1;
        case TextureFormat::kR16:         return 2;
        case TextureFormat::kR16F:        return 2;
        case TextureFormat::kR32F:        return 4;
        case TextureFormat::kA8:          return 1;
        case TextureFormat::kRG8:         return 2;
        case TextureFormat::kRG16:        return 4;
        case TextureFormat::kRG16F:       return 4;
        case TextureFormat::kRG32F:       return 8;
        case TextureFormat::kRGB8:        return 3;
        case TextureFormat::kBGR8:        return 3;
        case TextureFormat::kB5_G6_R5:    return 2;
        case TextureFormat::kR5_G6_B5:    return 2;
        case TextureFormat::kRGB16:       return 6;
        case TextureFormat::kRGB16F:      return 6;
        case TextureFormat::kRGB32F:      return 12;
        case TextureFormat::kRGB8_sRGB:   return 3;
        case TextureFormat::kBGR10_XR:    return 4;
        case TextureFormat::kRGBA8:       return 4;
        case TextureFormat::kRGBA16:      return 8;
        case TextureFormat::kRGBA16F:     return 8;
        case TextureFormat::kRGBA32F:     return 16;
        case TextureFormat::kRGB10_A2:    return 4;
        case TextureFormat::kRGBA8_sRGB:  return 4;
        case TextureFormat::kBGRA8:       return 4;
        case TextureFormat::kBGR10_A2:    return 4;
        case TextureFormat::kBGRA8_sRGB:  return 4;
        case TextureFormat::kABGR4:       return 2;
        case TextureFormat::kARGB4:       return 2;
        case TextureFormat::kBGRA10x6_XR: return 8;
        case TextureFormat::kS8:          return 1;
        case TextureFormat::kD16:         return 2;
        case TextureFormat::kD32F:        return 4;
        case TextureFormat::kD24_S8:      return 4;
        case TextureFormat::kD32F_S8:     return 8;
        // NOTE: For compressed formats, the block size refers to an actual compressed block of
        // multiple texels, whereas with other formats the block size represents a single pixel.
        case TextureFormat::kRGB8_ETC2:
        case TextureFormat::kRGB8_ETC2_sRGB:
        case TextureFormat::kRGB8_BC1:
        case TextureFormat::kRGBA8_BC1:
        case TextureFormat::kRGBA8_BC1_sRGB:
            return 8;
        // NOTE: We don't actually know the size of external formats, so this is an arbitrary value.
        // We will see external formats only in wrapped SkImages, so this won't impact Skia's
        // internal budgeting.
        case TextureFormat::kExternal:
            return 4;
        // TODO(b/401016699): We are just over estimating this value to be used in gpu size
        // calculations even though the actually size is probably less. We should instead treat
        // planar formats similar to compressed textures that go through their own special query for
        // calculating size.
        case TextureFormat::kYUV8_P2_420:
        case TextureFormat::kYUV8_P3_420:
            return 3;
        case TextureFormat::kYUV10x6_P2_420:
            return 6;
    }
    SkUNREACHABLE;
}

uint32_t TextureFormatChannelMask(TextureFormat format) {
    switch (format) {
        case TextureFormat::kA8:             return kAlpha_SkColorChannelFlag;

        case TextureFormat::kR8:             [[fallthrough]];
        case TextureFormat::kR16:
        case TextureFormat::kR16F:
        case TextureFormat::kR32F:           return kRed_SkColorChannelFlag;

        case TextureFormat::kRG8:            [[fallthrough]];
        case TextureFormat::kRG16:
        case TextureFormat::kRG16F:
        case TextureFormat::kRG32F:          return kRG_SkColorChannelFlags;

        case TextureFormat::kRGB8:           [[fallthrough]];
        case TextureFormat::kBGR8:
        case TextureFormat::kB5_G6_R5:
        case TextureFormat::kR5_G6_B5:
        case TextureFormat::kRGB16:
        case TextureFormat::kRGB16F:
        case TextureFormat::kRGB32F:
        case TextureFormat::kRGB8_sRGB:
        case TextureFormat::kBGR10_XR:
        case TextureFormat::kRGB8_ETC2:
        case TextureFormat::kRGB8_ETC2_sRGB:
        case TextureFormat::kRGB8_BC1:
        case TextureFormat::kYUV8_P2_420:
        case TextureFormat::kYUV8_P3_420:
        case TextureFormat::kYUV10x6_P2_420: return kRGB_SkColorChannelFlags;

        case TextureFormat::kRGBA8:          [[fallthrough]];
        case TextureFormat::kRGBA16:
        case TextureFormat::kRGBA16F:
        case TextureFormat::kRGBA32F:
        case TextureFormat::kRGB10_A2:
        case TextureFormat::kRGBA8_sRGB:
        case TextureFormat::kBGRA8:
        case TextureFormat::kBGR10_A2:
        case TextureFormat::kBGRA8_sRGB:
        case TextureFormat::kABGR4:
        case TextureFormat::kARGB4:
        case TextureFormat::kBGRA10x6_XR:
        case TextureFormat::kRGBA8_BC1:
        case TextureFormat::kRGBA8_BC1_sRGB:
        case TextureFormat::kExternal:       return kRGBA_SkColorChannelFlags;

        case TextureFormat::kS8:             [[fallthrough]];
        case TextureFormat::kD16:
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
        case TextureFormat::kUnsupported:    return 0;
    }
    SkUNREACHABLE;
}

bool TextureFormatIsDepthOrStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::kS8:      [[fallthrough]];
        case TextureFormat::kD16:
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasDepth(TextureFormat format) {
    switch (format) {
        case TextureFormat::kD16:     [[fallthrough]];
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::kS8:      [[fallthrough]];
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatIsMultiplanar(TextureFormat format) {
    switch (format) {
        case TextureFormat::kYUV8_P2_420:    [[fallthrough]];
        case TextureFormat::kYUV8_P3_420:
        case TextureFormat::kYUV10x6_P2_420:
            return true;
        default:
            return false;
    }
}

} // namespace skgpu::graphite
