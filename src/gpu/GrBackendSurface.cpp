/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#include "gl/GrGLUtil.h"

#ifdef SK_VULKAN
#include "vk/GrVkImageLayout.h"
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"
#endif
#ifdef SK_METAL
#include "mtl/GrMtlTypes.h"
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

#ifdef SK_METAL
GrBackendFormat::GrBackendFormat(GrMTLPixelFormat mtlFormat)
        : fBackend(kMetal_GrBackend)
        , fValid(true)
        , fMtlFormat(mtlFormat) {
}

const GrMTLPixelFormat* GrBackendFormat::getMtlFormat() const {
    if (this->isValid() && kMetal_GrBackend == fBackend) {
        return &fMtlFormat;
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
        : GrBackendTexture(width, height, vkInfo,
                           sk_sp<GrVkImageLayout>(new GrVkImageLayout(vkInfo.fImageLayout))) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   sk_sp<GrVkImageLayout> layout)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(kUnknown_GrPixelConfig)
        , fMipMapped(GrMipMapped(vkInfo.fLevelCount > 1))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo, layout.release()) {
}
#endif

#ifdef SK_METAL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrMtlTextureInfo& mtlInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(GrPixelConfig::kUnknown_GrPixelConfig)
        , fMipMapped(mipMapped)
        , fBackend(kMetal_GrBackend)
        , fMtlInfo(mtlInfo) {}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(kUnknown_GrPixelConfig)
        , fMipMapped(mipMapped)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrMockTextureInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(mockInfo.fConfig)
        , fMipMapped(mipMapped)
        , fBackend(kMock_GrBackend)
        , fMockInfo(mockInfo) {}

GrBackendTexture::~GrBackendTexture() {
    this->cleanup();
}

void GrBackendTexture::cleanup() {
#ifdef SK_VULKAN
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        fVkInfo.cleanup();
    }
#endif
}

GrBackendTexture::GrBackendTexture(const GrBackendTexture& that) : fIsValid(false) {
    *this = that;
}

GrBackendTexture& GrBackendTexture::operator=(const GrBackendTexture& that) {
    if (!that.isValid()) {
        this->cleanup();
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fConfig = that.fConfig;
    fMipMapped = that.fMipMapped;
    fBackend = that.fBackend;

    switch (that.fBackend) {
        case kOpenGL_GrBackend:
            fGLInfo = that.fGLInfo;
            break;
#ifdef SK_VULKAN
        case kVulkan_GrBackend:
            fVkInfo.assign(that.fVkInfo, this->isValid());
            break;
#endif
#ifdef SK_METAL
        case kMetal_GrBackend:
            fMtlInfo = that.fMtlInfo;
            break;
#endif
        case kMock_GrBackend:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = that.fIsValid;
    return *this;
}

#ifdef SK_VULKAN
bool GrBackendTexture::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        *outInfo = fVkInfo.snapImageInfo();
        return true;
    }
    return false;
}

void GrBackendTexture::setVkImageLayout(VkImageLayout layout) {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        fVkInfo.setImageLayout(layout);
    }
}

sk_sp<GrVkImageLayout> GrBackendTexture::getGrVkImageLayout() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
    return nullptr;
}
#endif

#ifdef SK_METAL
bool GrBackendTexture::getMtlTextureInfo(GrMtlTextureInfo* outInfo) const {
    if (this->isValid() && kMetal_GrBackend == fBackend) {
        *outInfo = fMtlInfo;
        return true;
    }
    return false;
}
#endif

bool GrBackendTexture::getGLTextureInfo(GrGLTextureInfo* outInfo) const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        *outInfo = fGLInfo;
        return true;
    }
    return false;
}

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

#if GR_TEST_UTILS
bool GrBackendTexture::TestingOnly_Equals(const GrBackendTexture& t0, const GrBackendTexture& t1) {
    if (!t0.isValid() || !t1.isValid()) {
        return false; // two invalid backend textures are not considered equal
    }

    if (t0.fWidth != t1.fWidth ||
        t0.fHeight != t1.fHeight ||
        t0.fConfig != t1.fConfig ||
        t0.fMipMapped != t1.fMipMapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    switch (t0.fBackend) {
    case kOpenGL_GrBackend:
        return t0.fGLInfo == t1.fGLInfo;
    case kMock_GrBackend:
        return t0.fMockInfo == t1.fMockInfo;
    case kVulkan_GrBackend:
#ifdef SK_VULKAN
        return t0.fVkInfo == t1.fVkInfo;
#else
        // fall through
#endif
    case kMetal_GrBackend:
#ifdef SK_METAL
        return t0.fMtlInfo == t1.fMtlInfo;
#else
        // fall through
#endif
    default:
        return false;
    }

    SkASSERT(0);
    return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, sampleCnt, vkInfo) {
    // This is a deprecated constructor that takes a bogus stencil bits.
    SkASSERT(0 == stencilBits);
}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, sampleCnt, vkInfo,
                                sk_sp<GrVkImageLayout>(new GrVkImageLayout(vkInfo.fImageLayout))) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrVkImageInfo& vkInfo,
                                             sk_sp<GrVkImageLayout> layout)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(0)  // We always create stencil buffers internally for vulkan
        , fConfig(kUnknown_GrPixelConfig)
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo, layout.release()) {}
#endif

#ifdef SK_METAL
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrMtlTextureInfo& mtlInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(0)
        , fConfig(GrPixelConfig::kUnknown_GrPixelConfig)
        , fBackend(kMetal_GrBackend)
        , fMtlInfo(mtlInfo) {}
#endif

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrGLFramebufferInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(kUnknown_GrPixelConfig)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrMockRenderTargetInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(mockInfo.fConfig)
        , fMockInfo(mockInfo) {}

GrBackendRenderTarget::~GrBackendRenderTarget() {
    this->cleanup();
}

void GrBackendRenderTarget::cleanup() {
#ifdef SK_VULKAN
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        fVkInfo.cleanup();
    }
#endif
}

GrBackendRenderTarget::GrBackendRenderTarget(const GrBackendRenderTarget& that) : fIsValid(false) {
    *this = that;
}

GrBackendRenderTarget& GrBackendRenderTarget::operator=(const GrBackendRenderTarget& that) {
    if (!that.isValid()) {
        this->cleanup();
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fSampleCnt = that.fSampleCnt;
    fStencilBits = that.fStencilBits;
    fConfig = that.fConfig;
    fBackend = that.fBackend;

    switch (that.fBackend) {
        case kOpenGL_GrBackend:
            fGLInfo = that.fGLInfo;
            break;
#ifdef SK_VULKAN
        case kVulkan_GrBackend:
            fVkInfo.assign(that.fVkInfo, this->isValid());
            break;
#endif
#ifdef SK_METAL
        case kMetal_GrBackend:
            fMtlInfo = that.fMtlInfo;
            break;
#endif
        case kMock_GrBackend:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = that.fIsValid;
    return *this;
}

#ifdef SK_VULKAN
bool GrBackendRenderTarget::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        *outInfo = fVkInfo.snapImageInfo();
        return true;
    }
    return false;
}

void GrBackendRenderTarget::setVkImageLayout(VkImageLayout layout) {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        fVkInfo.setImageLayout(layout);
    }
}

sk_sp<GrVkImageLayout> GrBackendRenderTarget::getGrVkImageLayout() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
    return nullptr;
}
#endif

#ifdef SK_METAL
bool GrBackendRenderTarget::getMtlTextureInfo(GrMtlTextureInfo* outInfo) const {
    if (this->isValid() && kMetal_GrBackend == fBackend) {
        *outInfo = fMtlInfo;
        return true;
    }
    return false;
}
#endif

bool GrBackendRenderTarget::getGLFramebufferInfo(GrGLFramebufferInfo* outInfo) const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        *outInfo = fGLInfo;
        return true;
    }
    return false;
}

bool GrBackendRenderTarget::getMockRenderTargetInfo(GrMockRenderTargetInfo* outInfo) const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

#if GR_TEST_UTILS
bool GrBackendRenderTarget::TestingOnly_Equals(const GrBackendRenderTarget& r0,
                                               const GrBackendRenderTarget& r1) {
    if (!r0.isValid() || !r1.isValid()) {
        return false; // two invalid backend rendertargets are not considered equal
    }

    if (r0.fWidth != r1.fWidth ||
        r0.fHeight != r1.fHeight ||
        r0.fSampleCnt != r1.fSampleCnt ||
        r0.fStencilBits != r1.fStencilBits ||
        r0.fConfig != r1.fConfig ||
        r0.fBackend != r1.fBackend) {
        return false;
    }

    switch (r0.fBackend) {
    case kOpenGL_GrBackend:
        return r0.fGLInfo == r1.fGLInfo;
    case kMock_GrBackend:
        return r0.fMockInfo == r1.fMockInfo;
    case kVulkan_GrBackend:
#ifdef SK_VULKAN
        return r0.fVkInfo == r1.fVkInfo;
#else
        // fall through
#endif
    case kMetal_GrBackend:
#ifdef SK_METAL
        return r0.fMtlInfo == r1.fMtlInfo;
#else
        // fall through
#endif
    default:
        return false;
    }

    SkASSERT(0);
    return false;
}
#endif
