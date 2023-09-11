/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBackendUtils.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h" // IWYU pragma: keep
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/GrDataUtils.h"

#ifdef SK_DIRECT3D
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"
#endif

#ifdef SK_METAL
#include "src/gpu/ganesh/mtl/GrMtlCppUtil.h"
#endif

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
        case GrBackendApi::kVulkan:
            return GrBackendSurfacePriv::GetBackendData(format)->compressionType();
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatToCompressionType(format);
#else
            break;
#endif
        }
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
            return GrBackendSurfacePriv::GetBackendData(format)->bytesPerBlock();
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatBytesPerBlock(format);
#else
            break;
#endif
        }
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
                return GrCompressedRowBytes(compression, 1);
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
            return GrBackendSurfacePriv::GetBackendData(format)->stencilBits();
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatStencilBits(format);
#else
            break;
#endif
        }
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
