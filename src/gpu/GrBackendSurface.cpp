/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBackendSurfaceMutableStateImpl.h"
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

static GrTextureType gl_target_to_gr_target(GrGLenum target) {
    switch (target) {
        case GR_GL_TEXTURE_NONE:
            return GrTextureType::kNone;
        case GR_GL_TEXTURE_2D:
            return  GrTextureType::k2D;
        case GR_GL_TEXTURE_RECTANGLE:
            return GrTextureType::kRectangle;
        case GR_GL_TEXTURE_EXTERNAL:
            return GrTextureType::kExternal;
        default:
            SkUNREACHABLE;
    }
}

GrBackendFormat::GrBackendFormat(GrGLenum format, GrGLenum target)
        : fBackend(GrBackendApi::kOpenGL)
        , fValid(true)
        , fGLFormat(format)
        , fTextureType(gl_target_to_gr_target(target)) {}

GrGLFormat GrBackendFormat::asGLFormat() const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        return GrGLFormatFromGLEnum(fGLFormat);
    }
    return GrGLFormat::kUnknown;
}
#endif

#ifdef SK_VULKAN
GrBackendFormat GrBackendFormat::MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo,
                                        bool willUseDRMFormatModifiers) {
    SkASSERT(ycbcrInfo.isValid());
    return GrBackendFormat(ycbcrInfo.fFormat, ycbcrInfo, willUseDRMFormatModifiers);
}

GrBackendFormat::GrBackendFormat(VkFormat vkFormat, const GrVkYcbcrConversionInfo& ycbcrInfo,
                                 bool willUseDRMFormatModifiers)
        : fBackend(GrBackendApi::kVulkan)
        , fValid(true)
        , fTextureType(GrTextureType::k2D) {
    fVk.fFormat = vkFormat;
    fVk.fYcbcrConversionInfo = ycbcrInfo;
    if ((fVk.fYcbcrConversionInfo.isValid() && fVk.fYcbcrConversionInfo.fExternalFormat) ||
        willUseDRMFormatModifiers) {
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
#endif

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

GrBackendFormat::GrBackendFormat(GrColorType colorType, SkImage::CompressionType compression,
                                 bool isStencilFormat)
        : fBackend(GrBackendApi::kMock)
        , fValid(true)
        , fTextureType(GrTextureType::k2D) {
    fMock.fColorType = colorType;
    fMock.fCompressionType = compression;
    fMock.fIsStencilFormat = isStencilFormat;
    SkASSERT(this->validateMock());
}

uint32_t GrBackendFormat::channelMask() const {
    if (!this->isValid()) {
        return 0;
    }
    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return GrGLFormatChannels(GrGLFormatFromGLEnum(fGLFormat));
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return GrVkFormatChannels(fVk.fFormat);
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return GrMtlFormatChannels(fMtlFormat);
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            return GrDawnFormatChannels(fDawnFormat);
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return GrDxgiFormatChannels(fDxgiFormat);
#endif
        case GrBackendApi::kMock:
            return GrColorTypeChannelFlags(fMock.fColorType);

        default:
            return 0;
    }
}

GrColorFormatDesc GrBackendFormat::desc() const {
    if (!this->isValid()) {
        return GrColorFormatDesc::MakeInvalid();
    }
    switch (fBackend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL:
            return GrGLFormatDesc(GrGLFormatFromGLEnum(fGLFormat));
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return GrVkFormatDesc(fVk.fFormat);
#endif
#ifdef SK_METAL
       case GrBackendApi::kMetal:
            return GrMtlFormatDesc(fMtlFormat);
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn:
            return GrDawnFormatDesc(fDawnFormat);
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return GrDxgiFormatDesc(fDxgiFormat);
#endif
        case GrBackendApi::kMock:
            return GrGetColorTypeDesc(fMock.fColorType);

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

#ifdef SK_DEBUG
bool GrBackendFormat::validateMock() const {
    int trueStates = 0;
    if (fMock.fCompressionType != SkImage::CompressionType::kNone) {
        trueStates++;
    }
    if (fMock.fColorType != GrColorType::kUnknown) {
        trueStates++;
    }
    if (fMock.fIsStencilFormat) {
        trueStates++;
    }
    return trueStates == 1;
}
#endif

GrColorType GrBackendFormat::asMockColorType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fColorType;
    }

    return GrColorType::kUnknown;
}

SkImage::CompressionType GrBackendFormat::asMockCompressionType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fCompressionType;
    }

    return SkImage::CompressionType::kNone;
}

bool GrBackendFormat::isMockStencilFormat() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fIsStencilFormat;
    }

    return false;
}

GrBackendFormat GrBackendFormat::makeTexture2D() const {
    GrBackendFormat copy = *this;
#ifdef SK_VULKAN
    if (const GrVkYcbcrConversionInfo* ycbcrInfo = this->getVkYcbcrConversionInfo()) {
        if (ycbcrInfo->isValid()) {
            // If we have a ycbcr we remove it from the backend format and set the VkFormat to
            // R8G8B8A8_UNORM
            SkASSERT(copy.fBackend == GrBackendApi::kVulkan);
            copy.fVk.fYcbcrConversionInfo = GrVkYcbcrConversionInfo();
            copy.fVk.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }
    }
#endif
    copy.fTextureType = GrTextureType::k2D;
    return copy;
}

GrBackendFormat GrBackendFormat::MakeMock(GrColorType colorType,
                                          SkImage::CompressionType compression,
                                          bool isStencilFormat) {
    return GrBackendFormat(colorType, compression, isStencilFormat);
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

#if defined(SK_DEBUG) || GR_TEST_UTILS
#include "include/core/SkString.h"

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
GrBackendTexture::GrBackendTexture() : fIsValid(false) {}

#ifdef SK_DAWN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrDawnTextureInfo& dawnInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipmapped(GrMipmapped(dawnInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kDawn)
        , fTextureType(GrTextureType::k2D)
        , fDawnInfo(dawnInfo) {}
#endif

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width, int height, const GrVkImageInfo& vkInfo)
        : GrBackendTexture(width, height, vkInfo,
                           sk_sp<GrBackendSurfaceMutableStateImpl>(
                                   new GrBackendSurfaceMutableStateImpl(
                                        vkInfo.fImageLayout, vkInfo.fCurrentQueueFamily))) {}

static const VkImageUsageFlags kDefaultUsageFlags =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

// We don't know if the backend texture is made renderable or not, so we default the usage flags
// to include color attachment as well.
static const VkImageUsageFlags kDefaultTexRTUsageFlags =
        kDefaultUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

static GrVkImageInfo apply_default_usage_flags(const GrVkImageInfo& info,
                                               VkImageUsageFlags defaultFlags) {
    if (info.fImageUsageFlags == 0) {
        GrVkImageInfo newInfo = info;
        newInfo.fImageUsageFlags = defaultFlags;
        return newInfo;
    }
    return info;
}

static GrTextureType vk_image_info_to_texture_type(const GrVkImageInfo& info) {
    if ((info.fYcbcrConversionInfo.isValid() && info.fYcbcrConversionInfo.fExternalFormat != 0) ||
        info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        return GrTextureType::kExternal;
    }
    return GrTextureType::k2D;
}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipmapped(GrMipmapped(vkInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kVulkan)
        , fTextureType(vk_image_info_to_texture_type(vkInfo))
        , fVkInfo(apply_default_usage_flags(vkInfo, kDefaultTexRTUsageFlags))
        , fMutableState(std::move(mutableState)) {}
#endif

#ifdef SK_GL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrGLTextureInfo glInfo,
                                   sk_sp<GrGLTextureParameters> params)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kOpenGL)
        , fTextureType(gl_target_to_gr_target(glInfo.fTarget))
        , fGLInfo(glInfo, params.release()) {}

sk_sp<GrGLTextureParameters> GrBackendTexture::getGLTextureParams() const {
    if (fBackend != GrBackendApi::kOpenGL) {
        return nullptr;
    }
    return fGLInfo.refParameters();
}
#endif

#ifdef SK_METAL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrMtlTextureInfo& mtlInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kMetal)
        , fTextureType(GrTextureType::k2D)
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
        , fMipmapped(GrMipmapped(d3dInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kDirect3D)
        , fTextureType(GrTextureType::k2D)
        , fD3DInfo(d3dInfo, state.release()) {}
#endif

#ifdef SK_GL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrGLTextureInfo& glInfo)
        : GrBackendTexture(width, height, mipmapped, glInfo, sk_make_sp<GrGLTextureParameters>()) {
    // Make no assumptions about client's texture's parameters.
    this->glTextureParametersModified();
}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrMockTextureInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kMock)
        , fTextureType(GrTextureType::k2D)
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
    fMipmapped = that.fMipmapped;
    fBackend = that.fBackend;
    fTextureType = that.fTextureType;

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
    fMutableState = that.fMutableState;
    fIsValid = true;
    return *this;
}

sk_sp<GrBackendSurfaceMutableStateImpl> GrBackendTexture::getMutableState() const {
    return fMutableState;
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

#ifdef SK_VULKAN
bool GrBackendTexture::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = fVkInfo.snapImageInfo(fMutableState.get());
        return true;
    }
    return false;
}

void GrBackendTexture::setVkImageLayout(VkImageLayout layout) {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        fMutableState->setImageLayout(layout);
    }
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

#ifdef SK_GL
bool GrBackendTexture::getGLTextureInfo(GrGLTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kOpenGL == fBackend) {
        *outInfo = fGLInfo.info();
        return true;
    }
    else if (this->isValid() && GrBackendApi::kMock == fBackend) {
        // Hack! This allows some blink unit tests to work when using the Mock GrContext.
        // Specifically, tests that rely on CanvasResourceProviderTextureGpuMemoryBuffer.
        // If that code ever goes away (or ideally becomes backend-agnostic), this can go away.
        *outInfo = GrGLTextureInfo{ GR_GL_TEXTURE_2D,
                                    static_cast<GrGLuint>(fMockInfo.id()),
                                    GR_GL_RGBA8 };
        return true;
    }
    return false;
}

void GrBackendTexture::glTextureParametersModified() {
    if (this->isValid() && fBackend == GrBackendApi::kOpenGL) {
        fGLInfo.parameters()->invalidate();
    }
}
#endif

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

void GrBackendTexture::setMutableState(const GrBackendSurfaceMutableState& state) {
    fMutableState->set(state);
}

bool GrBackendTexture::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
#ifdef SK_VULKAN
    if (this->backend() == GrBackendApi::kVulkan) {
        return fVkInfo.isProtected();
    }
#endif
    return false;
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
            return fVkInfo.snapImageInfo(fMutableState.get()).fImage ==
                   that.fVkInfo.snapImageInfo(that.fMutableState.get()).fImage;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return this->fMtlInfo.fTexture == that.fMtlInfo.fTexture;
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return fD3DInfo.snapTextureResourceInfo().fResource ==
                    that.fD3DInfo.snapTextureResourceInfo().fResource;
#endif
#ifdef SK_DAWN
        case GrBackendApi::kDawn: {
            return this->fDawnInfo.fTexture.Get() == that.fDawnInfo.fTexture.Get();
        }
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
            auto info = fVkInfo.snapImageInfo(fMutableState.get());
            bool usesDRMModifier = info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
            if (info.fYcbcrConversionInfo.isValid()) {
                SkASSERT(info.fFormat == info.fYcbcrConversionInfo.fFormat);
                return GrBackendFormat::MakeVk(info.fYcbcrConversionInfo, usesDRMModifier);
            }
            return GrBackendFormat::MakeVk(info.fFormat, usesDRMModifier);
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
        t0.fMipmapped != t1.fMipmapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    // For our tests when checking equality we are assuming the both backendTexture objects will
    // be using the same mutable state object.
    if (t0.fMutableState != t1.fMutableState) {
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

GrBackendRenderTarget::GrBackendRenderTarget() : fIsValid(false) {}


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

#ifdef SK_VULKAN
static GrVkImageInfo resolve_vkii_sample_count(const GrVkImageInfo& vkII, int sidebandSampleCnt) {
    auto result = vkII;
    result.fSampleCount = std::max({vkII.fSampleCount,
                                    static_cast<uint32_t>(sidebandSampleCnt),
                                    1U});
    return result;
}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, resolve_vkii_sample_count(vkInfo, sampleCnt)) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, vkInfo,
                                sk_sp<GrBackendSurfaceMutableStateImpl>(
                                        new GrBackendSurfaceMutableStateImpl(
                                                vkInfo.fImageLayout, vkInfo.fCurrentQueueFamily))) {}

static const VkImageUsageFlags kDefaultRTUsageFlags =
        kDefaultUsageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             const GrVkImageInfo& vkInfo,
                                             sk_sp<GrBackendSurfaceMutableStateImpl> mutableState)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1U, vkInfo.fSampleCount))
        , fStencilBits(0)  // We always create stencil buffers internally for vulkan
        , fBackend(GrBackendApi::kVulkan)
        , fVkInfo(apply_default_usage_flags(vkInfo, kDefaultRTUsageFlags))
        , fMutableState(mutableState) {}
#endif

#ifdef SK_METAL
GrBackendRenderTarget::GrBackendRenderTarget(int width, int height, const GrMtlTextureInfo& mtlInfo)
        : fIsValid(true)
        , fFramebufferOnly(false)  // TODO: set this from mtlInfo.fTexture->framebufferOnly
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, GrMtlTextureInfoSampleCount(mtlInfo)))
        , fStencilBits(0)
        , fBackend(GrBackendApi::kMetal)
        , fMtlInfo(mtlInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width, int height,
                                             int sampleCount,
                                             const GrMtlTextureInfo& mtlInfo)
        : GrBackendRenderTarget(width, height, mtlInfo) {
    fSampleCnt = sampleCount;
}
#endif

#ifdef SK_DIRECT3D
GrBackendRenderTarget::GrBackendRenderTarget(int width, int height,
                                             const GrD3DTextureResourceInfo& d3dInfo)
        : GrBackendRenderTarget(
                width, height, d3dInfo,
                sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                        static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState)))) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             const GrD3DTextureResourceInfo& d3dInfo,
                                             sk_sp<GrD3DResourceState> state)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1U, d3dInfo.fSampleCount))
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
#ifdef SK_DIRECT3D
    if (this->isValid() && GrBackendApi::kDirect3D == fBackend) {
        fD3DInfo.cleanup();
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
    fMutableState = that.fMutableState;
    fIsValid = that.fIsValid;
    return *this;
}

sk_sp<GrBackendSurfaceMutableStateImpl> GrBackendRenderTarget::getMutableState() const {
    return fMutableState;
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

#ifdef SK_VULKAN
bool GrBackendRenderTarget::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = fVkInfo.snapImageInfo(fMutableState.get());
        return true;
    }
    return false;
}

void GrBackendRenderTarget::setVkImageLayout(VkImageLayout layout) {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        fMutableState->setImageLayout(layout);
    }
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
            auto info = fVkInfo.snapImageInfo(fMutableState.get());
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

void GrBackendRenderTarget::setMutableState(const GrBackendSurfaceMutableState& state) {
    fMutableState->set(state);
}

bool GrBackendRenderTarget::isProtected() const {
    if (!this->isValid() || this->backend() != GrBackendApi::kVulkan) {
        return false;
    }
#ifdef SK_VULKAN
    return fVkInfo.isProtected();
#else
    return false;
#endif
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
