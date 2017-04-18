/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrVkImageInfo* vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(GrVkFormatToPixelConfig(vkInfo->fFormat))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif // SK_VULKAN

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrGLTextureInfo* glInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendTexture::GrBackendTexture(const GrBackendTextureDesc& desc, GrBackend backend)
        : fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fConfig(kVulkan_GrBackend == backend
#ifdef SK_VULKAN
                  ? GrVkFormatToPixelConfig(((GrVkImageInfo*)desc.fTextureHandle)->fFormat)
#else
                  ? kUnknown_GrPixelConfig
#endif
                  : desc.fConfig)
        , fBackend(backend)
        , fHandle(desc.fTextureHandle) {}

GrVkImageInfo* GrBackendTexture::getVkImageInfo() {
    if (kVulkan_GrBackend == fBackend) {
        return fVkInfo;
    }
    return nullptr;
}

GrGLTextureInfo* GrBackendTexture::getGLTextureInfo() {
    if (kOpenGL_GrBackend == fBackend) {
        return fGLInfo;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             GrVkImageInfo* vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
        , fStencilBits(stencilBits)
        , fConfig(GrVkFormatToPixelConfig(vkInfo->fFormat))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif // SK_VULKAN

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             GrPixelConfig config,
                                             GrGLTextureInfo* glInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
        , fStencilBits(stencilBits)
        , fConfig(config)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(const GrBackendRenderTargetDesc& desc,
                                             GrBackend backend)
        : fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fSampleCnt(desc.fSampleCnt)
        , fStencilBits(desc.fStencilBits)
        , fConfig(kVulkan_GrBackend == backend
#ifdef SK_VULKAN
                  ? GrVkFormatToPixelConfig(((GrVkImageInfo*)desc.fRenderTargetHandle)->fFormat)
#else
                  ? kUnknown_GrPixelConfig
#endif
                  : desc.fConfig)
        , fBackend(backend)
        , fHandle(desc.fRenderTargetHandle) {}

GrVkImageInfo* GrBackendRenderTarget::getVkImageInfo() {
    if (kVulkan_GrBackend == fBackend) {
        return fVkInfo;
    }
    return nullptr;
}

GrGLTextureInfo* GrBackendRenderTarget::getGLTextureInfo() {
    if (kOpenGL_GrBackend == fBackend) {
        return fGLInfo;
    }
    return nullptr;
}

