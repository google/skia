/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTexture.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrTexturePriv.h"
#include "GrVkUtil.h"
#include "SkMipMap.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkBudgeted budgeted,
                         const GrSurfaceDesc& desc,
                         const GrVkImage::Resource* imageResource,
                         const GrVkImageView* view)
    : GrSurface(gpu, desc)
    , GrVkImage(imageResource)
    , INHERITED(gpu, desc, kSampler2D_GrSLType,
                false) // false because we don't upload MIP data in Vk yet
    , fTextureView(view) {
    this->registerWithCache(budgeted);
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         Wrapped,
                         const GrSurfaceDesc& desc,
                         const GrVkImage::Resource* imageResource,
                         const GrVkImageView* view)
    : GrSurface(gpu, desc)
    , GrVkImage(imageResource)
    , INHERITED(gpu, desc, kSampler2D_GrSLType,
                false) // false because we don't upload MIP data in Vk yet
    , fTextureView(view) {
    this->registerWithCacheWrapped();
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         const GrVkImage::Resource* imageResource,
                         const GrVkImageView* view)
    : GrSurface(gpu, desc)
    , GrVkImage(imageResource)
    , INHERITED(gpu, desc, kSampler2D_GrSLType,
                false) // false because we don't upload MIP data in Vk yet
    , fTextureView(view) {}


template<typename ResourceType>
GrVkTexture* GrVkTexture::Create(GrVkGpu* gpu,
                                 ResourceType type,
                                 const GrSurfaceDesc& desc,
                                 VkFormat format,
                                 const GrVkImage::Resource* imageResource) {
    VkImage image = imageResource->fImage;

    uint32_t mipLevels = 1;
    // TODO: enable when mipLevel loading is implemented in GrVkGpu
    //if (desc.fIsMipMapped) {
    //    mipLevels = SkMipMap::ComputeLevelCount(this->width(), this->height());
    //}
    const GrVkImageView* imageView = GrVkImageView::Create(gpu, image, format,
                                                           GrVkImageView::kColor_Type, mipLevels);
    if (!imageView) {
        return nullptr;
    }

    return new GrVkTexture(gpu, type, desc, imageResource, imageView);
}

GrVkTexture* GrVkTexture::CreateNewTexture(GrVkGpu* gpu, SkBudgeted budgeted,
                                           const GrSurfaceDesc& desc,
                                           const GrVkImage::ImageDesc& imageDesc) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    const GrVkImage::Resource* imageResource = GrVkImage::CreateResource(gpu, imageDesc);
    if (!imageResource) {
        return nullptr;
    }

    GrVkTexture* texture = Create(gpu, budgeted, desc, imageDesc.fFormat, imageResource);
    // Create() will increment the refCount of the image resource if it succeeds
    imageResource->unref(gpu);

    return texture;
}

GrVkTexture* GrVkTexture::CreateWrappedTexture(GrVkGpu* gpu,
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

    GrVkTexture* texture = Create(gpu, kWrapped, desc, format, imageResource);
    if (texture) {
        texture->fCurrentLayout = info->fImageLayout;
    }
    // Create() will increment the refCount of the image resource if it succeeds
    imageResource->unref(gpu);

    return texture;
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTextureView);
}

void GrVkTexture::onRelease() {
    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView->unref(this->getVkGpu());
        fTextureView = nullptr;
    }

    this->releaseImage(this->getVkGpu());

    INHERITED::onRelease();
}

void GrVkTexture::onAbandon() {
    if (fTextureView) {
        fTextureView->unrefAndAbandon();
        fTextureView = nullptr;
    }

    this->abandonImage();
    INHERITED::onAbandon();
}

GrBackendObject GrVkTexture::getTextureHandle() const {
    // Currently just passing back the pointer to the Resource as the handle
    return (GrBackendObject)&fResource;
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

bool GrVkTexture::reallocForMipmap(const GrVkGpu* gpu) {
    const GrVkImage::Resource* oldResource = fResource;

    // Does this even make sense for rendertargets?
    bool renderTarget = SkToBool(fDesc.fFlags & kRenderTarget_GrSurfaceFlag);

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (renderTarget) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t mipLevels = SkMipMap::ComputeLevelCount(this->width(), this->height());
    if (mipLevels == 1) {
        // don't need to do anything for a 1x1 texture
        return false;
    }

    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = oldResource->fFormat;
    imageDesc.fWidth = fDesc.fWidth;
    imageDesc.fHeight = fDesc.fHeight;
    imageDesc.fLevels = mipLevels;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const GrVkImage::Resource* imageResource = GrVkImage::CreateResource(gpu, imageDesc);
    if (!imageResource) {
        return false;
    }

    // have to create a new image view for new resource
    const GrVkImageView* oldView = fTextureView;
    VkImage image = imageResource->fImage;
    const GrVkImageView* textureView = GrVkImageView::Create(gpu, image, imageResource->fFormat,
                                                             GrVkImageView::kColor_Type, mipLevels);
    if (!textureView) {
        imageResource->unref(gpu);
        return false;
    }

    oldResource->unref(gpu);
    oldView->unref(gpu);
    fResource = imageResource;
    fTextureView = textureView;
    this->texturePriv().setMaxMipMapLevel(mipLevels);

    return true;
}
