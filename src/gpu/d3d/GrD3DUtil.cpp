/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DUtil.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/sksl/SkSLCompiler.h"

#ifdef SK_DEBUG
bool GrDxgiFormatColorTypePairIsValid(DXGI_FORMAT format, GrColorType colorType) {
    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:          return GrColorType::kRGBA_8888 == colorType ||
                                                         GrColorType::kRGB_888x == colorType;
        case DXGI_FORMAT_B8G8R8A8_UNORM:          return GrColorType::kBGRA_8888 == colorType;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:     return GrColorType::kRGBA_8888_SRGB == colorType;
        case DXGI_FORMAT_R8G8_UNORM:              return GrColorType::kRG_88 == colorType;
        case DXGI_FORMAT_R10G10B10A2_UNORM:       return GrColorType::kRGBA_1010102 == colorType;
        case DXGI_FORMAT_B5G6R5_UNORM:            return GrColorType::kBGR_565 == colorType;
        case DXGI_FORMAT_B4G4R4A4_UNORM:          return GrColorType::kABGR_4444 == colorType;
        case DXGI_FORMAT_R8_UNORM:                return GrColorType::kAlpha_8 == colorType ||
                                                         GrColorType::kGray_8 == colorType;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:      return GrColorType::kRGBA_F16 == colorType ||
                                                         GrColorType::kRGBA_F16_Clamped == colorType;
        case DXGI_FORMAT_R16_FLOAT:               return GrColorType::kAlpha_F16 == colorType;
        case DXGI_FORMAT_R16_UNORM:               return GrColorType::kAlpha_16 == colorType;
        case DXGI_FORMAT_R16G16_UNORM:            return GrColorType::kRG_1616 == colorType;
        case DXGI_FORMAT_NV12:                    return GrColorType::kRGB_888x == colorType;
        case DXGI_FORMAT_R16G16B16A16_UNORM:      return GrColorType::kRGBA_16161616 == colorType;
        case DXGI_FORMAT_R16G16_FLOAT:            return GrColorType::kRG_F16 == colorType;
        case DXGI_FORMAT_BC1_UNORM:               return GrColorType::kRGBA_8888 == colorType;
        default:                                  return false;
    }

    SkUNREACHABLE;
}
#endif

bool GrDxgiFormatIsSupported(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16_FLOAT:
            return true;
        default:
            return false;
    }
}

bool GrDxgiFormatNeedsYcbcrSampler(DXGI_FORMAT format) {
    return format == DXGI_FORMAT_NV12;
}

bool GrDxgiFormatIsCompressed(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_BC1_UNORM:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}

SkImage::CompressionType GrDxgiFormatToCompressionType(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_BC1_UNORM:    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
        default:                       return SkImage::CompressionType::kNone;
    }
    SkUNREACHABLE;
}
