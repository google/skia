/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/MtlUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

#ifdef SK_BUILD_FOR_IOS
#import <UIKit/UIApplication.h>
#endif

namespace skgpu {

bool MtlFormatIsDepthOrStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool MtlFormatIsDepth(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool MtlFormatIsStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool MtlFormatIsCompressed(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatETC2_RGB8:
            return true;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:
            return true;
#endif
        default:
            return false;
    }
}

const char* MtlFormatToString(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatInvalid:         return "Invalid";
        case MTLPixelFormatRGBA8Unorm:      return "RGBA8Unorm";
        case MTLPixelFormatR8Unorm:         return "R8Unorm";
        case MTLPixelFormatA8Unorm:         return "A8Unorm";
        case MTLPixelFormatBGRA8Unorm:      return "BGRA8Unorm";
        case MTLPixelFormatB5G6R5Unorm:     return "B5G6R5Unorm";
        case MTLPixelFormatRGBA16Float:     return "RGBA16Float";
        case MTLPixelFormatR16Float:        return "R16Float";
        case MTLPixelFormatRG8Unorm:        return "RG8Unorm";
        case MTLPixelFormatRGB10A2Unorm:    return "RGB10A2Unorm";
        case MTLPixelFormatBGR10A2Unorm:    return "BGR10A2Unorm";
        case MTLPixelFormatABGR4Unorm:      return "ABGR4Unorm";
        case MTLPixelFormatRGBA8Unorm_sRGB: return "RGBA8Unorm_sRGB";
        case MTLPixelFormatR16Unorm:        return "R16Unorm";
        case MTLPixelFormatRG16Unorm:       return "RG16Unorm";
        case MTLPixelFormatETC2_RGB8:       return "ETC2_RGB8";
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:        return "BC1_RGBA";
#endif
        case MTLPixelFormatRGBA16Unorm:     return "RGBA16Unorm";
        case MTLPixelFormatRG16Float:       return "RG16Float";
        case MTLPixelFormatStencil8:        return "Stencil8";

        default:                            return "Unknown";
    }
}

uint32_t MtlFormatChannels(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatRGBA8Unorm:      return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR8Unorm:         return kRed_SkColorChannelFlag;
        case MTLPixelFormatA8Unorm:         return kAlpha_SkColorChannelFlag;
        case MTLPixelFormatBGRA8Unorm:      return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatB5G6R5Unorm:     return kRGB_SkColorChannelFlags;
        case MTLPixelFormatRGBA16Float:     return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR16Float:        return kRed_SkColorChannelFlag;
        case MTLPixelFormatRG8Unorm:        return kRG_SkColorChannelFlags;
        case MTLPixelFormatRGB10A2Unorm:    return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatBGR10A2Unorm:    return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatABGR4Unorm:      return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatRGBA8Unorm_sRGB: return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR16Unorm:        return kRed_SkColorChannelFlag;
        case MTLPixelFormatRG16Unorm:       return kRG_SkColorChannelFlags;
        case MTLPixelFormatETC2_RGB8:       return kRGB_SkColorChannelFlags;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:        return kRGBA_SkColorChannelFlags;
#endif
        case MTLPixelFormatRGBA16Unorm:     return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatRG16Float:       return kRG_SkColorChannelFlags;
        case MTLPixelFormatStencil8:        return 0;

        default:                            return 0;
    }
}

size_t MtlFormatBytesPerBlock(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatInvalid:         return 0;
        case MTLPixelFormatRGBA8Unorm:      return 4;
        case MTLPixelFormatR8Unorm:         return 1;
        case MTLPixelFormatA8Unorm:         return 1;
        case MTLPixelFormatBGRA8Unorm:      return 4;
        case MTLPixelFormatB5G6R5Unorm:     return 2;
        case MTLPixelFormatRGBA16Float:     return 8;
        case MTLPixelFormatR16Float:        return 2;
        case MTLPixelFormatRG8Unorm:        return 2;
        case MTLPixelFormatRGB10A2Unorm:    return 4;
        case MTLPixelFormatBGR10A2Unorm:    return 4;
        case MTLPixelFormatABGR4Unorm:      return 2;
        case MTLPixelFormatRGBA8Unorm_sRGB: return 4;
        case MTLPixelFormatR16Unorm:        return 2;
        case MTLPixelFormatRG16Unorm:       return 4;
        case MTLPixelFormatETC2_RGB8:       return 8;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:        return 8;
#endif
        case MTLPixelFormatRGBA16Unorm:     return 8;
        case MTLPixelFormatRG16Float:       return 4;
        case MTLPixelFormatStencil8:        return 1;

        default:                            return 0;
    }
}

} // namespace skgpu
