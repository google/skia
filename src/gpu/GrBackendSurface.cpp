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
#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DResourceState.h"
#include "src/gpu/d3d/GrD3DUtil.h"
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            fDxgiFormat = that.fDxgiFormat;
            break;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            fDawnFormat = that.fDawnFormat;
            break;
#endif
        case GrBackendApi::kMock:
            fMock = that.fMock;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

GrBackendFormat& GrBackendFormat::operator=(const GrBackendFormat& that) {
    if (this != &that) {
        this->~GrBackendFormat();
        new (this) GrBackendFormat(that);
    }
    return *this;
}

#ifdef SK_GL
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
#endif

GrGLFormat GrBackendFormat::asGLFormat() const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        return GrGLFormatFromGLEnum(fGLFormat);
    }
    return GrGLFormat::kUnknown;
}

GrBackendFormat GrBackendFormat::MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo) {
    SkASSERT(ycbcrInfo.isValid());
    return GrBackendFormat(ycbcrInfo.fFormat, ycbcrInfo);
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
    if (fVk.fYcbcrConversionInfo.isValid() && fVk.fYcbcrConversionInfo.fExternalFormat) {
        fTextureType = GrTextureType::kExternal;
    }
}

bool GrBackendFormat::asVkFormat(VkFormat* format) const {
    SkASSERT(format);
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *format = fVk.fFormat;
        return true;
    }
    return false;
}

const GrVkYcbcrConversionInfo* GrBackendFormat::getVkYcbcrConversionInfo() const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        return &fVk.fYcbcrConversionInfo;
    }
    return nullptr;
}

#ifdef SK_DAWN
GrBackendFormat::GrBackendFormat(wgpu::TextureFormat format)
        : fBackend(GrBackendApi::kDawn)
        , fValid(true)
        , fDawnFormat(format)
        , fTextureType(GrTextureType::k2D) {
}

bool GrBackendFormat::asDawnFormat(wgpu::TextureFormat* format) const {
    SkASSERT(format);
    if (this->isValid() && GrBackendApi::kDawn == fBackend) {
        *format = fDawnFormat;
        return true;
    }
    return false;
}
#endif

#ifdef SK_METAL
GrBackendFormat::GrBackendFormat(GrMTLPixelFormat mtlFormat)
        : fBackend(GrBackendApi::kMetal)
        , fValid(true)
        , fMtlFormat(mtlFormat)
        , fTextureType(GrTextureType::k2D) {
}

GrMTLPixelFormat GrBackendFormat::asMtlFormat() const {
    if (this->isValid() && GrBackendApi::kMetal == fBackend) {
        return fMtlFormat;
    }
    // MTLPixelFormatInvalid == 0
    return GrMTLPixelFormat(0);
}
#endif

#ifdef SK_DIRECT3D
GrBackendFormat::GrBackendFormat(DXGI_FORMAT dxgiFormat)
    : fBackend(GrBackendApi::kDirect3D)
    , fValid(true)
    , fDxgiFormat(dxgiFormat)
    , fTextureType(GrTextureType::k2D) {
}

bool GrBackendFormat::asDxgiFormat(DXGI_FORMAT* dxgiFormat) const {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        *dxgiFormat = fDxgiFormat;
        return true;
    }
    return false;
}
#endif

GrBackendFormat::GrBackendFormat(GrColorType colorType, SkImage::CompressionType compression)
        : fBackend(GrBackendApi::kMock)
        , fValid(true)
        , fTextureType(GrTextureType::k2D) {
    fMock.fColorType = colorType;
    fMock.fCompressionType = compression;
}

GrColorType GrBackendFormat::asMockColorType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(fMock.fCompressionType == SkImage::CompressionType::kNone ||
                 fMock.fColorType == GrColorType::kUnknown);

        return fMock.fColorType;
    }

    return GrColorType::kUnknown;
}

SkImage::CompressionType GrBackendFormat::asMockCompressionType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(fMock.fCompressionType == SkImage::CompressionType::kNone ||
                 fMock.fColorType == GrColorType::kUnknown);

        return fMock.fCompressionType;
    }

    return SkImage::CompressionType::kNone;
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

GrBackendFormat GrBackendFormat::MakeMock(GrColorType colorType,
                                          SkImage::CompressionType compression) {
    return GrBackendFormat(colorType, compression);
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
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return fGLFormat == that.fGLFormat;
            break;
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return fVk.fFormat == that.fVk.fFormat &&
                   fVk.fYcbcrConversionInfo == that.fVk.fYcbcrConversionInfo;
            break;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return fMtlFormat == that.fMtlFormat;
            break;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            return fDawnFormat == that.fDawnFormat;
            break;
#endif
        case GrBackendApi::kMock:
            return fMock.fColorType == that.fMock.fColorType &&
                   fMock.fCompressionType == that.fMock.fCompressionType;
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return fDxgiFormat == that.fDxgiFormat;
#endif
        default:
            SK_ABORT("Unknown GrBackend");
    }
    return false;
}

#if GR_TEST_UTILS
#include "include/core/SkString.h"
#include "src/gpu/GrTestUtils.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLUtil.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkUtil.h"
#endif

SkString GrBackendFormat::toStr() const {
    SkString str;

    if (!fValid) {
        str.append("invalid");
        return str;
    }

    str.appendf("%s-", GrBackendApiToStr(fBackend));

    switch (fBackend) {
        case GrBackendApi::kOpenGL:
#ifdef SK_GL
            str.append(GrGLFormatToStr(fGLFormat));
#endif
            break;
        case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
            str.append(GrVkFormatToStr(fVk.fFormat));
#endif
            break;
        case GrBackendApi::kMetal:
#ifdef SK_METAL
            str.append(GrMtlFormatToStr(fMtlFormat));
#endif
            break;
        case GrBackendApi::kDirect3D:
#ifdef SK_DIRECT3D
            str.append(GrDxgiFormatToStr(fDxgiFormat));
#endif
            break;
        case GrBackendApi::kDawn:
#ifdef SK_DAWN
            str.append(GrDawnFormatToStr(fDawnFormat));
#endif
            break;
        case GrBackendApi::kMock:
            str.append(GrColorTypeToStr(fMock.fColorType));
            str.appendf("-");
            str.append(GrCompressionTypeToStr(fMock.fCompressionType));
            break;
    }

    return str;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SK_DAWN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrDawnTextureInfo& dawnInfo)
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

#ifdef SK_DIRECT3D
GrBackendTexture::GrBackendTexture(int width, int height, const GrD3DTextureResourceInfo& d3dInfo)
        : GrBackendTexture(
                width, height, d3dInfo,
                sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                        static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState)))) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrD3DTextureResourceInfo& d3dInfo,
                                   sk_sp<GrD3DResourceState> state)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipMapped(GrMipMapped(d3dInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kDirect3D)
        , fD3DInfo(d3dInfo, state.release()) {}
#endif

#ifdef SK_GL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : GrBackendTexture(width, height, mipMapped, glInfo, sk_make_sp<GrGLTextureParameters>()) {
    // Make no assumptions about client's texture's parameters.
    this->glTextureParametersModified();
}
#endif

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
#ifdef SK_DIRECT3D
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        fD3DInfo.cleanup();
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            fD3DInfo.assign(that.fD3DInfo, this->isValid());
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
bool GrBackendTexture::getDawnTextureInfo(GrDawnTextureInfo* outInfo) const {
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

#ifdef SK_DIRECT3D
bool GrBackendTexture::getD3DTextureResourceInfo(GrD3DTextureResourceInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        *outInfo = fD3DInfo.snapTextureResourceInfo();
        return true;
    }
    return false;
}

void GrBackendTexture::setD3DResourceState(GrD3DResourceStateEnum state) {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        fD3DInfo.setResourceState(state);
    }
}

sk_sp<GrD3DResourceState> GrBackendTexture::getGrD3DResourceState() const {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        return fD3DInfo.getGrD3DResourceState();
    }
    return nullptr;
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
                                    static_cast<GrGLuint>(fMockInfo.id()),
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return false; //TODO
#endif
        case GrBackendApi::kMock:
            return fMockInfo.id() == that.fMockInfo.id();
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
                SkASSERT(info.fFormat == info.fYcbcrConversionInfo.fFormat);
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D: {
            auto d3dInfo = fD3DInfo.snapTextureResourceInfo();
            return GrBackendFormat::MakeDxgi(d3dInfo.fFormat);
        }
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn: {
            return GrBackendFormat::MakeDawn(fDawnInfo.fFormat);
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return t0.fD3DInfo == t1.fD3DInfo;
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
                                             const GrDawnRenderTargetInfo& dawnInfo)
        : fIsValid(true)
        , fFramebufferOnly(true)
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
        , fSampleCnt(std::max(1, sampleCnt))
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
        , fFramebufferOnly(false) // TODO: set this from mtlInfo.fTexture->framebufferOnly
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, sampleCnt))
        , fStencilBits(0)
        , fBackend(GrBackendApi::kMetal)
        , fMtlInfo(mtlInfo) {}
#endif

#ifdef SK_DIRECT3D
GrBackendRenderTarget::GrBackendRenderTarget(int width, int height, int sampleCnt,
                                             const GrD3DTextureResourceInfo& d3dInfo)
        : GrBackendRenderTarget(
                width, height, sampleCnt, d3dInfo,
                sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                        static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState)))) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrD3DTextureResourceInfo& d3dInfo,
                                             sk_sp<GrD3DResourceState> state)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, sampleCnt))
        , fStencilBits(0)
        , fBackend(GrBackendApi::kDirect3D)
        , fD3DInfo(d3dInfo, state.release()) {}
#endif
#ifdef SK_GL
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrGLFramebufferInfo& glInfo)
        : fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fBackend(GrBackendApi::kOpenGL)
        , fGLInfo(glInfo) {
    fIsValid = SkToBool(glInfo.fFormat); // the glInfo must have a valid format
}
#endif

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrMockRenderTargetInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, sampleCnt))
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
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            fGLInfo = that.fGLInfo;
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            fD3DInfo.assign(that.fD3DInfo, this->isValid());
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
    fIsValid = that.fIsValid;
    return *this;
}

#ifdef SK_DAWN
bool GrBackendRenderTarget::getDawnRenderTargetInfo(GrDawnRenderTargetInfo* outInfo) const {
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

#ifdef SK_DIRECT3D
bool GrBackendRenderTarget::getD3DTextureResourceInfo(GrD3DTextureResourceInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        *outInfo = fD3DInfo.snapTextureResourceInfo();
        return true;
    }
    return false;
}

void GrBackendRenderTarget::setD3DResourceState(GrD3DResourceStateEnum state) {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        fD3DInfo.setResourceState(state);
    }
}

sk_sp<GrD3DResourceState> GrBackendRenderTarget::getGrD3DResourceState() const {
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        return fD3DInfo.getGrD3DResourceState();
    }
    return nullptr;
}
#endif

#ifdef SK_GL
bool GrBackendRenderTarget::getGLFramebufferInfo(GrGLFramebufferInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        *outInfo = fGLInfo;
        return true;
    }
    return false;
}
#endif

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
                SkASSERT(info.fFormat == info.fYcbcrConversionInfo.fFormat);
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D: {
            auto info = fD3DInfo.snapTextureResourceInfo();
            return GrBackendFormat::MakeDxgi(info.fFormat);
        }
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn: {
            GrDawnRenderTargetInfo dawnInfo;
            SkAssertResult(this->getDawnRenderTargetInfo(&dawnInfo));
            return GrBackendFormat::MakeDawn(dawnInfo.fFormat);
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
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return r0.fD3DInfo == r1.fD3DInfo;
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
