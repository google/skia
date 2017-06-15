/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTextureRenderTarget.h"

#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkUtil.h"

#include "SkMipMap.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 const GrSurfaceDesc& desc,
                                                 const GrVkImageInfo& info,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageInfo& msaaInfo,
                                                 const GrVkImageView* colorAttachmentView,
                                                 const GrVkImageView* resolveAttachmentView)
        : GrSurface(gpu, desc)
        , GrVkImage(info, GrVkImage::kNot_Wrapped)
        , GrVkTexture(gpu, desc, info, texView, GrVkImage::kNot_Wrapped)
        , GrVkRenderTarget(gpu, desc, info, msaaInfo, colorAttachmentView,
                           resolveAttachmentView, GrVkImage::kNot_Wrapped) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 const GrSurfaceDesc& desc,
                                                 const GrVkImageInfo& info,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageView* colorAttachmentView)
        : GrSurface(gpu, desc)
        , GrVkImage(info, GrVkImage::kNot_Wrapped)
        , GrVkTexture(gpu, desc, info, texView, GrVkImage::kNot_Wrapped)
        , GrVkRenderTarget(gpu, desc, info, colorAttachmentView, GrVkImage::kNot_Wrapped) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 const GrSurfaceDesc& desc,
                                                 const GrVkImageInfo& info,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageInfo& msaaInfo,
                                                 const GrVkImageView* colorAttachmentView,
                                                 const GrVkImageView* resolveAttachmentView,
                                                 GrVkImage::Wrapped wrapped)
        : GrSurface(gpu, desc)
        , GrVkImage(info, wrapped)
        , GrVkTexture(gpu, desc, info, texView, wrapped)
        , GrVkRenderTarget(gpu, desc, info, msaaInfo, colorAttachmentView,
                           resolveAttachmentView, wrapped) {
    this->registerWithCacheWrapped();
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 const GrSurfaceDesc& desc,
                                                 const GrVkImageInfo& info,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageView* colorAttachmentView,
                                                 GrVkImage::Wrapped wrapped)
        : GrSurface(gpu, desc)
        , GrVkImage(info, wrapped)
        , GrVkTexture(gpu, desc, info, texView, wrapped)
        , GrVkRenderTarget(gpu, desc, info, colorAttachmentView, wrapped) {
    this->registerWithCacheWrapped();
}


sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::Make(GrVkGpu* gpu,
                                                             const GrSurfaceDesc& desc,
                                                             const GrVkImageInfo& info,
                                                             SkBudgeted budgeted,
                                                             GrVkImage::Wrapped wrapped) {
    VkImage image = info.fImage;
    // Create the texture ImageView
    const GrVkImageView* imageView = GrVkImageView::Create(gpu, image, info.fFormat,
                                                           GrVkImageView::kColor_Type,
                                                           info.fLevelCount);
    if (!imageView) {
        return nullptr;
    }

    VkFormat pixelFormat;
    GrPixelConfigToVkFormat(desc.fConfig, &pixelFormat);

    VkImage colorImage;

    // create msaa surface if necessary
    GrVkImageInfo msInfo;
    const GrVkImageView* resolveAttachmentView = nullptr;
    if (desc.fSampleCnt) {
        GrVkImage::ImageDesc msImageDesc;
        msImageDesc.fImageType = VK_IMAGE_TYPE_2D;
        msImageDesc.fFormat = pixelFormat;
        msImageDesc.fWidth = desc.fWidth;
        msImageDesc.fHeight = desc.fHeight;
        msImageDesc.fLevels = 1;
        msImageDesc.fSamples = desc.fSampleCnt;
        msImageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        msImageDesc.fUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        msImageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (!GrVkImage::InitImageInfo(gpu, msImageDesc, &msInfo)) {
            imageView->unref(gpu);
            return nullptr;
        }

        // Set color attachment image
        colorImage = msInfo.fImage;

        // Create resolve attachment view.
        resolveAttachmentView = GrVkImageView::Create(gpu, image, pixelFormat,
                                                      GrVkImageView::kColor_Type,
                                                      info.fLevelCount);
        if (!resolveAttachmentView) {
            GrVkImage::DestroyImageInfo(gpu, &msInfo);
            imageView->unref(gpu);
            return nullptr;
        }
    } else {
        // Set color attachment image
        colorImage = info.fImage;
    }

    const GrVkImageView* colorAttachmentView = GrVkImageView::Create(gpu, colorImage, pixelFormat,
                                                                     GrVkImageView::kColor_Type, 1);
    if (!colorAttachmentView) {
        if (desc.fSampleCnt) {
            resolveAttachmentView->unref(gpu);
            GrVkImage::DestroyImageInfo(gpu, &msInfo);
        }
        imageView->unref(gpu);
        return nullptr;
    }

    sk_sp<GrVkTextureRenderTarget> texRT;
    if (desc.fSampleCnt) {
        if (GrVkImage::kNot_Wrapped == wrapped) {
            texRT = sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                                                      gpu, budgeted, desc,
                                                      info, imageView, msInfo,
                                                      colorAttachmentView,
                                                      resolveAttachmentView));
        } else {
            texRT = sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                                                        gpu, desc,
                                                        info, imageView, msInfo,
                                                        colorAttachmentView,
                                                        resolveAttachmentView, wrapped));
        }
    } else {
        if (GrVkImage::kNot_Wrapped == wrapped) {
            texRT = sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                                                        gpu, budgeted, desc,
                                                        info, imageView,
                                                        colorAttachmentView));
        } else {
            texRT = sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                                                        gpu, desc,
                                                        info, imageView,
                                                        colorAttachmentView, wrapped));
        }
    }
    return texRT;
}

sk_sp<GrVkTextureRenderTarget>
GrVkTextureRenderTarget::CreateNewTextureRenderTarget(GrVkGpu* gpu,
                                                      SkBudgeted budgeted,
                                                      const GrSurfaceDesc& desc,
                                                      const GrVkImage::ImageDesc& imageDesc) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<GrVkTextureRenderTarget> trt = Make(gpu, desc, info, budgeted, GrVkImage::kNot_Wrapped);
    if (!trt) {
        GrVkImage::DestroyImageInfo(gpu, &info);
    }

    return trt;
}

sk_sp<GrVkTextureRenderTarget>
GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(GrVkGpu* gpu,
                                                        const GrSurfaceDesc& desc,
                                                        GrWrapOwnership ownership,
                                                        const GrVkImageInfo* info) {
    SkASSERT(info);
    // Wrapped textures require both image and allocation (because they can be mapped)
    SkASSERT(VK_NULL_HANDLE != info->fImage && VK_NULL_HANDLE != info->fAlloc.fMemory);

    GrVkImage::Wrapped wrapped = kBorrow_GrWrapOwnership == ownership ? GrVkImage::kBorrowed_Wrapped
                                                                      : GrVkImage::kAdopted_Wrapped;

    return Make(gpu, desc, *info, SkBudgeted::kNo, wrapped);
}

bool GrVkTextureRenderTarget::updateForMipmap(GrVkGpu* gpu, const GrVkImageInfo& newInfo) {
    VkFormat pixelFormat;
    GrPixelConfigToVkFormat(this->config(), &pixelFormat);
    if (this->numStencilSamples()) {
        const GrVkImageView* resolveAttachmentView =
                GrVkImageView::Create(gpu,
                                      newInfo.fImage,
                                      pixelFormat,
                                      GrVkImageView::kColor_Type,
                                      newInfo.fLevelCount);
        if (!resolveAttachmentView) {
            return false;
        }
        fResolveAttachmentView->unref(gpu);
        fResolveAttachmentView = resolveAttachmentView;
    } else {
        const GrVkImageView* colorAttachmentView = GrVkImageView::Create(gpu,
                                                                         newInfo.fImage,
                                                                         pixelFormat,
                                                                         GrVkImageView::kColor_Type,
                                                                         1);
        if (!colorAttachmentView) {
            return false;
        }
        fColorAttachmentView->unref(gpu);
        fColorAttachmentView = colorAttachmentView;
    }

    this->createFramebuffer(gpu);
    return true;
}

size_t GrVkTextureRenderTarget::onGpuMemorySize() const {
    // The plus 1 is to account for the resolve texture.
    int numColorSamples = this->numColorSamples() + 1;
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  numColorSamples,  // TODO: this still correct?
                                  this->texturePriv().hasMipMaps());
}
