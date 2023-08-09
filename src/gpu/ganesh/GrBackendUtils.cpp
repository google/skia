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

#ifdef SK_VULKAN
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#endif

#ifdef SK_DIRECT3D
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"
#endif

#ifdef SK_METAL
#include "src/gpu/ganesh/mtl/GrMtlCppUtil.h"
#endif

#ifdef SK_DAWN
#include "src/gpu/dawn/DawnUtilsPriv.h"
#include "src/gpu/ganesh/dawn/GrDawnUtil.h"
#include <cstdint>
namespace wgpu { enum class TextureFormat : uint32_t; }
#endif

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL: {
            return GrBackendSurfacePriv::GetBackendData(format)->compressionType();
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            switch (vkFormat) {
                case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
                    return SkTextureCompressionType::kETC2_RGB8_UNORM;
                case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                    return SkTextureCompressionType::kBC1_RGB8_UNORM;
                case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                    return SkTextureCompressionType::kBC1_RGBA8_UNORM;
                default:
                    return SkTextureCompressionType::kNone;
            }
#else
            break;
#endif
        }
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
        case GrBackendApi::kDawn: {
            return SkTextureCompressionType::kNone;
        }
        case GrBackendApi::kMock: {
            return format.asMockCompressionType();
        }
    }
    return SkTextureCompressionType::kNone;
}

size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL: {
            return GrBackendSurfacePriv::GetBackendData(format)->bytesPerBlock();
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            return skgpu::VkFormatBytesPerBlock(vkFormat);
#else
            break;
#endif
        }
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
        case GrBackendApi::kDawn: {
#ifdef SK_DAWN
            wgpu::TextureFormat dawnFormat;
            SkAssertResult(format.asDawnFormat(&dawnFormat));
            return skgpu::DawnFormatBytesPerBlock(dawnFormat);
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
        case GrBackendApi::kOpenGL: {
            return GrBackendSurfacePriv::GetBackendData(format)->stencilBits();
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            return skgpu::VkFormatStencilBits(vkFormat);
#else
            break;
#endif
        }
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
        case GrBackendApi::kDawn: {
#ifdef SK_DAWN
            wgpu::TextureFormat dawnFormat;
            SkAssertResult(format.asDawnFormat(&dawnFormat));
            return GrDawnFormatStencilBits(dawnFormat);
#else
            break;
#endif
        }
        case GrBackendApi::kMock: {
            if (format.isMockStencilFormat()) {
                static constexpr int kMockStencilBits = 8;
                return kMockStencilBits;
            }
        }
    }
    return 0;
}
