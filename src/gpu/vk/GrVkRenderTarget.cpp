/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrBackendSurfaceMutableStateImpl.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/vk/GrVkAttachment.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkFramebuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkResourceProvider.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

static int renderpass_features_to_index(bool hasResolve, bool hasStencil,
                                        GrVkRenderPass::SelfDependencyFlags selfDepFlags,
                                        GrVkRenderPass::LoadFromResolve loadFromReslove) {
    int index = 0;
    if (hasResolve) {
        index += 1;
    }
    if (hasStencil) {
        index += 2;
    }
    if (selfDepFlags & GrVkRenderPass::SelfDependencyFlags::kForInputAttachment) {
        index += 4;
    }
    if (selfDepFlags & GrVkRenderPass::SelfDependencyFlags::kForNonCoherentAdvBlend) {
        index += 8;
    }
    if (loadFromReslove == GrVkRenderPass::LoadFromResolve::kLoad) {
        index += 16;
    }
    return index;
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   sk_sp<GrVkAttachment> colorAttachment,
                                   sk_sp<GrVkAttachment> resolveAttachment,
                                   CreateType createType)
        : GrSurface(gpu, dimensions,
                    colorAttachment->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, colorAttachment->numSamples(),
                         colorAttachment->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment))
        , fCachedFramebuffers() {
    SkASSERT(fColorAttachment);
    SkASSERT(!resolveAttachment ||
             (fResolveAttachment->isProtected() == fColorAttachment->isProtected()));
    SkASSERT(SkToBool(fColorAttachment->vkUsageFlags() & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    this->setFlags();
    if (createType == CreateType::kDirectlyWrapped) {
        this->registerWithCacheWrapped(GrWrapCacheable::kNo);
    }
}

GrVkRenderTarget::GrVkRenderTarget(GrVkGpu* gpu,
                                   SkISize dimensions,
                                   sk_sp<GrVkFramebuffer> externalFramebuffer)
        : GrSurface(gpu, dimensions,
                    externalFramebuffer->colorAttachment()->isProtected() ? GrProtected::kYes
                                                                          : GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1,
                         externalFramebuffer->colorAttachment()->isProtected() ? GrProtected::kYes
                                                                               : GrProtected::kNo)
        , fCachedFramebuffers()
        , fExternalFramebuffer(externalFramebuffer) {
    SkASSERT(fExternalFramebuffer);
    SkASSERT(!fColorAttachment);
    SkDEBUGCODE(auto colorAttachment = fExternalFramebuffer->colorAttachment());
    SkASSERT(colorAttachment);
    SkASSERT(colorAttachment->numSamples() == 1);
    SkASSERT(SkToBool(colorAttachment->vkUsageFlags() & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    SkASSERT(!SkToBool(colorAttachment->vkUsageFlags() & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));
    this->setFlags();
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

void GrVkRenderTarget::setFlags() {
    if (this->wrapsSecondaryCommandBuffer()) {
        return;
    }
    GrVkAttachment* nonMSAAAttachment = this->nonMSAAAttachment();
    if (nonMSAAAttachment && nonMSAAAttachment->supportsInputAttachmentUsage()) {
        this->setVkRTSupportsInputAttachment();
    }
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeWrappedRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState) {
    SkASSERT(VK_NULL_HANDLE != info.fImage);
    SkASSERT(1 == info.fLevelCount);
    SkASSERT(sampleCnt >= 1 && info.fSampleCount >= 1);

    int wrappedImageSampleCnt = static_cast<int>(info.fSampleCount);
    if (sampleCnt != wrappedImageSampleCnt && wrappedImageSampleCnt != 1) {
        return nullptr;
    }

    sk_sp<GrVkAttachment> wrappedAttachment =
            GrVkAttachment::MakeWrapped(gpu, dimensions, info, std::move(mutableState),
                                        GrAttachment::UsageFlags::kColorAttachment,
                                        kBorrow_GrWrapOwnership, GrWrapCacheable::kNo);
    if (!wrappedAttachment) {
        return nullptr;
    }

    sk_sp<GrVkAttachment> colorAttachment;
    colorAttachment = std::move(wrappedAttachment);

     if (!colorAttachment) {
        return nullptr;
    }

    GrVkRenderTarget* vkRT = new GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                                                  nullptr, CreateType::kDirectlyWrapped);
    return sk_sp<GrVkRenderTarget>(vkRT);
}

sk_sp<GrVkRenderTarget> GrVkRenderTarget::MakeSecondaryCBRenderTarget(
        GrVkGpu* gpu, SkISize dimensions, const GrVkDrawableInfo& vkInfo) {
    const GrVkRenderPass* rp = gpu->resourceProvider().findCompatibleExternalRenderPass(
            vkInfo.fCompatibleRenderPass, vkInfo.fColorAttachmentIndex);
    if (!rp) {
        return nullptr;
    }

    if (vkInfo.fSecondaryCommandBuffer == VK_NULL_HANDLE) {
        return nullptr;
    }

    // We only set the few properties of the GrVkImageInfo that we know like layout and format. The
    // others we keep at the default "null" values.
    GrVkImageInfo info;
    info.fImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    info.fFormat = vkInfo.fFormat;
    info.fImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(new GrBackendSurfaceMutableStateImpl(
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_QUEUE_FAMILY_IGNORED));

    sk_sp<GrVkAttachment> colorAttachment =
        GrVkAttachment::MakeWrapped(gpu, dimensions, info, std::move(mutableState),
                                    GrAttachment::UsageFlags::kColorAttachment,
                                    kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, true);

    std::unique_ptr<GrVkSecondaryCommandBuffer> scb(
            GrVkSecondaryCommandBuffer::Create(vkInfo.fSecondaryCommandBuffer, rp));
    if (!scb) {
        return nullptr;
    }

    sk_sp<GrVkFramebuffer> framebuffer(new GrVkFramebuffer(
            gpu, std::move(colorAttachment), sk_sp<const GrVkRenderPass>(rp),
            std::move(scb)));

    GrVkRenderTarget* vkRT = new GrVkRenderTarget(gpu, dimensions, std::move(framebuffer));

    return sk_sp<GrVkRenderTarget>(vkRT);
}

GrBackendFormat GrVkRenderTarget::backendFormat() const {
    if (this->wrapsSecondaryCommandBuffer()) {
        return fExternalFramebuffer->colorAttachment()->getBackendFormat();
    }
    return fColorAttachment->getBackendFormat();
}

GrVkAttachment* GrVkRenderTarget::nonMSAAAttachment() const {
    if (fColorAttachment->numSamples() == 1) {
        return fColorAttachment.get();
    } else {
        return fResolveAttachment.get();
    }
}

GrVkAttachment* GrVkRenderTarget::dynamicMSAAAttachment() {
    if (fDynamicMSAAAttachment) {
        return fDynamicMSAAAttachment.get();
    }
    const GrVkAttachment* nonMSAAColorAttachment = this->colorAttachment();
    SkASSERT(nonMSAAColorAttachment->numSamples() == 1);

    GrVkGpu* gpu = this->getVkGpu();
    auto rp = gpu->getContext()->priv().resourceProvider();

    const GrBackendFormat& format = nonMSAAColorAttachment->backendFormat();

    sk_sp<GrAttachment> msaaAttachment =
            rp->getDiscardableMSAAAttachment(nonMSAAColorAttachment->dimensions(),
                                             format,
                                             gpu->caps()->internalMultisampleCount(format),
                                             GrProtected(nonMSAAColorAttachment->isProtected()));
    if (!msaaAttachment) {
        return nullptr;
    }
    fDynamicMSAAAttachment =
            sk_sp<GrVkAttachment>(static_cast<GrVkAttachment*>(msaaAttachment.release()));
    return fDynamicMSAAAttachment.get();
}

GrVkAttachment* GrVkRenderTarget::msaaAttachment() {
    return this->colorAttachment()->numSamples() == 1 ? this->dynamicMSAAAttachment()
                                                      : this->colorAttachment();
}

bool GrVkRenderTarget::completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    SkASSERT(useMSAASurface == (this->numSamples() > 1));
    return true;
}

sk_sp<GrVkFramebuffer> GrVkRenderTarget::externalFramebuffer() const {
    return fExternalFramebuffer;
}

GrVkResourceProvider::CompatibleRPHandle GrVkRenderTarget::compatibleRenderPassHandle(
        bool withResolve,
        bool withStencil,
        SelfDependencyFlags selfDepFlags,
        LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());

    const GrVkFramebuffer* fb =
            this->getFramebuffer(withResolve, withStencil, selfDepFlags, loadFromResolve);
    if (!fb) {
        return {};
    }

    return fb->compatibleRenderPassHandle();
}

const GrVkRenderPass* GrVkRenderTarget::getSimpleRenderPass(bool withResolve,
                                                            bool withStencil,
                                                            SelfDependencyFlags selfDepFlags,
                                                            LoadFromResolve loadFromResolve) {
    if (this->wrapsSecondaryCommandBuffer()) {
         return fExternalFramebuffer->externalRenderPass();
    }

    const GrVkFramebuffer* fb =
            this->getFramebuffer(withResolve, withStencil, selfDepFlags, loadFromResolve);
    if (!fb) {
        return nullptr;
    }

    return fb->compatibleRenderPass();
}

std::pair<const GrVkRenderPass*, GrVkResourceProvider::CompatibleRPHandle>
GrVkRenderTarget::createSimpleRenderPass(bool withResolve,
                                         bool withStencil,
                                         SelfDependencyFlags selfDepFlags,
                                         LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());

    GrVkResourceProvider& rp = this->getVkGpu()->resourceProvider();

    GrVkResourceProvider::CompatibleRPHandle handle;
    const GrVkRenderPass* renderPass = rp.findCompatibleRenderPass(
            this, &handle, withResolve, withStencil, selfDepFlags,
            loadFromResolve);
    SkASSERT(!renderPass || handle.isValid());
    return {renderPass, handle};
}

const GrVkFramebuffer* GrVkRenderTarget::getFramebuffer(bool withResolve,
                                                        bool withStencil,
                                                        SelfDependencyFlags selfDepFlags,
                                                        LoadFromResolve loadFromResolve) {
    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil, selfDepFlags, loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedFramebuffers);
    if (auto fb = fCachedFramebuffers[cacheIndex]) {
        return fb.get();
    }

    this->createFramebuffer(withResolve, withStencil, selfDepFlags, loadFromResolve);
    return fCachedFramebuffers[cacheIndex].get();
}

void GrVkRenderTarget::createFramebuffer(bool withResolve,
                                         bool withStencil,
                                         SelfDependencyFlags selfDepFlags,
                                         LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    GrVkGpu* gpu = this->getVkGpu();

    auto[renderPass, compatibleHandle] =
            this->createSimpleRenderPass(withResolve, withStencil, selfDepFlags, loadFromResolve);
    if (!renderPass) {
        return;
    }
    SkASSERT(compatibleHandle.isValid());

    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil, selfDepFlags, loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedFramebuffers);

    GrVkAttachment* resolve = withResolve ? this->resolveAttachment() : nullptr;
    GrVkAttachment* colorAttachment =
            withResolve ? this->msaaAttachment() : this->colorAttachment();

    // Stencil attachment view is stored in the base RT stencil attachment
    GrVkAttachment* stencil =
            withStencil ? static_cast<GrVkAttachment*>(this->getStencilAttachment())
                        : nullptr;
    fCachedFramebuffers[cacheIndex] =
            GrVkFramebuffer::Make(gpu, this->dimensions(),
                                  sk_sp<const GrVkRenderPass>(renderPass),
                                  colorAttachment, resolve, stencil, compatibleHandle);
}

void GrVkRenderTarget::getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                                GrVkRenderPass::AttachmentFlags* attachmentFlags,
                                                bool withResolve,
                                                bool withStencil) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    const GrVkAttachment* colorAttachment =
            withResolve ? this->msaaAttachment() : this->colorAttachment();

    desc->fColor.fFormat = colorAttachment->imageFormat();
    desc->fColor.fSamples = colorAttachment->numSamples();
    *attachmentFlags = GrVkRenderPass::kColor_AttachmentFlag;
    uint32_t attachmentCount = 1;

    if (withResolve) {
        desc->fResolve.fFormat = desc->fColor.fFormat;
        desc->fResolve.fSamples = 1;
        *attachmentFlags |= GrVkRenderPass::kResolve_AttachmentFlag;
        ++attachmentCount;
    }

    if (withStencil) {
        const GrAttachment* stencil = this->getStencilAttachment();
        SkASSERT(stencil);
        const GrVkAttachment* vkStencil = static_cast<const GrVkAttachment*>(stencil);
        desc->fStencil.fFormat = vkStencil->imageFormat();
        desc->fStencil.fSamples = vkStencil->numSamples();
        SkASSERT(desc->fStencil.fSamples == desc->fColor.fSamples);
        *attachmentFlags |= GrVkRenderPass::kStencil_AttachmentFlag;
        ++attachmentCount;
    }
    desc->fAttachmentCount = attachmentCount;
}

void GrVkRenderTarget::ReconstructAttachmentsDescriptor(const GrVkCaps& vkCaps,
                                                        const GrProgramInfo& programInfo,
                                                        GrVkRenderPass::AttachmentsDescriptor* desc,
                                                        GrVkRenderPass::AttachmentFlags* flags) {
    VkFormat format;
    SkAssertResult(programInfo.backendFormat().asVkFormat(&format));

    desc->fColor.fFormat = format;
    desc->fColor.fSamples = programInfo.numSamples();
    *flags = GrVkRenderPass::kColor_AttachmentFlag;
    uint32_t attachmentCount = 1;

    if (programInfo.targetSupportsVkResolveLoad() && vkCaps.preferDiscardableMSAAAttachment()) {
        desc->fResolve.fFormat = desc->fColor.fFormat;
        desc->fResolve.fSamples = 1;
        *flags |= GrVkRenderPass::kResolve_AttachmentFlag;
        ++attachmentCount;
    }

    SkASSERT(!programInfo.isStencilEnabled() || programInfo.needsStencil());
    if (programInfo.needsStencil()) {
        VkFormat stencilFormat = vkCaps.preferredStencilFormat();
        desc->fStencil.fFormat = stencilFormat;
        desc->fStencil.fSamples = programInfo.numSamples();
        SkASSERT(desc->fStencil.fSamples == desc->fColor.fSamples);
        *flags |= GrVkRenderPass::kStencil_AttachmentFlag;
        ++attachmentCount;
    }
    desc->fAttachmentCount = attachmentCount;
}

GrVkRenderTarget::~GrVkRenderTarget() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fColorAttachment);
    SkASSERT(!fResolveAttachment);
    SkASSERT(!fDynamicMSAAAttachment);

    for (int i = 0; i < kNumCachedFramebuffers; ++i) {
        SkASSERT(!fCachedFramebuffers[i]);
    }

    SkASSERT(!fCachedInputDescriptorSet);
}

void GrVkRenderTarget::releaseInternalObjects() {
    fColorAttachment.reset();
    fResolveAttachment.reset();
    fDynamicMSAAAttachment.reset();

    for (int i = 0; i < kNumCachedFramebuffers; ++i) {
        if (fCachedFramebuffers[i]) {
            fCachedFramebuffers[i].reset();
        }
    }

    if (fCachedInputDescriptorSet) {
        fCachedInputDescriptorSet->recycle();
        fCachedInputDescriptorSet = nullptr;
    }

    fExternalFramebuffer.reset();
}

void GrVkRenderTarget::onRelease() {
    this->releaseInternalObjects();
    GrRenderTarget::onRelease();
}

void GrVkRenderTarget::onAbandon() {
    this->releaseInternalObjects();
    GrRenderTarget::onAbandon();
}

GrBackendRenderTarget GrVkRenderTarget::getBackendRenderTarget() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    // This should only get called with a non-released GrVkRenderTargets.
    SkASSERT(!this->wasDestroyed());
    // If we have a resolve attachment that is what we return for the backend render target
    const GrVkAttachment* beAttachment = this->externalAttachment();
    return GrBackendRenderTarget(beAttachment->width(), beAttachment->height(),
                                 beAttachment->vkImageInfo(), beAttachment->getMutableState());
}

const GrManagedResource* GrVkRenderTarget::stencilImageResource() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    const GrAttachment* stencil = this->getStencilAttachment();
    if (stencil) {
        const GrVkAttachment* vkStencil = static_cast<const GrVkAttachment*>(stencil);
        return vkStencil->imageResource();
    }

    return nullptr;
}

const GrVkImageView* GrVkRenderTarget::stencilAttachmentView() const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    const GrAttachment* stencil = this->getStencilAttachment();
    if (stencil) {
        const GrVkAttachment* vkStencil = static_cast<const GrVkAttachment*>(stencil);
        return vkStencil->framebufferView();
    }

    return nullptr;
}

GrVkGpu* GrVkRenderTarget::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
