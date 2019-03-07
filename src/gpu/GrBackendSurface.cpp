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
#include "mtl/GrMtlCppUtil.h"
#endif

GrBackendFormat::GrBackendFormat(GrGLenum format, GrGLenum target)
        : fBackend(GrBackendApi::kOpenGL)
        , fValid(true)
        , fGLFormat(format) {
    switch (target) {
        case GR_GL_TEXTURE_2D:
            fTextureType = GrTextureType::k2D;
            break;
        case GR_GL_TEXTURE_RECTANGLE:
            fTextureType = GrTextureType::kRectangle;
            break;
        case GR_GL_TEXTURE_EXTERNAL:
            fTextureType = GrTextureType::kExternal;
            break;
        default:
            SK_ABORT("Unexpected texture target");
    }
}

const GrGLenum* GrBackendFormat::getGLFormat() const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        return &fGLFormat;
    }
    return nullptr;
}

const GrGLenum* GrBackendFormat::getGLTarget() const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        static constexpr GrGLenum k2D = GR_GL_TEXTURE_2D;
        static constexpr GrGLenum kRect = GR_GL_TEXTURE_RECTANGLE;
        static constexpr GrGLenum kExternal = GR_GL_TEXTURE_EXTERNAL;
        switch (fTextureType) {
            case GrTextureType::k2D:
                return &k2D;
            case GrTextureType::kRectangle:
                return &kRect;
            case GrTextureType::kExternal:
                return &kExternal;
        }
    }
    return nullptr;
}

GrBackendFormat GrBackendFormat::MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo) {
#ifdef SK_BUILD_FOR_ANDROID
    return GrBackendFormat(VK_FORMAT_UNDEFINED, ycbcrInfo);
#else
    return GrBackendFormat();
#endif
}

GrBackendFormat::GrBackendFormat(VkFormat vkFormat, const GrVkYcbcrConversionInfo& ycbcrInfo)
        : fBackend(GrBackendApi::kVulkan)
#ifdef SK_VULKAN
        , fValid(true)
#else
        , fValid(false)
#endif
        , fTextureType(GrTextureType::k2D) {
    fVk.fFormat = vkFormat;
    fVk.fYcbcrConversionInfo = ycbcrInfo;
    if (fVk.fYcbcrConversionInfo.isValid()) {
        fTextureType = GrTextureType::kExternal;
    }
}

const VkFormat* GrBackendFormat::getVkFormat() const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return &fVk.fFormat;
    }
    return nullptr;
}

const GrVkYcbcrConversionInfo* GrBackendFormat::getVkYcbcrConversionInfo() const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return &fVk.fYcbcrConversionInfo;
    }
    return nullptr;
}

#ifdef SK_METAL
GrBackendFormat::GrBackendFormat(GrMTLPixelFormat mtlFormat)
        : fBackend(GrBackendApi::kMetal)
        , fValid(true)
        , fMtlFormat(mtlFormat)
        , fTextureType(GrTextureType::k2D) {
}

const GrMTLPixelFormat* GrBackendFormat::getMtlFormat() const {
    if (this->isValid() && GrBackendApi::kMetal == fBackend) {
        return &fMtlFormat;
    }
    return nullptr;
}
#endif

GrBackendFormat::GrBackendFormat(GrPixelConfig config)
        : fBackend(GrBackendApi::kMock)
        , fValid(true)
        , fMockFormat(config)
        , fTextureType(GrTextureType::k2D) {
}

const GrPixelConfig* GrBackendFormat::getMockFormat() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        return &fMockFormat;
    }
    return nullptr;
}

GrBackendFormat GrBackendFormat::makeTexture2D() const {
    GrBackendFormat copy = *this;
    if (const GrVkYcbcrConversionInfo* ycbcrInfo = this->getVkYcbcrConversionInfo()) {
        if (ycbcrInfo->isValid()) {
            // If we have a ycbcr we remove it from the backend format and set the VkFormat to
            // R8G8B8A8_UNORM
            SkASSERT(copy.fBackend == GrBackendApi::kVulkan);
            copy.fVk.fYcbcrConversionInfo = GrVkYcbcrConversionInfo();
            copy.fVk.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }
    }
    copy.fTextureType = GrTextureType::k2D;
    return copy;
}

bool GrBackendFormat::operator==(const GrBackendFormat& that) const {
    // Invalid GrBackendFormats are never equal to anything.
    if (!fValid || !that.fValid) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    switch (fBackend) {
        case GrBackendApi::kOpenGL:
            return fGLFormat == that.fGLFormat;
        case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
            return fVk.fFormat == that.fVk.fFormat &&
                   fVk.fYcbcrConversionInfo == that.fVk.fYcbcrConversionInfo;
#endif
            break;
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return fMtlFormat == that.fMtlFormat;
#endif
            break;
        case GrBackendApi::kMock:
            return fMockFormat == that.fMockFormat;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    return false;
}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo)
#ifdef SK_VULKAN
        : GrBackendTexture(width, height, vkInfo,
                           sk_sp<GrVkImageLayout>(new GrVkImageLayout(vkInfo.fImageLayout))) {}
#else
        : fIsValid(false) {}
#endif

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   sk_sp<GrVkImageLayout> layout)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(kUnknown_GrPixelConfig)
        , fMipMapped(GrMipMapped(vkInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kVulkan)
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
        , fBackend(GrBackendApi::kMetal)
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
        , fBackend(GrBackendApi::kOpenGL)
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
        , fBackend(GrBackendApi::kMock)
        , fMockInfo(mockInfo) {}

GrBackendTexture::~GrBackendTexture() {
    this->cleanup();
}

void GrBackendTexture::cleanup() {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
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
        case GrBackendApi::kOpenGL:
            fGLInfo = that.fGLInfo;
            break;
        case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
            fVkInfo.assign(that.fVkInfo, this->isValid());
#endif
            break;
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            fMtlInfo = that.fMtlInfo;
            break;
#endif
        case GrBackendApi::kMock:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = that.fIsValid;
    return *this;
}

bool GrBackendTexture::getVkImageInfo(GrVkImageInfo* outInfo) const {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = fVkInfo.snapImageInfo();
        return true;
    }
#endif
    return false;
}

void GrBackendTexture::setVkImageLayout(VkImageLayout layout) {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        fVkInfo.setImageLayout(layout);
    }
#endif
}

// We need a stubbed version of GrVkImageLayout for non vulkan builds
#ifndef SK_VULKAN
class GrVkImageLayout : public SkRefCnt {
    GrVkImageLayout(VkImageLayout layout) {}
};
#endif

sk_sp<GrVkImageLayout> GrBackendTexture::getGrVkImageLayout() const {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
#endif
    return nullptr;
}

#ifdef SK_METAL
bool GrBackendTexture::getMtlTextureInfo(GrMtlTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMetal == fBackend) {
        *outInfo = fMtlInfo;
        return true;
    }
    return false;
}
#endif

bool GrBackendTexture::getGLTextureInfo(GrGLTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        *outInfo = fGLInfo;
        return true;
    }
    return false;
}

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

GrBackendFormat GrBackendTexture::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kOpenGL:
            return GrBackendFormat::MakeGL(fGLInfo.fFormat, fGLInfo.fTarget);
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            auto info = fVkInfo.snapImageInfo();
            if (info.fYcbcrConversionInfo.isValid()) {
                SkASSERT(info.fFormat == VK_FORMAT_UNDEFINED);
                return GrBackendFormat::MakeVk(info.fYcbcrConversionInfo);
            }
            return GrBackendFormat::MakeVk(info.fFormat);
        }
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal: {
            GrMtlTextureInfo mtlInfo;
            SkAssertResult(this->getMtlTextureInfo(&mtlInfo));
            return GrBackendFormat::MakeMtl(GrGetMTLPixelFormatFromMtlTextureInfo(mtlInfo));
        }
#endif
        case GrBackendApi::kMock:
            return GrBackendFormat::MakeMock(fMockInfo.fConfig);
        default:
            return GrBackendFormat();
    }
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
    case GrBackendApi::kOpenGL:
        return t0.fGLInfo == t1.fGLInfo;
    case GrBackendApi::kMock:
        return t0.fMockInfo == t1.fMockInfo;
    case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
        return t0.fVkInfo == t1.fVkInfo;
#else
        // fall through
#endif
    case GrBackendApi::kMetal:
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
#ifdef SK_VULKAN
        : GrBackendRenderTarget(width, height, sampleCnt, vkInfo,
                                sk_sp<GrVkImageLayout>(new GrVkImageLayout(vkInfo.fImageLayout))) {}
#else
        : fIsValid(false) {}
#endif

#ifdef SK_VULKAN
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
        , fBackend(GrBackendApi::kVulkan)
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
        , fBackend(GrBackendApi::kMetal)
        , fMtlInfo(mtlInfo) {}
#endif

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrGLFramebufferInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(kUnknown_GrPixelConfig)
        , fBackend(GrBackendApi::kOpenGL)
        , fGLInfo(glInfo) {
    fIsValid = SkToBool(glInfo.fFormat); // the glInfo must have a valid format
}

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
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
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
        case GrBackendApi::kOpenGL:
            fGLInfo = that.fGLInfo;
            break;
        case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
            fVkInfo.assign(that.fVkInfo, this->isValid());
#endif
            break;
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            fMtlInfo = that.fMtlInfo;
            break;
#endif
        case GrBackendApi::kMock:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = that.fIsValid;
    return *this;
}

bool GrBackendRenderTarget::getVkImageInfo(GrVkImageInfo* outInfo) const {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = fVkInfo.snapImageInfo();
        return true;
    }
#endif
    return false;
}

void GrBackendRenderTarget::setVkImageLayout(VkImageLayout layout) {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        fVkInfo.setImageLayout(layout);
    }
#endif
}

sk_sp<GrVkImageLayout> GrBackendRenderTarget::getGrVkImageLayout() const {
#ifdef SK_VULKAN
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
#endif
    return nullptr;
}

#ifdef SK_METAL
bool GrBackendRenderTarget::getMtlTextureInfo(GrMtlTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMetal == fBackend) {
        *outInfo = fMtlInfo;
        return true;
    }
    return false;
}
#endif

bool GrBackendRenderTarget::getGLFramebufferInfo(GrGLFramebufferInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        *outInfo = fGLInfo;
        return true;
    }
    return false;
}

bool GrBackendRenderTarget::getMockRenderTargetInfo(GrMockRenderTargetInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
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
    case GrBackendApi::kOpenGL:
        return r0.fGLInfo == r1.fGLInfo;
    case GrBackendApi::kMock:
        return r0.fMockInfo == r1.fMockInfo;
    case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
        return r0.fVkInfo == r1.fVkInfo;
#else
        // fall through
#endif
    case GrBackendApi::kMetal:
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
