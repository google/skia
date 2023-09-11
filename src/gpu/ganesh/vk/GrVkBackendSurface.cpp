/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/ganesh/GrVkTypesPriv.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

class GrVkBackendFormatData final : public GrBackendFormatData {
public:
    GrVkBackendFormatData(VkFormat format, const GrVkYcbcrConversionInfo& ycbcrInfo)
            : fFormat(format), fYcbcrConversionInfo(ycbcrInfo) {}

    VkFormat asVkFormat() const { return fFormat; }
    const GrVkYcbcrConversionInfo* getYcbcrConversionInfo() const { return &fYcbcrConversionInfo; }

private:
    SkTextureCompressionType compressionType() const override {
        switch (fFormat) {
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
                return SkTextureCompressionType::kETC2_RGB8_UNORM;
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                return SkTextureCompressionType::kBC1_RGB8_UNORM;
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                return SkTextureCompressionType::kBC1_RGBA8_UNORM;
            default:
                return SkTextureCompressionType::kNone;
        }
    }

    size_t bytesPerBlock() const override {
        return skgpu::VkFormatBytesPerBlock(fFormat);
    }

    int stencilBits() const override {
        return skgpu::VkFormatStencilBits(fFormat);
    }

    uint32_t channelMask() const override {
        return skgpu::VkFormatChannels(fFormat);
    }

    GrColorFormatDesc desc() const override {
        return GrVkFormatDesc(fFormat);
    }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kVulkan);
        if (auto otherVk = static_cast<const GrVkBackendFormatData*>(that)) {
            return fFormat == otherVk->fFormat &&
                   fYcbcrConversionInfo == otherVk->fYcbcrConversionInfo;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || GR_TEST_UTILS
        return skgpu::VkFormatToStr(fFormat);
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrVkBackendFormatData>(fFormat, fYcbcrConversionInfo);
    }

    void makeTexture2D() override {
        // If we have a ycbcr we remove it from the backend format and set the VkFormat to
        // R8G8B8A8_UNORM
        if (fYcbcrConversionInfo.isValid()) {
            fYcbcrConversionInfo = GrVkYcbcrConversionInfo();
            fFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kVulkan; }
#endif

    VkFormat fFormat;
    GrVkYcbcrConversionInfo fYcbcrConversionInfo;
};

static const GrVkBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<const GrVkBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeVk(VkFormat format, bool willUseDRMFormatModifiers) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kVulkan,
            GrVkBackendFormatData(format, GrVkYcbcrConversionInfo{}));
}

GrBackendFormat MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo, bool willUseDRMFormatModifiers) {
    SkASSERT(ycbcrInfo.isValid());
    GrTextureType textureType =
            ((ycbcrInfo.isValid() && ycbcrInfo.fExternalFormat) || willUseDRMFormatModifiers)
                    ? GrTextureType::kExternal
                    : GrTextureType::k2D;
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            textureType,
            GrBackendApi::kVulkan,
            GrVkBackendFormatData(ycbcrInfo.fFormat, ycbcrInfo));
}

bool AsVkFormat(const GrBackendFormat& format, VkFormat* vkFormat) {
    SkASSERT(vkFormat);
    if (format.isValid() && format.backend() == GrBackendApi::kVulkan) {
        const GrVkBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        *vkFormat = data->asVkFormat();
        return true;
    }
    return false;
}

const GrVkYcbcrConversionInfo* GetVkYcbcrConversionInfo(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kVulkan) {
        const GrVkBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        return data->getYcbcrConversionInfo();
    }
    return nullptr;
}

}  // namespace GrBackendFormats


class GrVkBackendTextureData final : public GrBackendTextureData {
public:
    GrVkBackendTextureData(const GrVkImageInfo& info,
                           sk_sp<skgpu::MutableTextureStateRef> mutableState = nullptr)
            : fVkInfo(info)
            , fMutableState(mutableState ? std::move(mutableState)
                                         : sk_make_sp<skgpu::MutableTextureStateRef>(
                                                   info.fImageLayout, info.fCurrentQueueFamily)) {}

    const GrVkImageInfo& info() const { return fVkInfo; }

    sk_sp<skgpu::MutableTextureStateRef> getMutableState() const override {
        return fMutableState;
    }
    void setMutableState(const skgpu::MutableTextureState& state) override {
        fMutableState->set(state);
    }

    skgpu::MutableTextureStateRef* mutableState() { return fMutableState.get(); }
    const skgpu::MutableTextureStateRef* mutableState() const { return fMutableState.get(); }

private:
    void copyTo(AnyTextureData& textureData) const override {
        textureData.emplace<GrVkBackendTextureData>(fVkInfo, fMutableState);
    }

    bool isProtected() const override { return fVkInfo.fProtected == skgpu::Protected::kYes; }

    bool equal(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kVulkan);
        if (auto otherVk = static_cast<const GrVkBackendTextureData*>(that)) {
            // For our tests when checking equality we are assuming both backendTexture objects will
            // be using the same mutable state object.
            if (fMutableState != otherVk->fMutableState) {
                return false;
            }
            return GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get()) ==
                   GrVkImageInfoWithMutableState(otherVk->fVkInfo, fMutableState.get());
        }
        return false;
    }

    bool isSameTexture(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kVulkan);
        if (auto otherVk = static_cast<const GrVkBackendTextureData*>(that)) {
            return fVkInfo.fImage == otherVk->fVkInfo.fImage;
        }
        return false;

    }

    GrBackendFormat getBackendFormat() const override {
        auto info = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
        bool usesDRMModifier = info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        if (info.fYcbcrConversionInfo.isValid()) {
            SkASSERT(info.fFormat == info.fYcbcrConversionInfo.fFormat);
            return GrBackendFormats::MakeVk(info.fYcbcrConversionInfo, usesDRMModifier);
        }
        return GrBackendFormats::MakeVk(info.fFormat, usesDRMModifier);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kVulkan; }
#endif

    GrVkImageInfo fVkInfo;
    sk_sp<skgpu::MutableTextureStateRef> fMutableState;
};

static const GrVkBackendTextureData* get_and_cast_data(const GrBackendTexture& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<const GrVkBackendTextureData*>(data);
}

static GrVkBackendTextureData* get_and_cast_data(GrBackendTexture* texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<GrVkBackendTextureData*>(data);
}

static GrTextureType vk_image_info_to_texture_type(const GrVkImageInfo& info) {
    if ((info.fYcbcrConversionInfo.isValid() && info.fYcbcrConversionInfo.fExternalFormat != 0) ||
        info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        return GrTextureType::kExternal;
    }
    return GrTextureType::k2D;
}

static const VkImageUsageFlags kDefaultUsageFlags =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

static const VkImageUsageFlags kDefaultRTUsageFlags =
        kDefaultUsageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

namespace GrBackendTextures {

GrBackendTexture MakeVk(int width,
                        int height,
                        const GrVkImageInfo& vkInfo,
                        std::string_view label) {
    return GrBackendSurfacePriv::MakeGrBackendTexture(
            width,
            height,
            label,
            skgpu::Mipmapped(vkInfo.fLevelCount > 1),
            GrBackendApi::kVulkan,
            vk_image_info_to_texture_type(vkInfo),
            GrVkBackendTextureData(apply_default_usage_flags(vkInfo, kDefaultTexRTUsageFlags)));
}

GrBackendTexture MakeVk(int width,
                        int height,
                        const GrVkImageInfo& vkInfo,
                        sk_sp<skgpu::MutableTextureStateRef> mutableState) {
    return GrBackendSurfacePriv::MakeGrBackendTexture(
            width,
            height,
            /*label=*/{},
            skgpu::Mipmapped(vkInfo.fLevelCount > 1),
            GrBackendApi::kVulkan,
            vk_image_info_to_texture_type(vkInfo),
            GrVkBackendTextureData(apply_default_usage_flags(vkInfo, kDefaultTexRTUsageFlags),
                                   std::move(mutableState)));
}

bool GetVkImageInfo(const GrBackendTexture& tex, GrVkImageInfo* outInfo) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kVulkan) {
        return false;
    }
    const GrVkBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    *outInfo = GrVkImageInfoWithMutableState(data->info(), data->mutableState());
    return true;
}

void SetVkImageLayout(GrBackendTexture* tex, VkImageLayout layout) {
    if (tex && tex->isValid() && tex->backend() == GrBackendApi::kVulkan) {
        GrVkBackendTextureData* data = get_and_cast_data(tex);
        SkASSERT(data);
        data->mutableState()->setImageLayout(layout);
    }
}

}  // namespace GrBackendTextures


class GrVkBackendRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrVkBackendRenderTargetData(const GrVkImageInfo& info,
                                sk_sp<skgpu::MutableTextureStateRef> mutableState = nullptr)
            : fVkInfo(info)
            , fMutableState(mutableState ? std::move(mutableState)
                                         : sk_make_sp<skgpu::MutableTextureStateRef>(
                                                   info.fImageLayout, info.fCurrentQueueFamily)) {}

    const GrVkImageInfo& info() const { return fVkInfo; }

    sk_sp<skgpu::MutableTextureStateRef> getMutableState() const override {
        return fMutableState;
    }
    void setMutableState(const skgpu::MutableTextureState& state) override {
        fMutableState->set(state);
    }

    skgpu::MutableTextureStateRef* mutableState() { return fMutableState.get(); }
    const skgpu::MutableTextureStateRef* mutableState() const { return fMutableState.get(); }

private:
    GrBackendFormat getBackendFormat() const override {
        auto info = GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get());
        if (info.fYcbcrConversionInfo.isValid()) {
            SkASSERT(info.fFormat == info.fYcbcrConversionInfo.fFormat);
            return GrBackendFormats::MakeVk(info.fYcbcrConversionInfo);
        }
        return GrBackendFormats::MakeVk(info.fFormat);
    }

    bool isProtected() const override { return fVkInfo.fProtected == skgpu::Protected::kYes; }

    bool equal(const GrBackendRenderTargetData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kVulkan);
        if (auto otherVk = static_cast<const GrVkBackendRenderTargetData*>(that)) {
            // For our tests when checking equality we are assuming both objects will be using the
            // same mutable state object.
            if (fMutableState != otherVk->fMutableState) {
                return false;
            }
            return GrVkImageInfoWithMutableState(fVkInfo, fMutableState.get()) ==
                   GrVkImageInfoWithMutableState(otherVk->fVkInfo, fMutableState.get());
        }
        return false;
    }

    void copyTo(AnyRenderTargetData& rtData) const override {
        rtData.emplace<GrVkBackendRenderTargetData>(fVkInfo, fMutableState);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kVulkan; }
#endif

    GrVkImageInfo fVkInfo;
    sk_sp<skgpu::MutableTextureStateRef> fMutableState;
};

static const GrVkBackendRenderTargetData* get_and_cast_data(const GrBackendRenderTarget& rt) {
    auto data = GrBackendSurfacePriv::GetBackendData(rt);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<const GrVkBackendRenderTargetData*>(data);
}

static GrVkBackendRenderTargetData* get_and_cast_data(GrBackendRenderTarget* rt) {
    auto data = GrBackendSurfacePriv::GetBackendData(rt);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<GrVkBackendRenderTargetData*>(data);
}

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeVk(int width, int height, const GrVkImageInfo& vkInfo) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(
            width,
            height,
            std::max(1U, vkInfo.fSampleCount),
            /*stencilBits=*/0,
            GrBackendApi::kVulkan,
            /*framebufferOnly=*/false,
            GrVkBackendRenderTargetData(apply_default_usage_flags(vkInfo, kDefaultRTUsageFlags)));
}

GrBackendRenderTarget MakeVk(int width,
                             int height,
                             const GrVkImageInfo& vkInfo,
                             sk_sp<skgpu::MutableTextureStateRef> mutableState) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(
            width,
            height,
            std::max(1U, vkInfo.fSampleCount),
            /*stencilBits=*/0,
            GrBackendApi::kVulkan,
            /*framebufferOnly=*/false,
            GrVkBackendRenderTargetData(apply_default_usage_flags(vkInfo, kDefaultRTUsageFlags),
                                        std::move(mutableState)));
}

bool GetVkImageInfo(const GrBackendRenderTarget& rt, GrVkImageInfo* outInfo) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kVulkan) {
        return false;
    }
    const GrVkBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    *outInfo = GrVkImageInfoWithMutableState(data->info(), data->mutableState());
    return true;
}

void SetVkImageLayout(GrBackendRenderTarget* rt, VkImageLayout layout) {
    if (rt && rt->isValid() && rt->backend() == GrBackendApi::kVulkan) {
        GrVkBackendRenderTargetData* data = get_and_cast_data(rt);
        data->mutableState()->setImageLayout(layout);
    }
}

}  // namespace GrBackendRenderTargets


#if !defined(SK_DISABLE_LEGACY_VK_BACKEND_SURFACE) && defined(SK_VULKAN)
GrBackendFormat GrBackendFormat::MakeVk(VkFormat format, bool willUseDRMFormatModifiers) {
    return GrBackendFormats::MakeVk(format, willUseDRMFormatModifiers);
}

GrBackendFormat GrBackendFormat::MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo,
                                        bool willUseDRMFormatModifiers) {
    return GrBackendFormats::MakeVk(ycbcrInfo, willUseDRMFormatModifiers);
}

bool GrBackendFormat::asVkFormat(VkFormat* format) const {
    return GrBackendFormats::AsVkFormat(*this, format);
}

const GrVkYcbcrConversionInfo* GrBackendFormat::getVkYcbcrConversionInfo() const {
    return GrBackendFormats::GetVkYcbcrConversionInfo(*this);
}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo,
                                   std::string_view label)
        : GrBackendTexture(width,
                           height,
                           label,
                           skgpu::Mipmapped(vkInfo.fLevelCount > 1),
                           GrBackendApi::kVulkan,
                           vk_image_info_to_texture_type(vkInfo),
                           GrVkBackendTextureData(
                                   apply_default_usage_flags(vkInfo, kDefaultTexRTUsageFlags))) {}

bool GrBackendTexture::getVkImageInfo(GrVkImageInfo* outInfo) const {
    return GrBackendTextures::GetVkImageInfo(*this, outInfo);
}

void GrBackendTexture::setVkImageLayout(VkImageLayout layout) {
    GrBackendTextures::SetVkImageLayout(this, layout);
}

GrBackendRenderTarget::GrBackendRenderTarget(int width, int height, const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width,
                                height,
                                std::max(1U, vkInfo.fSampleCount),
                                /*stencilBits=*/0,
                                GrBackendApi::kVulkan,
                                /*framebufferOnly=*/false,
                                GrVkBackendRenderTargetData(vkInfo)) {}

bool GrBackendRenderTarget::getVkImageInfo(GrVkImageInfo* outInfo) const {
    return GrBackendRenderTargets::GetVkImageInfo(*this, outInfo);
}

void GrBackendRenderTarget::setVkImageLayout(VkImageLayout layout) {
    GrBackendRenderTargets::SetVkImageLayout(this, layout);
}
#endif
