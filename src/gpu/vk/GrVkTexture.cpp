/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkTexture.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkTextureRenderTarget.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkBudgeted budgeted,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         sk_sp<GrVkImageLayout> layout,
                         const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected)
        , GrVkImage(info, std::move(layout), GrBackendObjectOwnership::kOwned)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected,
                    GrTextureType::k2D, mipMapsStatus)
        , fTextureView(view) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    this->registerWithCache(budgeted);
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        this->setReadOnly();
    }
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu, const GrSurfaceDesc& desc, const GrVkImageInfo& info,
                         sk_sp<GrVkImageLayout> layout, const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus, GrBackendObjectOwnership ownership,
                         GrWrapCacheable cacheable, GrIOType ioType)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected)
        , GrVkImage(info, std::move(layout), ownership)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected,
                    GrTextureType::k2D, mipMapsStatus)
        , fTextureView(view) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         sk_sp<GrVkImageLayout> layout,
                         const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus,
                         GrBackendObjectOwnership ownership)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected)
        , GrVkImage(info, layout, ownership)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, info.fProtected,
                    GrTextureType::k2D, mipMapsStatus)
        , fTextureView(view) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
}

sk_sp<GrVkTexture> GrVkTexture::MakeNewTexture(GrVkGpu* gpu, SkBudgeted budgeted,
                                               const GrSurfaceDesc& desc,
                                               const GrVkImage::ImageDesc& imageDesc,
                                               GrMipMapsStatus mipMapsStatus) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    const GrVkImageView* imageView = GrVkImageView::Create(
            gpu, info.fImage, info.fFormat, GrVkImageView::kColor_Type, info.fLevelCount,
            info.fYcbcrConversionInfo);
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    sk_sp<GrVkImageLayout> layout(new GrVkImageLayout(info.fImageLayout));

    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, budgeted, desc, info, std::move(layout),
                                              imageView, mipMapsStatus));
}

sk_sp<GrVkTexture> GrVkTexture::MakeWrappedTexture(GrVkGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   GrWrapOwnership wrapOwnership,
                                                   GrWrapCacheable cacheable,
                                                   GrIOType ioType,
                                                   const GrVkImageInfo& info,
                                                   sk_sp<GrVkImageLayout> layout) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    const GrVkImageView* imageView = GrVkImageView::Create(
            gpu, info.fImage, info.fFormat, GrVkImageView::kColor_Type, info.fLevelCount,
            info.fYcbcrConversionInfo);
    if (!imageView) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus = info.fLevelCount > 1 ? GrMipMapsStatus::kValid
                                                         : GrMipMapsStatus::kNotAllocated;

    GrBackendObjectOwnership ownership = kBorrow_GrWrapOwnership == wrapOwnership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;
    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, desc, info, std::move(layout), imageView,
                                              mipMapsStatus, ownership, cacheable, ioType));
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTextureView);
}

void GrVkTexture::onRelease() {
    // We're about to be severed from our GrVkResource. If there are "finish" idle procs we have to
    // decide who will handle them. If the resource is still tied to a command buffer we let it
    // handle them. Otherwise, we handle them.
    if (this->hasResource() && this->resource()->isOwnedByCommandBuffer()) {
        this->removeFinishIdleProcs();
    }

    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView->unref(this->getVkGpu());
        fTextureView = nullptr;
    }

    this->releaseImage(this->getVkGpu());

    INHERITED::onRelease();
}

void GrVkTexture::onAbandon() {
    // We're about to be severed from our GrVkResource. If there are "finish" idle procs we have to
    // decide who will handle them. If the resource is still tied to a command buffer we let it
    // handle them. Otherwise, we handle them.
    if (this->hasResource() && this->resource()->isOwnedByCommandBuffer()) {
        this->removeFinishIdleProcs();
    }

    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView->unrefAndAbandon();
        fTextureView = nullptr;
    }

    this->abandonImage();
    INHERITED::onAbandon();
}

GrBackendTexture GrVkTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo, this->grVkImageLayout());
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

const GrVkImageView* GrVkTexture::textureView() {
    return fTextureView;
}

void GrVkTexture::addIdleProc(sk_sp<GrRefCntedCallback> idleProc, IdleState type) {
    INHERITED::addIdleProc(idleProc, type);
    if (type == IdleState::kFinished) {
        if (auto* resource = this->resource()) {
            resource->addIdleProc(this, std::move(idleProc));
        }
    }
}

void GrVkTexture::callIdleProcsOnBehalfOfResource() {
    // If we got here then the resource is being removed from its last command buffer and the
    // texture is idle in the cache. Any kFlush idle procs should already have been called. So
    // the texture and resource should have the same set of procs.
    SkASSERT(this->resource());
    SkASSERT(this->resource()->idleProcCnt() == fIdleProcs.count());
#ifdef SK_DEBUG
    for (int i = 0; i < fIdleProcs.count(); ++i) {
        SkASSERT(fIdleProcs[i] == this->resource()->idleProc(i));
    }
#endif
    fIdleProcs.reset();
    this->resource()->resetIdleProcs();
}

void GrVkTexture::willRemoveLastRefOrPendingIO() {
    if (!fIdleProcs.count()) {
        return;
    }
    // This is called when the GrTexture is purgeable. However, we need to check whether the
    // Resource is still owned by any command buffers. If it is then it will call the proc.
    auto* resource = this->hasResource() ? this->resource() : nullptr;
    bool callFinishProcs = !resource || !resource->isOwnedByCommandBuffer();
    if (callFinishProcs) {
        // Everything must go!
        fIdleProcs.reset();
        resource->resetIdleProcs();
    } else {
        // The procs that should be called on flush but not finish are those that are owned
        // by the GrVkTexture and not the Resource. We do this by copying the resource's array
        // and thereby dropping refs to procs we own but the resource does not.
        SkASSERT(resource);
        fIdleProcs.reset(resource->idleProcCnt());
        for (int i = 0; i < fIdleProcs.count(); ++i) {
            fIdleProcs[i] = resource->idleProc(i);
        }
    }
}

void GrVkTexture::removeFinishIdleProcs() {
    // This should only be called by onRelease/onAbandon when we have already checked for a
    // resource.
    const auto* resource = this->resource();
    SkASSERT(resource);
    SkSTArray<4, sk_sp<GrRefCntedCallback>> procsToKeep;
    int resourceIdx = 0;
    // The idle procs that are common between the GrVkTexture and its Resource should be found in
    // the same order.
    for (int i = 0; i < fIdleProcs.count(); ++i) {
        if (fIdleProcs[i] == resource->idleProc(resourceIdx)) {
            ++resourceIdx;
        } else {
            procsToKeep.push_back(fIdleProcs[i]);
        }
    }
    SkASSERT(resourceIdx == resource->idleProcCnt());
    fIdleProcs = procsToKeep;
}
