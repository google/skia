/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"

#ifdef SK_VULKAN
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/gpu/ganesh/GrVkTypesPriv.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <utility>
#endif

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/ganesh/mtl/GrMtlCppUtil.h"
#endif

#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/d3d/GrD3DResourceState.h"
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"
#endif

#include <algorithm>
#include <new>

namespace skgpu { class MutableTextureState; }

GrBackendFormat::GrBackendFormat() : fValid(false) {}
GrBackendFormat::~GrBackendFormat() = default;

GrBackendFormat::GrBackendFormat(const GrBackendFormat& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fTextureType(that.fTextureType) {
    if (!fValid) {
        return;
    }

    switch (fBackend) {
        case GrBackendApi::kOpenGL:
            fFormatData.reset();
            that.fFormatData->copyTo(fFormatData);
            break;  // fFormatData is sufficient
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

GrBackendFormat::GrBackendFormat(GrColorType colorType, SkTextureCompressionType compression,
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
        case GrBackendApi::kOpenGL:
            return fFormatData->channelMask();
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return skgpu::VkFormatChannels(fVk.fFormat);
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return GrMtlFormatChannels(fMtlFormat);
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
        case GrBackendApi::kOpenGL:
            return fFormatData->desc();
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return GrVkFormatDesc(fVk.fFormat);
#endif
#ifdef SK_METAL
       case GrBackendApi::kMetal:
            return GrMtlFormatDesc(fMtlFormat);
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
    if (fMock.fCompressionType != SkTextureCompressionType::kNone) {
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

SkTextureCompressionType GrBackendFormat::asMockCompressionType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fCompressionType;
    }

    return SkTextureCompressionType::kNone;
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
                                          SkTextureCompressionType compression,
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
        case GrBackendApi::kOpenGL:
            return fFormatData->equal(that.fFormatData.get());
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return fVk.fFormat == that.fVk.fFormat &&
                   fVk.fYcbcrConversionInfo == that.fVk.fYcbcrConversionInfo;
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return fMtlFormat == that.fMtlFormat;
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

SkString GrBackendFormat::toStr() const {
    SkString str;

    if (!fValid) {
        str.append("invalid");
        return str;
    }

    str.appendf("%s-", GrBackendApiToStr(fBackend));

    switch (fBackend) {
        case GrBackendApi::kOpenGL:
            str.append(fFormatData->toString());
            break;
        case GrBackendApi::kVulkan:
#ifdef SK_VULKAN
            str.append(skgpu::VkFormatToStr(fVk.fFormat));
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

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   std::string_view label)
        : GrBackendTexture(
                  width,
                  height,
                  vkInfo,
                  sk_sp<skgpu::MutableTextureStateRef>(new skgpu::MutableTextureStateRef(
                          vkInfo.fImageLayout, vkInfo.fCurrentQueueFamily)),
                  label) {}

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
                                   sk_sp<skgpu::MutableTextureStateRef> mutableState,
                                   std::string_view label)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fLabel(label)
        , fMipmapped(GrMipmapped(vkInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kVulkan)
        , fTextureType(vk_image_info_to_texture_type(vkInfo))
        , fVkInfo(apply_default_usage_flags(vkInfo, kDefaultTexRTUsageFlags))
        , fMutableState(std::move(mutableState)) {}
#endif

#ifdef SK_METAL
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrMtlTextureInfo& mtlInfo,
                                   std::string_view label)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fLabel(label)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kMetal)
        , fTextureType(GrTextureType::k2D)
        , fMtlInfo(mtlInfo) {}
#endif

#ifdef SK_DIRECT3D
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrD3DTextureResourceInfo& d3dInfo,
                                   std::string_view label)
        : GrBackendTexture(width,
                           height,
                           d3dInfo,
                           sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                                   static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState))),
                           label) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrD3DTextureResourceInfo& d3dInfo,
                                   sk_sp<GrD3DResourceState> state,
                                   std::string_view label)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fLabel(label)
        , fMipmapped(GrMipmapped(d3dInfo.fLevelCount > 1))
        , fBackend(GrBackendApi::kDirect3D)
        , fTextureType(GrTextureType::k2D)
        , fD3DInfo(d3dInfo, state.release()) {}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipmapped mipmapped,
                                   const GrMockTextureInfo& mockInfo,
                                   std::string_view label)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fLabel(label)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kMock)
        , fTextureType(GrTextureType::k2D)
        , fMockInfo(mockInfo) {}

GrBackendTexture::~GrBackendTexture() {
    this->cleanup();
}

void GrBackendTexture::cleanup() {
    fTextureData.reset();
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
    if (this == &that) {
        return *this;
    }

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
        case GrBackendApi::kOpenGL:
            fTextureData.reset();
            that.fTextureData->copyTo(fTextureData);
            break;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            fVkInfo = that.fVkInfo;
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

sk_sp<skgpu::MutableTextureStateRef> GrBackendTexture::getMutableState() const {
    return fMutableState;
}

#ifdef SK_VULKAN
bool GrBackendTexture::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
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

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

void GrBackendTexture::setMutableState(const skgpu::MutableTextureState& state) {
    fMutableState->set(state);
}

bool GrBackendTexture::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    if (this->backend() == GrBackendApi::kOpenGL) {
        return fTextureData->isProtected();
    }
#ifdef SK_VULKAN
    if (this->backend() == GrBackendApi::kVulkan) {
        return fVkInfo.fProtected == skgpu::Protected::kYes;
    }
#endif
    if (this->backend() == GrBackendApi::kMock) {
        return fMockInfo.isProtected();
    }

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
        case GrBackendApi::kOpenGL:
            return fTextureData->isSameTexture(that.fTextureData.get());
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
        return fVkInfo.fImage == that.fVkInfo.fImage;
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
        case GrBackendApi::kOpenGL:
            return fTextureData->getBackendFormat();
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            auto info = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
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
        case GrBackendApi::kOpenGL:
            return t0.fTextureData->equal(t1.fTextureData.get());
        case GrBackendApi::kMock:
            return t0.fMockInfo == t1.fMockInfo;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return GrVkImageInfoWithMutableState(t0.fVkInfo, t0.fMutableState.get()) ==
                   GrVkImageInfoWithMutableState(t1.fVkInfo, t0.fMutableState.get());
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return t0.fMtlInfo == t1.fMtlInfo;
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return t0.fD3DInfo == t1.fD3DInfo;
#endif
        default:
            return false;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

GrBackendRenderTarget::GrBackendRenderTarget() : fIsValid(false) {}

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, vkInfo,
                                sk_sp<skgpu::MutableTextureStateRef>(
                                        new skgpu::MutableTextureStateRef(
                                                vkInfo.fImageLayout, vkInfo.fCurrentQueueFamily))) {}

static const VkImageUsageFlags kDefaultRTUsageFlags =
        kDefaultUsageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             const GrVkImageInfo& vkInfo,
                                             sk_sp<skgpu::MutableTextureStateRef> mutableState)
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
        , fBackend(GrBackendApi::kMock)
        , fMockInfo(mockInfo) {}

GrBackendRenderTarget::~GrBackendRenderTarget() {
    this->cleanup();
}

void GrBackendRenderTarget::cleanup() {
    fRTData.reset();
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
    if (this == &that) {
        return *this;
    }

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
            fRTData.reset();
            that.fRTData->copyTo(fRTData);
            break;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            fVkInfo = that.fVkInfo;
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

sk_sp<skgpu::MutableTextureStateRef> GrBackendRenderTarget::getMutableState() const {
    return fMutableState;
}

#ifdef SK_VULKAN
bool GrBackendRenderTarget::getVkImageInfo(GrVkImageInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kVulkan == fBackend) {
        *outInfo = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
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

GrBackendFormat GrBackendRenderTarget::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kOpenGL:
            return fRTData->getBackendFormat();
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            auto info = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
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

void GrBackendRenderTarget::setMutableState(const skgpu::MutableTextureState& state) {
    fMutableState->set(state);
}

bool GrBackendRenderTarget::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    if (this->backend() == GrBackendApi::kOpenGL) {
        return fRTData->isProtected();
    }
#ifdef SK_VULKAN
    if (this->backend() == GrBackendApi::kVulkan) {
        return fVkInfo.fProtected == skgpu::Protected::kYes;
    }
#endif
    if (this->backend() == GrBackendApi::kMock) {
        return fMockInfo.isProtected();
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
        r0.fBackend != r1.fBackend) {
        return false;
    }

    // For our tests when checking equality we are assuming the both backendTexture objects will
    // be using the same mutable state object.
    if (r0.fMutableState != r1.fMutableState) {
        return false;
    }

    switch (r0.fBackend) {
        case GrBackendApi::kOpenGL:
            return r0.fRTData->equal(r1.fRTData.get());
        case GrBackendApi::kMock:
            return r0.fMockInfo == r1.fMockInfo;
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan:
            return GrVkImageInfoWithMutableState(r0.fVkInfo, r0.fMutableState.get()) ==
                   GrVkImageInfoWithMutableState(r1.fVkInfo, r0.fMutableState.get());
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal:
            return r0.fMtlInfo == r1.fMtlInfo;
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            return r0.fD3DInfo == r1.fD3DInfo;
#endif
        default:
            return false;
    }

    SkASSERT(0);
    return false;
}
#endif

GrBackendFormatData::~GrBackendFormatData() {}
GrBackendTextureData::~GrBackendTextureData() {}
GrBackendRenderTargetData::~GrBackendRenderTargetData() {}
