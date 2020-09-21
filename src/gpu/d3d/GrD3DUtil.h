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
#include "include/private/GrTypesPriv.h"

#define GR_D3D_CALL_ERRCHECK(X)                                       \
    do {                                                              \
       HRESULT result = X;                                            \
       SkASSERT(SUCCEEDED(result));                                   \
       if (!SUCCEEDED(result)) {                                      \
           SkDebugf("Failed Direct3D call. Error: 0x%08x\n", result); \
       }                                                              \
    } while(false)

static constexpr bool operator==(const D3D12_CPU_DESCRIPTOR_HANDLE & first,
                                 const D3D12_CPU_DESCRIPTOR_HANDLE & second) {
    return first.ptr == second.ptr;
}

/**
 * Returns true if the format is compressed.
 */
bool GrDxgiFormatIsCompressed(DXGI_FORMAT);

static constexpr uint32_t GrDxgiFormatChannels(DXGI_FORMAT vkFormat) {
    switch (vkFormat) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R8_UNORM:                 return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_B8G8R8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_B5G6R5_UNORM:             return kRGB_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:       return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16_FLOAT:                return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_R8G8_UNORM:               return kRG_SkColorChannelFlags;
        case DXGI_FORMAT_R10G10B10A2_UNORM:        return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_B4G4R4A4_UNORM:           return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:       return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:      return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_BC1_UNORM:                return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case DXGI_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case DXGI_FORMAT_R16G16_FLOAT:             return kRG_SkColorChannelFlags;

        default:                                   return 0;
    }
}

#if defined(SK_DEBUG) || GR_TEST_UTILS
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

        default:                                   return "Unknown";
    }
}
#endif

#endif
