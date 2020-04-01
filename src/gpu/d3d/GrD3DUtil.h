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

/**
 * Returns true if the format is compressed.
 */
bool GrDxgiFormatIsCompressed(DXGI_FORMAT);

/**
 * Maps a dxgi format into the CompressionType enum if applicable.
 */
SkImage::CompressionType GrDxgiFormatToCompressionType(DXGI_FORMAT dxgiFormat);

#if GR_TEST_UTILS
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
