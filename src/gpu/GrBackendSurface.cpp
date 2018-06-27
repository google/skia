/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#include "gl/GrGLUtil.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"
#endif

GrBackendFormat::GrBackendFormat(GrGLenum format, GrGLenum target)
        : fBackend(kOpenGL_GrBackend)
        , fValid(true) {
    fGL.fTarget = target;
    fGL.fFormat = format;
}

const GrGLenum* GrBackendFormat::getGLFormat() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGL.fFormat;
    }
    return nullptr;
}

const GrGLenum* GrBackendFormat::getGLTarget() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGL.fTarget;
    }
    return nullptr;
}

#ifdef SK_VULKAN
GrBackendFormat::GrBackendFormat(VkFormat vkFormat)
        : fBackend(kVulkan_GrBackend)
        , fValid(true)
        , fVkFormat(vkFormat) {
}

const VkFormat* GrBackendFormat::getVkFormat() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return &fVkFormat;
    }
    return nullptr;
}
#endif

GrBackendFormat::GrBackendFormat(GrPixelConfig config)
        : fBackend(kMock_GrBackend)
        , fValid(true)
        , fMockFormat(config) {
}

const GrPixelConfig* GrBackendFormat::getMockFormat() const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        return &fMockFormat;
    }
    return nullptr;
}

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(GrVkFormatToPixelConfig(vkInfo.fFormat))
        , fMipMapped(GrMipMapped(vkInfo.fLevelCount > 1))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   const GrGLTextureInfo& glInfo)
        : GrBackendTexture(width, height, config, GrMipMapped::kNo, glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fMipMapped(mipMapped)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(GrGLSizedFormatToPixelConfig(glInfo.fFormat))
        , fMipMapped(mipMapped)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   const GrMockTextureInfo& mockInfo)
        : GrBackendTexture(width, height, config, GrMipMapped::kNo, mockInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrMipMapped mipMapped,
                                   const GrMockTextureInfo& mockInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fMipMapped(mipMapped)
        , fBackend(kMock_GrBackend)
        , fMockInfo(mockInfo) {}

#ifdef SK_VULKAN
const GrVkImageInfo* GrBackendTexture::getVkImageInfo() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return &fVkInfo;
    }
    return nullptr;
}
#endif

const GrGLTextureInfo* GrBackendTexture::getGLTextureInfo() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGLInfo;
    }
    return nullptr;
}

const GrMockTextureInfo* GrBackendTexture::getMockTextureInfo() const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        return &fMockInfo;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrVkImageInfo& vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(GrVkFormatToPixelConfig(vkInfo.fFormat))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             GrPixelConfig config,
                                             const GrGLFramebufferInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(config)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrGLFramebufferInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(GrGLSizedFormatToPixelConfig(glInfo.fFormat))
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

#ifdef SK_VULKAN
const GrVkImageInfo* GrBackendRenderTarget::getVkImageInfo() const {
    if (kVulkan_GrBackend == fBackend) {
        return &fVkInfo;
    }
    return nullptr;
}
#endif

const GrGLFramebufferInfo* GrBackendRenderTarget::getGLFramebufferInfo() const {
    if (kOpenGL_GrBackend == fBackend) {
        return &fGLInfo;
    }
    return nullptr;
}

