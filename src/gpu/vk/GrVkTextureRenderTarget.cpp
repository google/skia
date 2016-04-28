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

template<typename ResourceType>
GrVkTextureRenderTarget* GrVkTextureRenderTarget::Create(GrVkGpu* gpu,
                                                         ResourceType resourceType,
                                                         const GrSurfaceDesc& desc,
                                                         VkFormat format,
                                                         const GrVkImage::Resource* imageResource) {
    VkImage image = imageResource->fImage;
    // Create the texture ImageView
    uint32_t mipLevels = 1;
    //TODO: does a mipmapped textureRenderTarget make sense?
    //if (desc.fIsMipMapped) {
    //    mipLevels = SkMipMap::ComputeLevelCount(this->width(), this->height());
    //}
    const GrVkImageView* imageView = GrVkImageView::Create(gpu, image, format,
                                                           GrVkImageView::kColor_Type, mipLevels);
    if (!imageView) {
        return nullptr;
    }

    VkFormat pixelFormat;
    GrPixelConfigToVkFormat(desc.fConfig, &pixelFormat);

    VkImage colorImage;

    // create msaa surface if necessary
    const GrVkImage::Resource* msaaImageResource = nullptr;
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

        msaaImageResource = GrVkImage::CreateResource(gpu, msImageDesc);

        if (!msaaImageResource) {
            imageView->unref(gpu);
            return nullptr;
        }

        // Set color attachment image
        colorImage = msaaImageResource->fImage;

        // Create resolve attachment view if necessary.
        // If the format matches, this is the same as the texture imageView.
        if (pixelFormat == format) {
            resolveAttachmentView = imageView;
            resolveAttachmentView->ref();
        } else {
            resolveAttachmentView = GrVkImageView::Create(gpu, image, pixelFormat,
                                                          GrVkImageView::kColor_Type, 1);
            if (!resolveAttachmentView) {
                msaaImageResource->unref(gpu);
                imageView->unref(gpu);
                return nullptr;
            }
        }
    } else {
        // Set color attachment image
        colorImage = imageResource->fImage;
    }

    const GrVkImageView* colorAttachmentView;
    // Get color attachment view.
    // If the format matches and there's no multisampling,
    // this is the same as the texture imageView
    if (pixelFormat == format && !resolveAttachmentView) {
        colorAttachmentView = imageView;
        colorAttachmentView->ref();
    } else {
        colorAttachmentView = GrVkImageView::Create(gpu, colorImage, pixelFormat,
                                                    GrVkImageView::kColor_Type, 1);
        if (!colorAttachmentView) {
            if (msaaImageResource) {
                resolveAttachmentView->unref(gpu);
                msaaImageResource->unref(gpu);
            }
            imageView->unref(gpu);
            return nullptr;
        }
    }
    GrVkTextureRenderTarget* texRT;
    if (msaaImageResource) {
        texRT = new GrVkTextureRenderTarget(gpu, resourceType, desc,
                                            imageResource, imageView, msaaImageResource,
                                            colorAttachmentView,
                                            resolveAttachmentView);
        msaaImageResource->unref(gpu);
    } else {
        texRT = new GrVkTextureRenderTarget(gpu, resourceType, desc,
                                            imageResource, imageView,
                                            colorAttachmentView);
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

    const GrVkImage::Resource* imageRsrc = GrVkImage::CreateResource(gpu, imageDesc);

    if (!imageRsrc) {
        return nullptr;
    }

    GrVkTextureRenderTarget* trt = Create(gpu, budgeted, desc, imageDesc.fFormat,
                                          imageRsrc);
    // Create() will increment the refCount of the image resource if it succeeds
    imageRsrc->unref(gpu);

    return trt;
}

GrVkTextureRenderTarget*
GrVkTextureRenderTarget::CreateWrappedTextureRenderTarget(GrVkGpu* gpu,
                                                          const GrSurfaceDesc& desc,
                                                          GrWrapOwnership ownership,
                                                          VkFormat format,
                                                          const GrVkTextureInfo* info) {
    SkASSERT(info);
    // Wrapped textures require both image and allocation (because they can be mapped)
    SkASSERT(VK_NULL_HANDLE != info->fImage && VK_NULL_HANDLE != info->fAlloc);

    GrVkImage::Resource::Flags flags = (VK_IMAGE_TILING_LINEAR == info->fImageTiling)
                                     ? Resource::kLinearTiling_Flag : Resource::kNo_Flags;

    const GrVkImage::Resource* imageResource;
    if (kBorrow_GrWrapOwnership == ownership) {
        imageResource = new GrVkImage::BorrowedResource(info->fImage,
                                                        info->fAlloc,
                                                        flags,
                                                        info->fFormat);
    } else {
        imageResource = new GrVkImage::Resource(info->fImage, info->fAlloc, flags, info->fFormat);
    }
    if (!imageResource) {
        return nullptr;
    }
    GrVkTextureRenderTarget* trt = Create(gpu, kWrapped, desc, format, imageResource);
    if (trt) {
        trt->fCurrentLayout = info->fImageLayout;
    }
    // Create() will increment the refCount of the image resource if it succeeds
    imageResource->unref(gpu);

    return trt;
}
