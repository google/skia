/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrBackendSurface.h"

#include "src/gpu/gl/GrGLUtil.h"

#ifdef SK_DAWN
#include "include/gpu/dawn/GrDawnTypes.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkImageLayout.h"
#include "src/gpu/vk/GrVkUtil.h"
#endif
#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/mtl/GrMtlCppUtil.h"
#endif

GrBackendFormat::GrBackendFormat(const GrBackendFormat& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fTextureType(that.fTextureType) {
    if (!fValid) {
        return;
    }

    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            fGLFormat = that.fGLFormat;
            break;
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            fVk = that.fVk;
            break;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            fMtlFormat = that.fMtlFormat;
            break;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            fDawnFormat = that.fDawnFormat;
            break;
#endif
        case GrBackendApi::kMock:
            fMockColorType = that.fMockColorType;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

GrBackendFormat::GrBackendFormat(GrGLenum format, GrGLenum target)
        : fBackend(GrBackendApi::kOpenGL)
        , fValid(true)
        , fGLFormat(format) {
    switch (target) {
        case GR_GL_TEXTURE_NONE:
            fTextureType = GrTextureType::kNone;
            break;
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
        static constexpr GrGLenum kNone = GR_GL_TEXTURE_NONE;
        static constexpr GrGLenum k2D = GR_GL_TEXTURE_2D;
        static constexpr GrGLenum kRect = GR_GL_TEXTURE_RECTANGLE;
        static constexpr GrGLenum kExternal = GR_GL_TEXTURE_EXTERNAL;
        switch (fTextureType) {
            case GrTextureType::kNone:
                return &kNone;
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

#ifdef SK_DAWN
GrBackendFormat::GrBackendFormat(dawn::TextureFormat format)
        : fBackend(GrBackendApi::kDawn)
        , fValid(true)
        , fDawnFormat(format)
        , fTextureType(GrTextureType::k2D) {
}

const dawn::TextureFormat* GrBackendFormat::getDawnFormat() const {
    if (this->isValid() && GrBackendApi::kDawn == fBackend) {
        return &fDawnFormat;
    }
    return nullptr;
}
#endif

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

GrBackendFormat::GrBackendFormat(GrColorType colorType)
        : fBackend(GrBackendApi::kMock)
        , fValid(true)
        , fTextureType(GrTextureType::k2D) {
    fMockColorType = colorType;
}

const GrColorType* GrBackendFormat::getMockColorType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        return &fMockColorType;
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
        case GrBackendApi::kDawn:
#ifdef SK_DAWN
            return fDawnFormat == that.fDawnFormat;
#endif
            break;
        case GrBackendApi::kMock:
            return fMockColorType == that.fMockColorType;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    return false;
}

#ifdef SK_DAWN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrDawnImageInfo& dawnInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(GrMipMapped(dawnInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kDawn)
        , fDawnInfo(dawnInfo) {}
#endif

GrBackendTexture::GrBackendTexture(int width, int height, const GrVkImageInfo& vkInfo)
#ifdef SK_VULKAN
        : GrBackendTexture(width, height, vkInfo,
                           sk_sp<GrVkImageLayout>(new GrVkImageLayout(vkInfo.fImageLayout))) {}
#else
        : fIsValid(false) {}
#endif

#ifdef SK_GL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo glInfo,
                                   sk_sp<GrGLTextureParameters> params)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(mipMapped)
        , fBackend(GrBackendApi::kOpenGL)
        , fGLInfo(glInfo, params.release()) {}

sk_sp<GrGLTextureParameters> GrBackendTexture::getGLTextureParams() const {
    if (fBackend != GrBackendApi::kOpenGL) {
        return nullptr;
    }
    return fGLInfo.refParameters();
}
#endif

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   sk_sp<GrVkImageLayout> layout)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(GrMipMapped(vkInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kVulkan)
        , fVkInfo(vkInfo, layout.release()) {}
#endif

#ifdef SK_METAL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrMtlTextureInfo& mtlInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(mipMapped)
        , fBackend(GrBackendApi::kMetal)
        , fMtlInfo(mtlInfo) {}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : GrBackendTexture(width, height, mipMapped, glInfo, sk_make_sp<GrGLTextureParameters>()) {
    // Make no assumptions about client's texture's parameters.
    this->glTextureParametersModified();
}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrMockTextureInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(mipMapped)
        , fBackend(GrBackendApi::kMock)
        , fMockInfo(mockInfo) {}

GrBackendTexture::~GrBackendTexture() {
    this->cleanup();
}

void GrBackendTexture::cleanup() {
#ifdef SK_GL
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        fGLInfo.cleanup();
    }
#endif
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
    } else if (fIsValid && this->fBackend != that.fBackend) {
        this->cleanup();
        fIsValid = false;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fMipMapped = that.fMipMapped;
    fBackend = that.fBackend;

    switch (that.fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            fGLInfo.assign(that.fGLInfo, this->isValid());
            break;
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            fVkInfo.assign(that.fVkInfo, this->isValid());
            break;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            fMtlInfo = that.fMtlInfo;
            break;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            fDawnInfo = that.fDawnInfo;
            break;
#endif
        case GrBackendApi::kMock:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = true;
    return *this;
}

#ifdef SK_DAWN
bool GrBackendTexture::getDawnImageInfo(GrDawnImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kDawn == fBackend) {
        *outInfo = fDawnInfo;
        return true;
    }
    return false;
}
#endif

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

#ifdef SK_VULKAN
sk_sp<GrVkImageLayout> GrBackendTexture::getGrVkImageLayout() const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
    return nullptr;
}
#endif

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
#ifdef SK_GL
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        *outInfo = fGLInfo.info();
        return true;
    } else if (this->isValid() && GrBackendApi::kMock == fBackend) {
        // Hack! This allows some blink unit tests to work when using the Mock GrContext.
        // Specifically, tests that rely on CanvasResourceProviderTextureGpuMemoryBuffer.
        // If that code ever goes away (or ideally becomes backend-agnostic), this can go away.
        *outInfo = GrGLTextureInfo{ GR_GL_TEXTURE_2D,
                                    static_cast<GrGLuint>(fMockInfo.fID),
                                    GR_GL_RGBA8 };
        return true;
    }
#endif
    return false;
}

void GrBackendTexture::glTextureParametersModified() {
#ifdef SK_GL
    if (this->isValid() && fBackend == GrBackendApi::kOpenGL) {
        fGLInfo.parameters()->invalidate();
    }
#endif
}

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

bool GrBackendTexture::isProtected() const {
    if (!this->isValid() || this->backend() != GrBackendApi::kVulkan) {
        return false;
    }
    return fVkInfo.isProtected();
}

bool GrBackendTexture::isSameTexture(const GrBackendTexture& that) {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }
    if (fBackend != that.fBackend) {
        return false;
    }
    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return fGLInfo.info().fID == that.fGLInfo.info().fID;
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return fVkInfo.snapImageInfo().fImage == that.fVkInfo.snapImageInfo().fImage;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return this->fMtlInfo.fTexture == that.fMtlInfo.fTexture;
#endif
        case GrBackendApi::kMock:
            return fMockInfo.fID == that.fMockInfo.fID;
        default:
            return false;
    }
}

GrBackendFormat GrBackendTexture::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return GrBackendFormat::MakeGL(fGLInfo.info().fFormat, fGLInfo.info().fTarget);
#endif
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
#ifdef SK_DAWN
        case GrBackendApi::kDawn: {
            return GrBackendFormat::MakeDawn(fDawnInfo.fFormat);
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
            return fMockInfo.getBackendFormat();
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
        t0.fMipMapped != t1.fMipMapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    switch (t0.fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return t0.fGLInfo.info() == t1.fGLInfo.info();
#endif
        case GrBackendApi::kMock:
            return t0.fMockInfo == t1.fMockInfo;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return t0.fVkInfo == t1.fVkInfo;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return t0.fMtlInfo == t1.fMtlInfo;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            return t0.fDawnInfo == t1.fDawnInfo;
#endif
        default:
            return false;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DAWN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrDawnImageInfo& dawnInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(sampleCnt)
        , fStencilBits(stencilBits)
        , fBackend(GrBackendApi::kDawn)
        , fDawnInfo(dawnInfo) {}
#endif

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
    } else if (fIsValid && this->fBackend != that.fBackend) {
        this->cleanup();
        fIsValid = false;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fSampleCnt = that.fSampleCnt;
    fStencilBits = that.fStencilBits;
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
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            fDawnInfo = that.fDawnInfo;
            break;
#endif
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

#ifdef SK_DAWN
bool GrBackendRenderTarget::getDawnImageInfo(GrDawnImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kDawn == fBackend) {
        *outInfo = fDawnInfo;
        return true;
    }
    return false;
}
#endif

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

#ifdef SK_VULKAN
sk_sp<GrVkImageLayout> GrBackendRenderTarget::getGrVkImageLayout() const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return fVkInfo.getGrVkImageLayout();
    }
    return nullptr;
}
#endif

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

GrBackendFormat GrBackendRenderTarget::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return GrBackendFormat::MakeGL(fGLInfo.fFormat, GR_GL_TEXTURE_NONE);
#endif
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
            return fMockInfo.getBackendFormat();
        default:
            return GrBackendFormat();
    }
}

bool GrBackendRenderTarget::getMockRenderTargetInfo(GrMockRenderTargetInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

bool GrBackendRenderTarget::isProtected() const {
    if (!this->isValid() || this->backend() != GrBackendApi::kVulkan) {
        return false;
    }
    return fVkInfo.isProtected();
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
        r0.fBackend != r1.fBackend) {
        return false;
    }

    switch (r0.fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return r0.fGLInfo == r1.fGLInfo;
#endif
        case GrBackendApi::kMock:
            return r0.fMockInfo == r1.fMockInfo;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return r0.fVkInfo == r1.fVkInfo;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return r0.fMtlInfo == r1.fMtlInfo;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            return r0.fDawnInfo == r1.fDawnInfo;
#endif
        default:
            return false;
    }

    SkASSERT(0);
    return false;
}
#endif
