/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkAttachment.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkAttachment::GrVkAttachment(GrVkGpu* gpu,
                               SkISize dimensions,
                               UsageFlags supportedUsages,
                               const GrVkImageInfo& info,
                               sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                               sk_sp<const GrVkImageView> framebufferView,
                               sk_sp<const GrVkImageView> textureView,
                               SkBudgeted budgeted)
        : GrAttachment(gpu, dimensions, supportedUsages, info.fSampleCount, GrMipmapped::kNo,
                       info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView)) {
    this->registerWithCache(budgeted);
}

GrVkAttachment::GrVkAttachment(GrVkGpu* gpu,
                               SkISize dimensions,
                               UsageFlags supportedUsages,
                               const GrVkImageInfo& info,
                               sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                               sk_sp<const GrVkImageView> framebufferView,
                               sk_sp<const GrVkImageView> textureView,
                               GrBackendObjectOwnership ownership,
                               GrWrapCacheable cacheable,
                               bool forSecondaryCB)
        : GrAttachment(gpu, dimensions, supportedUsages, info.fSampleCount, GrMipmapped::kNo,
                       info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), ownership, forSecondaryCB)
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView)) {
    this->registerWithCacheWrapped(cacheable);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeStencil(GrVkGpu* gpu,
                                                  SkISize dimensions,
                                                  int sampleCnt,
                                                  VkFormat format) {
    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkAttachment::Make(gpu, dimensions, UsageFlags::kStencilAttachment, sampleCnt, format,
                                /*mipLevels=*/1, vkUsageFlags, GrProtected::kNo, SkBudgeted::kYes);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeMSAA(GrVkGpu* gpu,
                                               SkISize dimensions,
                                               int numSamples,
                                               VkFormat format,
                                               GrProtected isProtected) {
    SkASSERT(numSamples > 1);

    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkAttachment::Make(gpu, dimensions, UsageFlags::kColorAttachment, numSamples, format,
                                /*mipLevels=*/1, vkUsageFlags, isProtected, SkBudgeted::kYes);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeTexture(GrVkGpu* gpu,
                                                  SkISize dimensions,
                                                  VkFormat format,
                                                  uint32_t mipLevels,
                                                  GrRenderable renderable,
                                                  int numSamples,
                                                  SkBudgeted budgeted,
                                                  GrProtected isProtected) {
    UsageFlags usageFlags = UsageFlags::kTexture;
    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= UsageFlags::kColorAttachment;
        vkUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // We always make our render targets support being used as input attachments
        vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return GrVkAttachment::Make(gpu, dimensions, usageFlags, numSamples, format, mipLevels,
                                vkUsageFlags, isProtected, budgeted);
}

static bool make_views(GrVkGpu* gpu, const GrVkImageInfo& info,
                       GrAttachment::UsageFlags attachmentUsages,
                       sk_sp<const GrVkImageView>* framebufferView,
                       sk_sp<const GrVkImageView>* textureView) {
    GrVkImageView::Type viewType;
    if (attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) {
        // If we have stencil usage then we shouldn't have any other usages
        SkASSERT(attachmentUsages == GrAttachment::UsageFlags::kStencilAttachment);
        viewType = GrVkImageView::kStencil_Type;
    } else {
        viewType = GrVkImageView::kColor_Type;
    }

    if (SkToBool(attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) ||
        SkToBool(attachmentUsages & GrAttachment::UsageFlags::kColorAttachment)) {
        // Attachments can only have a mip level of 1
        *framebufferView = GrVkImageView::Make(gpu, info.fImage, info.fFormat, viewType, 1,
                                               info.fYcbcrConversionInfo);
        if (!*framebufferView) {
            return false;
        }
    }

    if (attachmentUsages & GrAttachment::UsageFlags::kTexture) {
        *textureView = GrVkImageView::Make(gpu, info.fImage, info.fFormat, viewType,
                                           info.fLevelCount, info.fYcbcrConversionInfo);
        if (!*textureView) {
            return false;
        }
    }
    return true;
}

sk_sp<GrVkAttachment> GrVkAttachment::Make(GrVkGpu* gpu,
                                           SkISize dimensions,
                                           UsageFlags attachmentUsages,
                                           int sampleCnt,
                                           VkFormat format,
                                           uint32_t mipLevels,
                                           VkImageUsageFlags vkUsageFlags,
                                           GrProtected isProtected,
                                           SkBudgeted budgeted) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = mipLevels;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = vkUsageFlags;
    imageDesc.fIsProtected = isProtected;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!make_views(gpu, info, attachmentUsages, &framebufferView, &textureView)) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkAttachment>(new GrVkAttachment(gpu, dimensions, attachmentUsages, info,
                                                    std::move(mutableState),
                                                    std::move(framebufferView),
                                                    std::move(textureView),
                                                    budgeted));
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeWrapped(
        GrVkGpu* gpu,
        SkISize dimensions,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        UsageFlags attachmentUsages,
        GrWrapOwnership ownership,
        GrWrapCacheable cacheable,
        bool forSecondaryCB) {
    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!forSecondaryCB) {
        if (!make_views(gpu, info, attachmentUsages, &framebufferView, &textureView)) {
            return nullptr;
        }
    }

     GrBackendObjectOwnership backendOwnership = kBorrow_GrWrapOwnership == ownership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;

    return sk_sp<GrVkAttachment>(new GrVkAttachment(gpu, dimensions, attachmentUsages, info,
                                                    std::move(mutableState),
                                                    std::move(framebufferView),
                                                    std::move(textureView),
                                                    backendOwnership, cacheable, forSecondaryCB));
}

GrVkAttachment::~GrVkAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fFramebufferView);
    SkASSERT(!fTextureView);
}

void GrVkAttachment::onRelease() {
    this->releaseImage();
    fFramebufferView.reset();
    fTextureView.reset();

    GrAttachment::onRelease();
}

void GrVkAttachment::onAbandon() {
    this->releaseImage();
    fFramebufferView.reset();
    fTextureView.reset();

    GrAttachment::onAbandon();
}

GrVkGpu* GrVkAttachment::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
