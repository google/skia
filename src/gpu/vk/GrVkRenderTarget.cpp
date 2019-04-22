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
                                   const GrSurfaceDesc& desc,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageInfo& msaaInfo,
                                   sk_sp<GrVkImageLayout> msaaLayout,
                                   const GrVkImageView* colorAttachmentView,
                                   const GrVkImageView* resolveAttachmentView)
        : GrSurface(gpu, desc)
        , GrVkImage(info, std::move(layout), GrBackendObjectOwnership::kBorrowed)
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, desc)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(new GrVkImage(msaaInfo, std::move(msaaLayout),
                                   GrBackendObjectOwnership::kOwned))
        , fResolveAttachmentView(resolveAttachmentView)
        , fFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(desc.fSampleCnt > 1);
    this->createFramebuffer(gpu);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageInfo& msaaInfo,
                                   sk_sp<GrVkImageLayout> msaaLayout,
                                   const GrVkImageView* colorAttachmentView,
                                   const GrVkImageView* resolveAttachmentView,
                                   GrBackendObjectOwnership ownership)
        : GrSurface(gpu, desc)
        , GrVkImage(info, std::move(layout), ownership)
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, desc)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(new GrVkImage(msaaInfo, std::move(msaaLayout),
                                   GrBackendObjectOwnership::kOwned))
        , fResolveAttachmentView(resolveAttachmentView)
        , fFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(desc.fSampleCnt > 1);
    this->createFramebuffer(gpu);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageView* colorAttachmentView)
        : GrSurface(gpu, desc)
        , GrVkImage(info, std::move(layout), GrBackendObjectOwnership::kBorrowed)
        , GrRenderTarget(gpu, desc)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(1 == desc.fSampleCnt);
    this->createFramebuffer(gpu);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkImageView* colorAttachmentView,
                                   GrBackendObjectOwnership ownership)
        : GrSurface(gpu, desc)
        , GrVkImage(info, std::move(layout), ownership)
        , GrRenderTarget(gpu, desc)
        , fColorAttachmentView(colorAttachmentView)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fFramebuffer(nullptr)
        , fCachedSimpleRenderPass(nullptr) {
    SkASSERT(1 == desc.fSampleCnt);
    this->createFramebuffer(gpu);
}

GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   const GrVkImageInfo& info,
                                   sk_sp<GrVkImageLayout> layout,
                                   const GrVkRenderPass* renderPass,
                                   GrVkSecondaryCommandBuffer* secondaryCommandBuffer)
        : GrSurface(gpu, desc)
        , GrVkImage(info, std::move(layout), GrBackendObjectOwnership::kBorrowed, true)
        , GrRenderTarget(gpu, desc)
        , fColorAttachmentView(nullptr)
        , fMSAAImage(nullptr)
        , fResolveAttachmentView(nullptr)
        , fFramebuffer(nullptr)
        , fCachedSimpleRenderPass(renderPass)
        , fSecondaryCommandBuffer(secondaryCommandBuffer) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeWrappedRenderTarget(
        GrVkGpu* gpu, const GrSurfaceDesc& desc, const GrVkImageInfo& info,
        sk_sp<GrVkImageLayout> layout) {
    SkASSERT(VK_NULL_HANDLE != info.fImage);

    SkASSERT(1 == info.fLevelCount);
    VkFormat pixelFormat;
    GrPixelConfigToVkFormat(desc.fConfig, &pixelFormat);

    VkImage colorImage;

    // create msaa surface if necessary
    GrVkImageInfo msInfo;
    sk_sp<GrVkImageLayout> msLayout;
    const GrVkImageView* resolveAttachmentView = nullptr;
    if (desc.fSampleCnt > 1) {
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
        if (desc.fSampleCnt > 1) {
            resolveAttachmentView->unref(gpu);
            GrVkImage::DestroyImageInfo(gpu, &msInfo);
        }
        return nullptr;
    }

    GrVkRenderTarget* vkRT;
    if (desc.fSampleCnt > 1) {
        vkRT = new GrVkRenderTarget(gpu, desc, info, std::move(layout), msInfo,
                                    std::move(msLayout), colorAttachmentView,
                                    resolveAttachmentView);
    } else {
        vkRT = new GrVkRenderTarget(gpu, desc, info, std::move(layout), colorAttachmentView);
    }

    return sk_sp<GrVkRenderTarget>(vkRT);
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeSecondaryCBRenderTarget(
        GrVkGpu* gpu, const GrSurfaceDesc& desc, const GrVkDrawableInfo& vkInfo) {
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

    GrVkSecondaryCommandBuffer* scb =
            GrVkSecondaryCommandBuffer::Create(vkInfo.fSecondaryCommandBuffer);
    if (!scb) {
        return nullptr;
    }

    GrVkRenderTarget* vkRT = new GrVkRenderTarget(gpu, desc, info, std::move(layout), rp, scb);

    return sk_sp<GrVkRenderTarget>(vkRT);
}

bool GrVkRenderTarget::completeStencilAttachment() {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    this->createFramebuffer(this->getVkGpu());
    return true;
}

void GrVkRenderTarget::createFramebuffer(GrVkGpu* gpu) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    if (fFramebuffer) {
        fFramebuffer->unref(gpu);
    }
    if (fCachedSimpleRenderPass) {
        fCachedSimpleRenderPass->unref(gpu);
    }

    // Vulkan requires us to create a compatible renderpass before we can create our framebuffer,
    // so we use this to get a (cached) basic renderpass, only for creation.
    fCachedSimpleRenderPass =
        gpu->resourceProvider().findCompatibleRenderPass(*this, &fCompatibleRPHandle);

    // Stencil attachment view is stored in the base RT stencil attachment
    const GrVkImageView* stencilView = this->stencilAttachmentView();
    fFramebuffer = GrVkFramebuffer::Create(gpu, this->width(), this->height(),
                                           fCachedSimpleRenderPass, fColorAttachmentView,
                                           stencilView);
    SkASSERT(fFramebuffer);
}

void GrVkRenderTarget::getAttachmentsDescriptor(
                                           GrVkRenderPass::AttachmentsDescriptor* desc,
                                           GrVkRenderPass::AttachmentFlags* attachmentFlags) const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    VkFormat colorFormat;
    GrPixelConfigToVkFormat(this->config(), &colorFormat);
    desc->fColor.fFormat = colorFormat;
    desc->fColor.fSamples = this->numColorSamples();
    *attachmentFlags = GrVkRenderPass::kColor_AttachmentFlag;
    uint32_t attachmentCount = 1;

    const GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment();
    if (stencil) {
        const GrVkStencilAttachment* vkStencil = static_cast<const GrVkStencilAttachment*>(stencil);
        desc->fStencil.fFormat = vkStencil->vkFormat();
        desc->fStencil.fSamples = vkStencil->numSamples();
        // Currently in vulkan stencil and color attachments must all have same number of samples
        SkASSERT(desc->fColor.fSamples == desc->fStencil.fSamples);
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
    SkASSERT(!fFramebuffer);
    SkASSERT(!fCachedSimpleRenderPass);
    SkASSERT(!fSecondaryCommandBuffer);
}

void GrVkRenderTarget::addResources(GrVkCommandBuffer& commandBuffer) const {
    commandBuffer.addResource(this->framebuffer());
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
        fResolveAttachmentView->unref(gpu);
        fResolveAttachmentView = nullptr;
    }
    if (fColorAttachmentView) {
        fColorAttachmentView->unref(gpu);
        fColorAttachmentView = nullptr;
    }
    if (fFramebuffer) {
        fFramebuffer->unref(gpu);
        fFramebuffer = nullptr;
    }
    if (fCachedSimpleRenderPass) {
        fCachedSimpleRenderPass->unref(gpu);
        fCachedSimpleRenderPass = nullptr;
    }
    if (fSecondaryCommandBuffer) {
        fSecondaryCommandBuffer->unref(gpu);
        fSecondaryCommandBuffer = nullptr;
    }
}

void GrVkRenderTarget::abandonInternalObjects() {
    if (fMSAAImage) {
        fMSAAImage->abandonImage();
        fMSAAImage.reset();
    }

    if (fResolveAttachmentView) {
        fResolveAttachmentView->unrefAndAbandon();
        fResolveAttachmentView = nullptr;
    }
    if (fColorAttachmentView) {
        fColorAttachmentView->unrefAndAbandon();
        fColorAttachmentView = nullptr;
    }
    if (fFramebuffer) {
        fFramebuffer->unrefAndAbandon();
        fFramebuffer = nullptr;
    }
    if (fCachedSimpleRenderPass) {
        fCachedSimpleRenderPass->unrefAndAbandon();
        fCachedSimpleRenderPass = nullptr;
    }
    if (fSecondaryCommandBuffer) {
        fSecondaryCommandBuffer->unrefAndAbandon();
        fSecondaryCommandBuffer = nullptr;
    }
}

void GrVkRenderTarget::onRelease() {
    this->releaseInternalObjects();
    this->releaseImage(this->getVkGpu());
    GrRenderTarget::onRelease();
}

void GrVkRenderTarget::onAbandon() {
    this->abandonInternalObjects();
    this->abandonImage();
    GrRenderTarget::onAbandon();
}


GrBackendRenderTarget GrVkRenderTarget::getBackendRenderTarget() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    return GrBackendRenderTarget(this->width(), this->height(), this->numColorSamples(),
                                 fInfo, this->grVkImageLayout());
}

const GrVkResource* GrVkRenderTarget::stencilImageResource() const {
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
