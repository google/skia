/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkFramebuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkResourceProvider.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   int sampleCnt,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageInfo& msaaInfo,
                                   sk_sp<GrVkImageLayout> msaaLayout,
                                   const GrVkImageView* colorAttachmentView,
                                   const GrVkImageView* resolveAttachmentView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(layout), GrBackendObjectOwnership::kBorrowed)
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, sampleCnt, info.fProtected)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(new GrVkImage(gpu, msaaInfo, std::move(msaaLayout),
                                   GrBackendObjectOwnership::kOwned))
        , fResolveAttachmentView(resolveAttachmentView)
        , fCachedFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   int sampleCnt,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageInfo& msaaInfo,
                                   sk_sp<GrVkImageLayout> msaaLayout,
                                   const GrVkImageView* colorAttachmentView,
                                   const GrVkImageView* resolveAttachmentView,
                                   GrBackendObjectOwnership ownership)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(layout), ownership)
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, sampleCnt, info.fProtected)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(new GrVkImage(gpu, msaaInfo, std::move(msaaLayout),
                                   GrBackendObjectOwnership::kOwned))
        , fResolveAttachmentView(resolveAttachmentView)
        , fCachedFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(sampleCnt > 1);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageView* colorAttachmentView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(layout), GrBackendObjectOwnership::kBorrowed)
        , GrRenderTarget(gpu, dimensions, 1, info.fProtected)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fCachedFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageView* colorAttachmentView,
                                   GrBackendObjectOwnership ownership)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(layout), ownership)
        , GrRenderTarget(gpu, dimensions, 1, info.fProtected)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fCachedFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {}

GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkRenderPass* renderPass,
                                   VkCommandBuffer secondaryCommandBuffer)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(layout), GrBackendObjectOwnership::kBorrowed, true)
        , GrRenderTarget(gpu, dimensions, 1, info.fProtected)
        , fColorAttachmentView(nullptr)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fCachedFramebuffer(nullptr)
        , fCachedSimpleRenderPass(renderPass)
        , fSecondaryCommandBuffer(secondaryCommandBuffer) {
    SkASSERT(fSecondaryCommandBuffer != VK_NULL_HANDLE);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeWrappedRenderTarget(GrVkGpu* gpu,
                                                                  SkISize dimensions,
                                                                  int sampleCnt,
                                                                  const GrVkImageInfo& info,
                                                                  sk_sp<GrVkImageLayout> layout) {
    SkASSERT(VK_NULL_HANDLE != info.fImage);

    SkASSERT(1 == info.fLevelCount);
    VkFormat pixelFormat = info.fFormat;

    VkImage colorImage;

    // create msaa surface if necessary
    GrVkImageInfo msInfo;
    sk_sp<GrVkImageLayout> msLayout;
    const GrVkImageView* resolveAttachmentView = nullptr;
    if (sampleCnt > 1) {
        GrVkImage::ImageDesc msImageDesc;
        msImageDesc.fImageType = VK_IMAGE_TYPE_2D;
        msImageDesc.fFormat = pixelFormat;
        msImageDesc.fWidth = dimensions.fWidth;
        msImageDesc.fHeight = dimensions.fHeight;
        msImageDesc.fLevels = 1;
        msImageDesc.fSamples = sampleCnt;
        msImageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        msImageDesc.fUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        msImageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        msImageDesc.fIsProtected = info.fProtected;

        if (!GrVkImage::InitImageInfo(gpu, msImageDesc, &msInfo)) {
            return nullptr;
        }

        // Set color attachment image
        colorImage = msInfo.fImage;

        // Create Resolve attachment view
        resolveAttachmentView = GrVkImageView::Create(gpu, info.fImage, pixelFormat,
                                                      GrVkImageView::kColor_Type, 1,
                                                      GrVkYcbcrConversionInfo());
        if (!resolveAttachmentView) {
            GrVkImage::DestroyImageInfo(gpu, &msInfo);
            return nullptr;
        }
        msLayout.reset(new GrVkImageLayout(msInfo.fImageLayout));
    } else {
        // Set color attachment image
        colorImage = info.fImage;
    }

    // Get color attachment view
    const GrVkImageView* colorAttachmentView = GrVkImageView::Create(gpu, colorImage, pixelFormat,
                                                                     GrVkImageView::kColor_Type, 1,
                                                                     GrVkYcbcrConversionInfo());
    if (!colorAttachmentView) {
        if (sampleCnt > 1) {
            resolveAttachmentView->unref();
            GrVkImage::DestroyImageInfo(gpu, &msInfo);
        }
        return nullptr;
    }

    GrVkRenderTarget* vkRT;
    if (sampleCnt > 1) {
        vkRT = new GrVkRenderTarget(gpu, dimensions, sampleCnt, info, std::move(layout), msInfo,
                                    std::move(msLayout), colorAttachmentView,
                                    resolveAttachmentView);
    } else {
        vkRT = new GrVkRenderTarget(gpu, dimensions, info, std::move(layout), colorAttachmentView);
    }

    return sk_sp<GrVkRenderTarget>(vkRT);
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeSecondaryCBRenderTarget(
        GrVkGpu* gpu, SkISize dimensions, const GrVkDrawableInfo& vkInfo) {
    // We only set the few properties of the GrVkImageInfo that we know like layout and format. The
    // others we keep at the default "null" values.
    GrVkImageInfo info;
    info.fImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    info.fFormat = vkInfo.fFormat;

    sk_sp<GrVkImageLayout> layout(new GrVkImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    const GrVkRenderPass* rp =
            gpu->resourceProvider().findCompatibleExternalRenderPass(vkInfo.fCompatibleRenderPass,
                                                                     vkInfo.fColorAttachmentIndex);
    if (!rp) {
        return nullptr;
    }

    if (vkInfo.fSecondaryCommandBuffer == VK_NULL_HANDLE) {
        return nullptr;
    }

    GrVkRenderTarget* vkRT = new GrVkRenderTarget(gpu, dimensions, info, std::move(layout), rp,
                                                  vkInfo.fSecondaryCommandBuffer);

    return sk_sp<GrVkRenderTarget>(vkRT);
}

bool GrVkRenderTarget::completeStencilAttachment() {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    // If we have a previous renderpass or framebuffer it will have been made without stencil, so
    // we set it to null to trigger creating a new one the next time we need it.
    if (fCachedSimpleRenderPass) {
        fCachedSimpleRenderPass->unref();
        fCachedSimpleRenderPass = nullptr;
    }
    if (fCachedFramebuffer) {
        fCachedFramebuffer->unref();
        fCachedFramebuffer = nullptr;
    }
    fCompatibleRPHandle = GrVkResourceProvider::CompatibleRPHandle();
    return true;
}

const GrVkRenderPass* GrVkRenderTarget::getSimpleRenderPass() {
    if (fCachedSimpleRenderPass) {
        return fCachedSimpleRenderPass;
    }
    return this->createSimpleRenderPass();
}

const GrVkRenderPass* GrVkRenderTarget::createSimpleRenderPass() {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    SkASSERT(!fCachedSimpleRenderPass);

    fCachedSimpleRenderPass =
        this->getVkGpu()->resourceProvider().findCompatibleRenderPass(*this, &fCompatibleRPHandle);
    return fCachedSimpleRenderPass;
}

const GrVkFramebuffer* GrVkRenderTarget::getFramebuffer() {
    if (fCachedFramebuffer) {
        return fCachedFramebuffer;
    }
    return this->createFramebuffer();
}

const GrVkFramebuffer* GrVkRenderTarget::createFramebuffer() {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    SkASSERT(!fCachedFramebuffer);

    GrVkGpu* gpu = this->getVkGpu();
    // Stencil attachment view is stored in the base RT stencil attachment
    const GrVkImageView* stencilView = this->stencilAttachmentView();
    const GrVkRenderPass* renderPass = this->getSimpleRenderPass();
    if (!renderPass) {
        return nullptr;
    }
    fCachedFramebuffer = GrVkFramebuffer::Create(gpu, this->width(), this->height(), renderPass,
                                                 fColorAttachmentView, stencilView);
    return fCachedFramebuffer;
}

void GrVkRenderTarget::getAttachmentsDescriptor(
                                           GrVkRenderPass::AttachmentsDescriptor* desc,
                                           GrVkRenderPass::AttachmentFlags* attachmentFlags) const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    desc->fColor.fFormat = this->imageFormat();
    desc->fColor.fSamples = this->numSamples();
    *attachmentFlags = GrVkRenderPass::kColor_AttachmentFlag;
    uint32_t attachmentCount = 1;

    const GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment();
    if (stencil) {
        const GrVkStencilAttachment* vkStencil = static_cast<const GrVkStencilAttachment*>(stencil);
        desc->fStencil.fFormat = vkStencil->vkFormat();
        desc->fStencil.fSamples = vkStencil->numSamples();
#ifdef SK_DEBUG
        if (this->getVkGpu()->caps()->mixedSamplesSupport()) {
            SkASSERT(desc->fStencil.fSamples >= desc->fColor.fSamples);
        } else {
            SkASSERT(desc->fStencil.fSamples == desc->fColor.fSamples);
        }
#endif
        *attachmentFlags |= GrVkRenderPass::kStencil_AttachmentFlag;
        ++attachmentCount;
    }
    desc->fAttachmentCount = attachmentCount;
}

GrVkRenderTarget::~GrVkRenderTarget() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fMSAAImage);
    SkASSERT(!fResolveAttachmentView);
    SkASSERT(!fColorAttachmentView);
    SkASSERT(!fCachedFramebuffer);
    SkASSERT(!fCachedSimpleRenderPass);
}

void GrVkRenderTarget::addResources(GrVkCommandBuffer& commandBuffer) {
    commandBuffer.addResource(this->getFramebuffer());
    commandBuffer.addResource(this->colorAttachmentView());
    commandBuffer.addResource(this->msaaImageResource() ? this->msaaImageResource()
                                                        : this->resource());
    if (this->stencilImageResource()) {
        commandBuffer.addResource(this->stencilImageResource());
        commandBuffer.addResource(this->stencilAttachmentView());
    }
}

void GrVkRenderTarget::releaseInternalObjects() {
    GrVkGpu* gpu = this->getVkGpu();

    if (fMSAAImage) {
        fMSAAImage->releaseImage(gpu);
        fMSAAImage.reset();
    }

    if (fResolveAttachmentView) {
        fResolveAttachmentView->unref();
        fResolveAttachmentView = nullptr;
    }
    if (fColorAttachmentView) {
        fColorAttachmentView->unref();
        fColorAttachmentView = nullptr;
    }
    if (fCachedFramebuffer) {
        fCachedFramebuffer->unref();
        fCachedFramebuffer = nullptr;
    }
    if (fCachedSimpleRenderPass) {
        fCachedSimpleRenderPass->unref();
        fCachedSimpleRenderPass = nullptr;
    }
    for (int i = 0; i < fGrSecondaryCommandBuffers.count(); ++i) {
        SkASSERT(fGrSecondaryCommandBuffers[i]);
        fGrSecondaryCommandBuffers[i]->releaseResources();
    }
    fGrSecondaryCommandBuffers.reset();
}

void GrVkRenderTarget::onRelease() {
    this->releaseInternalObjects();
    this->releaseImage(this->getVkGpu());
    GrRenderTarget::onRelease();
}

void GrVkRenderTarget::onAbandon() {
    this->releaseInternalObjects();
    this->releaseImage(this->getVkGpu());
    GrRenderTarget::onAbandon();
}

GrBackendRenderTarget GrVkRenderTarget::getBackendRenderTarget() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    return GrBackendRenderTarget(this->width(), this->height(), this->numSamples(), fInfo,
                                 this->grVkImageLayout());
}

const GrManagedResource* GrVkRenderTarget::stencilImageResource() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    const GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment();
    if (stencil) {
        const GrVkStencilAttachment* vkStencil = static_cast<const GrVkStencilAttachment*>(stencil);
        return vkStencil->imageResource();
    }

    return nullptr;
}

const GrVkImageView* GrVkRenderTarget::stencilAttachmentView() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    const GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment();
    if (stencil) {
        const GrVkStencilAttachment* vkStencil = static_cast<const GrVkStencilAttachment*>(stencil);
        return vkStencil->stencilView();
    }

    return nullptr;
}

GrVkGpu* GrVkRenderTarget::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
