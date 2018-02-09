/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#include "gl/GrGLUtil.h"

#ifdef SK_NXT
#include "nxt/GrNXTTypes.h"
#include "nxt/GrNXTUtil.h"
#endif

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"
#endif

#ifdef SK_NXT
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrNXTImageInfo& nxtInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(GrNXTFormatToPixelConfig(nxtInfo.fFormat))
        , fMipMapped(GrMipMapped(nxtInfo.fLevelCount > 1))
        , fBackend(kNXT_GrBackend)
        , fNXTInfo(nxtInfo) {}
#endif

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

#ifdef SK_NXT
const GrNXTImageInfo* GrBackendTexture::getNXTImageInfo() const {
    if (this->isValid() && kNXT_GrBackend == fBackend) {
        return &fNXTInfo;
    }
    return nullptr;
}
#endif

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

#ifdef SK_NXT
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrNXTImageInfo& nxtInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
        , fStencilBits(stencilBits)
        , fConfig(GrNXTFormatToPixelConfig(nxtInfo.fFormat))
        , fBackend(kNXT_GrBackend)
        , fNXTInfo(nxtInfo) {}
#endif

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrVkImageInfo& vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
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
        , fSampleCnt(sampleCnt)
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
        , fSampleCnt(sampleCnt)
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

