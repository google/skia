/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendUtils_DEFINED
#define GrBackendUtils_DEFINED

#include "include/core/SkImage.h"

#include "include/gpu/GrBackendSurface.h"

#ifdef SK_METAL
#include "src/gpu/mtl/GrMtlCppUtil.h"
#endif

static SkImage::CompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
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

#endif

