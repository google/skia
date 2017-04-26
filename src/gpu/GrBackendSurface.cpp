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
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo* vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(
#ifdef SK_VULKAN
                  GrVkFormatToPixelConfig(vkInfo->fFormat)
#else
                  kUnknown_GrPixelConfig
#endif
                  )
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   const GrGLTextureInfo* glInfo)
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

const GrVkImageInfo* GrBackendTexture::getVkImageInfo() const {
    if (kVulkan_GrBackend == fBackend) {
        return fVkInfo;
    }
    return nullptr;
}

const GrGLTextureInfo* GrBackendTexture::getGLTextureInfo() const {
    if (kOpenGL_GrBackend == fBackend) {
        return fGLInfo;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrVkImageInfo& vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
        , fStencilBits(stencilBits)
        , fConfig(
#ifdef SK_VULKAN
                  GrVkFormatToPixelConfig(vkInfo.fFormat)
#else
                  kUnknown_GrPixelConfig
#endif
                  )
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             GrPixelConfig config,
                                             const GrGLFramebufferInfo& glInfo)
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
        , fConfig(desc.fConfig)
        , fBackend(backend) {
    if (kOpenGL_GrBackend == backend) {
        fGLInfo = *reinterpret_cast<const GrGLFramebufferInfo*>(desc.fRenderTargetHandle);
    } else {
        SkASSERT(kVulkan_GrBackend == backend);
#ifdef SK_VULKAN
        const GrVkImageInfo* vkInfo =
                reinterpret_cast<const GrVkImageInfo*>(desc.fRenderTargetHandle);
        fConfig = GrVkFormatToPixelConfig(vkInfo->fFormat);
        fVkInfo = *vkInfo;
#else
        fConfig = kUnknown_GrPixelConfig;
#endif
    }
}

const GrVkImageInfo* GrBackendRenderTarget::getVkImageInfo() const {
    if (kVulkan_GrBackend == fBackend) {
        return &fVkInfo;
    }
    return nullptr;
}

const GrGLFramebufferInfo* GrBackendRenderTarget::getGLFramebufferInfo() const {
    if (kOpenGL_GrBackend == fBackend) {
        return &fGLInfo;
    }
    return nullptr;
}

