/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBackendUtils.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h" // IWYU pragma: keep
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"

#ifdef SK_DIRECT3D
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"
#endif

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
        case GrBackendApi::kVulkan:
        case GrBackendApi::kMetal:
            return GrBackendSurfacePriv::GetBackendData(format)->compressionType();
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            switch (dxgiFormat) {
                case DXGI_FORMAT_BC1_UNORM:
                    return SkTextureCompressionType::kBC1_RGBA8_UNORM;
                default:
                    return SkTextureCompressionType::kNone;
            }
#else
            break;
#endif
        }
        case GrBackendApi::kMock: {
            return format.asMockCompressionType();
        }
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return SkTextureCompressionType::kNone;
}

size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
        case GrBackendApi::kVulkan:
        case GrBackendApi::kMetal:
            return GrBackendSurfacePriv::GetBackendData(format)->bytesPerBlock();
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            return GrDxgiFormatBytesPerBlock(dxgiFormat);
#else
            break;
#endif
        }
        case GrBackendApi::kMock: {
            SkTextureCompressionType compression = format.asMockCompressionType();
            if (compression != SkTextureCompressionType::kNone) {
                return skgpu::CompressedRowBytes(compression, 1);
            } else if (format.isMockStencilFormat()) {
                static constexpr int kMockStencilSize = 4;
                return kMockStencilSize;
            }
            return GrColorTypeBytesPerPixel(format.asMockColorType());
        }
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return 0;
}

size_t GrBackendFormatBytesPerPixel(const GrBackendFormat& format) {
    if (GrBackendFormatToCompressionType(format) != SkTextureCompressionType::kNone) {
        return 0;
    }
    return GrBackendFormatBytesPerBlock(format);
}

int GrBackendFormatStencilBits(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
        case GrBackendApi::kVulkan:
        case GrBackendApi::kMetal:
            return GrBackendSurfacePriv::GetBackendData(format)->stencilBits();
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            return GrDxgiFormatStencilBits(dxgiFormat);
#else
            break;
#endif
        }
        case GrBackendApi::kMock: {
            if (format.isMockStencilFormat()) {
                static constexpr int kMockStencilBits = 8;
                return kMockStencilBits;
            }
            break;
        }
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return 0;
}
