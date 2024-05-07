/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DUtil_DEFINED
#define GrD3DUtil_DEFINED

#include "include/core/SkImage.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#define GR_D3D_CALL_ERRCHECK(X)                                         \
    do {                                                                \
        HRESULT result = X;                                             \
        SkASSERT(SUCCEEDED(result));                                    \
        if (!SUCCEEDED(result)) {                                       \
            SkDebugf("Failed Direct3D call. Error: 0x%08lx\n", result); \
        }                                                               \
    } while (false)

static constexpr bool operator==(const D3D12_CPU_DESCRIPTOR_HANDLE & first,
                                 const D3D12_CPU_DESCRIPTOR_HANDLE & second) {
    return first.ptr == second.ptr;
}

/**
 * Returns true if the format is compressed.
 */
bool GrDxgiFormatIsCompressed(DXGI_FORMAT);

static constexpr uint32_t GrDxgiFormatChannels(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R8_UNORM:                 return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_B8G8R8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_B5G6R5_UNORM:             return kRGB_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:       return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16_FLOAT:                return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_R8G8_UNORM:               return kRG_SkColorChannelFlags;
        case DXGI_FORMAT_R10G10B10A2_UNORM:        return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_B4G4R4A4_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:      return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_BC1_UNORM:                return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16_FLOAT:             return kRG_SkColorChannelFlags;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:        return 0;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:     return 0;

        default:                                   return 0;
    }
}

static constexpr GrColorFormatDesc GrDxgiFormatDesc(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R8_UNORM:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_B5G6R5_UNORM:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case DXGI_FORMAT_R16_FLOAT:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kFloat);
        case DXGI_FORMAT_R8G8_UNORM:
            return GrColorFormatDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kSRGBUnorm);
        case DXGI_FORMAT_R16_UNORM:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R16G16_UNORM:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kUnorm);
        case DXGI_FORMAT_R16G16_FLOAT:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kFloat);

        // Compressed texture formats are not expected to have a description.
        case DXGI_FORMAT_BC1_UNORM: return GrColorFormatDesc::MakeInvalid();

        // This type only describes color channels.
        case DXGI_FORMAT_D24_UNORM_S8_UINT:    return GrColorFormatDesc::MakeInvalid();
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return GrColorFormatDesc::MakeInvalid();

        default: return GrColorFormatDesc::MakeInvalid();
    }
}

static constexpr size_t GrDxgiFormatBytesPerBlock(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:           return 4;
        case DXGI_FORMAT_R8_UNORM:                 return 1;
        case DXGI_FORMAT_B8G8R8A8_UNORM:           return 4;
        case DXGI_FORMAT_B5G6R5_UNORM:             return 2;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:       return 8;
        case DXGI_FORMAT_R16_FLOAT:                return 2;
        case DXGI_FORMAT_R8G8_UNORM:               return 2;
        case DXGI_FORMAT_R10G10B10A2_UNORM:        return 4;
        case DXGI_FORMAT_B4G4R4A4_UNORM:           return 2;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:      return 4;
        case DXGI_FORMAT_BC1_UNORM:                return 8;
        case DXGI_FORMAT_R16_UNORM:                return 2;
        case DXGI_FORMAT_R16G16_UNORM:             return 4;
        case DXGI_FORMAT_R16G16B16A16_UNORM:       return 8;
        case DXGI_FORMAT_R16G16_FLOAT:             return 4;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:        return 4;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:     return 8;

        default:                                   return 0;
    }
}

static constexpr int GrDxgiFormatStencilBits(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return 8;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return 8;
        default:
            return 0;
    }
}

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
static constexpr const char* GrDxgiFormatToStr(DXGI_FORMAT dxgiFormat) {
    switch (dxgiFormat) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:           return "R8G8B8A8_UNORM";
        case DXGI_FORMAT_R8_UNORM:                 return "R8_UNORM";
        case DXGI_FORMAT_B8G8R8A8_UNORM:           return "B8G8R8A8_UNORM";
        case DXGI_FORMAT_B5G6R5_UNORM:             return "R5G6B5_UNORM";
        case DXGI_FORMAT_R16G16B16A16_FLOAT:       return "R16G16B16A16_FLOAT";
        case DXGI_FORMAT_R16_FLOAT:                return "R16_FLOAT";
        case DXGI_FORMAT_R8G8_UNORM:               return "R8G8_UNORM";
        case DXGI_FORMAT_R10G10B10A2_UNORM:        return "R10G10B10A2_UNORM";
        case DXGI_FORMAT_B4G4R4A4_UNORM:           return "B4G4R4A4_UNORM";
        case DXGI_FORMAT_R32G32B32A32_FLOAT:       return "R32G32B32A32_FLOAT";
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:      return "R8G8B8A8_UNORM_SRGB";
        case DXGI_FORMAT_BC1_UNORM:                return "BC1_UNORM";
        case DXGI_FORMAT_R16_UNORM:                return "R16_UNORM";
        case DXGI_FORMAT_R16G16_UNORM:             return "R16G16_UNORM";
        case DXGI_FORMAT_R16G16B16A16_UNORM:       return "R16G16B16A16_UNORM";
        case DXGI_FORMAT_R16G16_FLOAT:             return "R16G16_FLOAT";
        case DXGI_FORMAT_D24_UNORM_S8_UINT:        return "D24_UNORM_S8_UINT";
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:     return "D32_FLOAT_S8X24_UINT";

        default:                                   return "Unknown";
    }
}
#endif

std::wstring GrD3DMultiByteToWide(const std::string& str);

#endif
