/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBackendUtils.h"

#include "src/gpu/GrDataUtils.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLUtil.h"
#endif

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkUtil.h"
#endif

#ifdef SK_DIRECT3D
#include "src/gpu/d3d/GrD3DUtil.h"
#endif

#ifdef SK_METAL
#include "src/gpu/mtl/GrMtlCppUtil.h"
#endif

#ifdef SK_DAWN
#include "src/gpu/dawn/GrDawnUtil.h"
#endif

SkImage::CompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL: {
#ifdef SK_GL
            GrGLFormat glFormat = format.asGLFormat();
            switch (glFormat) {
                case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
                case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
                    return SkImage::CompressionType::kETC2_RGB8_UNORM;
                case GrGLFormat::kCOMPRESSED_RGB8_BC1:
                    return SkImage::CompressionType::kBC1_RGB8_UNORM;
                case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
                    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
                default:
                    return SkImage::CompressionType::kNone;
            }
#endif
            break;
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            switch (vkFormat) {
                case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
                    return SkImage::CompressionType::kETC2_RGB8_UNORM;
                case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                    return SkImage::CompressionType::kBC1_RGB8_UNORM;
                case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
                default:
                    return SkImage::CompressionType::kNone;
            }
#endif
            break;
        }
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatToCompressionType(format);
#endif
            break;
        }
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            switch (dxgiFormat) {
                case DXGI_FORMAT_BC1_UNORM:
                    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
                default:
                    return SkImage::CompressionType::kNone;
            }
#endif
            break;
        }
        case GrBackendApi::kDawn: {
            return SkImage::CompressionType::kNone;
        }
        case GrBackendApi::kMock: {
            return format.asMockCompressionType();
        }
    }
    return SkImage::CompressionType::kNone;
}

size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL: {
#ifdef SK_GL
            GrGLFormat glFormat = format.asGLFormat();
            return GrGLFormatBytesPerBlock(glFormat);
#endif
            break;
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            return GrVkFormatBytesPerBlock(vkFormat);
#endif
            break;
        }
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatBytesPerBlock(format);
#endif
            break;
        }
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            return GrDxgiFormatBytesPerBlock(dxgiFormat);
#endif
            break;
        }
        case GrBackendApi::kDawn: {
#ifdef SK_DAWN
            wgpu::TextureFormat dawnFormat;
            SkAssertResult(format.asDawnFormat(&dawnFormat));
            return GrDawnBytesPerBlock(dawnFormat);
#endif
            break;
        }
        case GrBackendApi::kMock: {
            SkImage::CompressionType compression = format.asMockCompressionType();
            if (compression != SkImage::CompressionType::kNone) {
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
    if (GrBackendFormatToCompressionType(format) != SkImage::CompressionType::kNone) {
        return 0;
    }
    return GrBackendFormatBytesPerBlock(format);
}

int GrBackendFormatStencilBits(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL: {
#ifdef SK_GL
            GrGLFormat glFormat = format.asGLFormat();
            return GrGLFormatStencilBits(glFormat);
#endif
            break;
        }
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            SkAssertResult(format.asVkFormat(&vkFormat));
            return GrVkFormatStencilBits(vkFormat);
#endif
            break;
        }
        case GrBackendApi::kMetal: {
#ifdef SK_METAL
            return GrMtlBackendFormatStencilBits(format);
#endif
            break;
        }
        case GrBackendApi::kDirect3D: {
#ifdef SK_DIRECT3D
            DXGI_FORMAT dxgiFormat;
            SkAssertResult(format.asDxgiFormat(&dxgiFormat));
            return GrDxgiFormatStencilBits(dxgiFormat);
#endif
            break;
        }
        case GrBackendApi::kDawn: {
#ifdef SK_DAWN
            wgpu::TextureFormat dawnFormat;
            SkAssertResult(format.asDawnFormat(&dawnFormat));
            return GrDawnFormatStencilBits(dawnFormat);
#endif
            break;
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
