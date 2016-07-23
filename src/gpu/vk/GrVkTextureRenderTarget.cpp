/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTextureRenderTarget.h"

#include "GrRenderTargetPriv.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkUtil.h"

#include "SkMipMap.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkTextureRenderTarget* GrVkTextureRenderTarget::Create(GrVkGpu* gpu,
                                                         const GrSurfaceDesc& desc,
                                                         const GrVkImageInfo& info,
                                                         SkBudgeted budgeted,
                                                         GrVkImage::Wrapped wrapped) {
    VkImage image = info.fImage;
    // Create the texture ImageView
    uint32_t mipLevels = 1;
    //TODO: does a mipmapped textureRenderTarget make sense?
    //if (desc.fIsMipMapped) {
    //    mipLevels = SkMipMap::ComputeLevelCount(this->width(), this->height()) + 1;
    //}
    const GrVkImageView* imageView = GrVkImageView::Create(gpu, image, info.fFormat,
                                                           GrVkImageView::kColor_Type, mipLevels);
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
        msImageDesc.fUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        msImageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (!GrVkImage::InitImageInfo(gpu, msImageDesc, &msInfo)) {
            return nullptr;
        }

        // Set color attachment image
        colorImage = msInfo.fImage;

        // Create resolve attachment view if necessary.
        // If the format matches, this is the same as the texture imageView.
        if (pixelFormat == info.fFormat) {
            resolveAttachmentView = imageView;
            resolveAttachmentView->ref();
        } else {
            resolveAttachmentView = GrVkImageView::Create(gpu, image, pixelFormat,
                                                          GrVkImageView::kColor_Type, 1);
            if (!resolveAttachmentView) {
                GrVkImage::DestroyImageInfo(gpu, &msInfo);
                imageView->unref(gpu);
                return nullptr;
            }
        }
    } else {
        // Set color attachment image
        colorImage = info.fImage;
    }

    const GrVkImageView* colorAttachmentView;
    // Get color attachment view.
    // If the format matches and there's no multisampling,
    // this is the same as the texture imageView
    if (pixelFormat == info.fFormat && !resolveAttachmentView) {
        colorAttachmentView = imageView;
        colorAttachmentView->ref();
    } else {
        colorAttachmentView = GrVkImageView::Create(gpu, colorImage, pixelFormat,
                                                    GrVkImageView::kColor_Type, 1);
        if (!colorAttachmentView) {
            if (desc.fSampleCnt) {
                resolveAttachmentView->unref(gpu);
                GrVkImage::DestroyImageInfo(gpu, &msInfo);
            }
            imageView->unref(gpu);
            return nullptr;
        }
    }
    GrVkTextureRenderTarget* texRT;
    if (desc.fSampleCnt) {
        if (GrVkImage::kNot_Wrapped == wrapped) {
            texRT = new GrVkTextureRenderTarget(gpu, budgeted, desc,
                                                info, imageView, msInfo,
                                                colorAttachmentView,
                                                resolveAttachmentView);
        } else {
            texRT = new GrVkTextureRenderTarget(gpu, desc,
                                                info, imageView, msInfo,
                                                colorAttachmentView,
                                                resolveAttachmentView, wrapped);
        }
    } else {
        if (GrVkImage::kNot_Wrapped == wrapped) {
            texRT = new GrVkTextureRenderTarget(gpu, budgeted, desc,
                                                info, imageView,
                                                colorAttachmentView);
        } else {
            texRT = new GrVkTextureRenderTarget(gpu, desc,
                                                info, imageView,
                                                colorAttachmentView, wrapped);
        }
    }
    return texRT;
}

GrVkTextureRenderTarget*
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

    GrVkTextureRenderTarget* trt = Create(gpu, desc, info, budgeted, GrVkImage::kNot_Wrapped);
    if (!trt) {
        GrVkImage::DestroyImageInfo(gpu, &info);
    }

    return trt;
}

GrVkTextureRenderTarget*
GrVkTextureRenderTarget::CreateWrappedTextureRenderTarget(GrVkGpu* gpu,
                                                          const GrSurfaceDesc& desc,
                                                          GrWrapOwnership ownership,
                                                          const GrVkImageInfo* info) {
    SkASSERT(info);
    // Wrapped textures require both image and allocation (because they can be mapped)
    SkASSERT(VK_NULL_HANDLE != info->fImage && VK_NULL_HANDLE != info->fAlloc.fMemory);

    GrVkImage::Wrapped wrapped = kBorrow_GrWrapOwnership == ownership ? GrVkImage::kBorrowed_Wrapped
                                                                      : GrVkImage::kAdopted_Wrapped;

    GrVkTextureRenderTarget* trt = Create(gpu, desc, *info, SkBudgeted::kNo, wrapped);

    return trt;
}
