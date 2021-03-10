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
        , fCachedFramebuffers()
        , fCachedRenderPasses() {
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
                                   sk_sp<GrVkAttachment> colorAttachment,
                                   const GrVkRenderPass* renderPass,
                                   VkCommandBuffer secondaryCommandBuffer)
        : GrSurface(gpu, dimensions,
                    colorAttachment->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1,
                         colorAttachment->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , fColorAttachment(std::move(colorAttachment))
        , fCachedFramebuffers()
        , fCachedRenderPasses()
        , fSecondaryCommandBuffer(secondaryCommandBuffer) {
    SkASSERT(fColorAttachment->numSamples() == 1);
    SkASSERT(fSecondaryCommandBuffer != VK_NULL_HANDLE);
    SkASSERT(SkToBool(fColorAttachment->vkUsageFlags() & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    SkASSERT(!SkToBool(fColorAttachment->vkUsageFlags() & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));
    this->setFlags();
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
    // We use the cached renderpass with no stencil and no extra dependencies to hold the external
    // render pass.
    int exteralRPIndex = renderpass_features_to_index(false, false, SelfDependencyFlags::kNone,
                                                      LoadFromResolve::kNo);
    fCachedRenderPasses[exteralRPIndex] = renderPass;
}

void GrVkRenderTarget::setFlags() {
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

    sk_sp<GrVkAttachment> scbAttachment =
        GrVkAttachment::MakeWrapped(gpu, dimensions, info, std::move(mutableState),
                                    GrAttachment::UsageFlags::kColorAttachment,
                                    kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, true);

    GrVkRenderTarget* vkRT = new GrVkRenderTarget(gpu, dimensions, std::move(scbAttachment),
                                                  rp, vkInfo.fSecondaryCommandBuffer);

    return sk_sp<GrVkRenderTarget>(vkRT);
}

GrVkAttachment* GrVkRenderTarget::nonMSAAAttachment() const {
    if (fColorAttachment->numSamples() == 1) {
        return fColorAttachment.get();
    } else {
        return fResolveAttachment.get();
    }
}

bool GrVkRenderTarget::completeStencilAttachment() {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    return true;
}

const GrVkRenderPass* GrVkRenderTarget::externalRenderPass() const {
    SkASSERT(this->wrapsSecondaryCommandBuffer());
    // We use the cached render pass with no attachments or self dependencies to hold the
    // external render pass.
    int exteralRPIndex = renderpass_features_to_index(false, false, SelfDependencyFlags::kNone,
                                                      LoadFromResolve::kNo);
    return fCachedRenderPasses[exteralRPIndex];
}

GrVkResourceProvider::CompatibleRPHandle GrVkRenderTarget::compatibleRenderPassHandle(
        bool withResolve,
        bool withStencil,
        SelfDependencyFlags selfDepFlags,
        LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());

    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil, selfDepFlags, loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedRenderPasses);

    GrVkResourceProvider::CompatibleRPHandle* pRPHandle;
    pRPHandle = &fCompatibleRPHandles[cacheIndex];

    if (!pRPHandle->isValid()) {
        this->createSimpleRenderPass(withResolve, withStencil, selfDepFlags, loadFromResolve);
    }

#ifdef SK_DEBUG
    const GrVkRenderPass* rp = fCachedRenderPasses[cacheIndex];
    SkASSERT(pRPHandle->isValid() == SkToBool(rp));
    if (rp) {
        SkASSERT(selfDepFlags == rp->selfDependencyFlags());
    }
#endif

    return *pRPHandle;
}

const GrVkRenderPass* GrVkRenderTarget::getSimpleRenderPass(bool withResolve,
                                                            bool withStencil,
                                                            SelfDependencyFlags selfDepFlags,
                                                            LoadFromResolve loadFromResolve) {
    int cacheIndex = renderpass_features_to_index(withResolve, withStencil, selfDepFlags,
                                                  loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedRenderPasses);
    if (const GrVkRenderPass* rp = fCachedRenderPasses[cacheIndex]) {
        return rp;
    }

    return this->createSimpleRenderPass(withResolve, withStencil, selfDepFlags, loadFromResolve);
}

const GrVkRenderPass* GrVkRenderTarget::createSimpleRenderPass(bool withResolve,
                                                               bool withStencil,
                                                               SelfDependencyFlags selfDepFlags,
                                                               LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());

    GrVkResourceProvider& rp = this->getVkGpu()->resourceProvider();
    int cacheIndex = renderpass_features_to_index(withResolve, withStencil, selfDepFlags,
                                                  loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedRenderPasses);
    SkASSERT(!fCachedRenderPasses[cacheIndex]);
    fCachedRenderPasses[cacheIndex] = rp.findCompatibleRenderPass(
            *this, &fCompatibleRPHandles[cacheIndex], withResolve, withStencil, selfDepFlags,
            loadFromResolve);
    return fCachedRenderPasses[cacheIndex];
}

const GrVkFramebuffer* GrVkRenderTarget::getFramebuffer(bool withResolve,
                                                        bool withStencil,
                                                        SelfDependencyFlags selfDepFlags,
                                                        LoadFromResolve loadFromResolve) {
    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil, selfDepFlags, loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedRenderPasses);
    if (auto fb = fCachedFramebuffers[cacheIndex]) {
        return fb;
    }

    return this->createFramebuffer(withResolve, withStencil, selfDepFlags, loadFromResolve);
}

const GrVkFramebuffer* GrVkRenderTarget::createFramebuffer(bool withResolve,
                                                           bool withStencil,
                                                           SelfDependencyFlags selfDepFlags,
                                                           LoadFromResolve loadFromResolve) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    GrVkGpu* gpu = this->getVkGpu();

    const GrVkRenderPass* renderPass =
            this->getSimpleRenderPass(withResolve, withStencil, selfDepFlags, loadFromResolve);
    if (!renderPass) {
        return nullptr;
    }

    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil, selfDepFlags, loadFromResolve);
    SkASSERT(cacheIndex < GrVkRenderTarget::kNumCachedRenderPasses);

    const GrVkImageView* resolveView = withResolve ? this->resolveAttachmentView() : nullptr;

    // Stencil attachment view is stored in the base RT stencil attachment
    const GrVkImageView* stencilView = withStencil ? this->stencilAttachmentView() : nullptr;
    fCachedFramebuffers[cacheIndex] =
            GrVkFramebuffer::Create(gpu, this->width(), this->height(), renderPass,
                                    this->colorAttachmentView(), resolveView, stencilView);

    return fCachedFramebuffers[cacheIndex];
}

void GrVkRenderTarget::getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                                GrVkRenderPass::AttachmentFlags* attachmentFlags,
                                                bool withResolve,
                                                bool withStencil) const {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());
    desc->fColor.fFormat = fColorAttachment->imageFormat();
    desc->fColor.fSamples = fColorAttachment->numSamples();
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

    SkASSERT(!programInfo.isStencilEnabled() || programInfo.numStencilSamples());
    if (programInfo.numStencilSamples()) {
        VkFormat stencilFormat = vkCaps.preferredStencilFormat();
        desc->fStencil.fFormat = stencilFormat;
        desc->fStencil.fSamples = programInfo.numStencilSamples();
#ifdef SK_DEBUG
        if (vkCaps.mixedSamplesSupport()) {
            SkASSERT(desc->fStencil.fSamples >= desc->fColor.fSamples);
        } else {
            SkASSERT(desc->fStencil.fSamples == desc->fColor.fSamples);
        }
#endif
        *flags |= GrVkRenderPass::kStencil_AttachmentFlag;
        ++attachmentCount;
    }
    desc->fAttachmentCount = attachmentCount;
}

const GrVkDescriptorSet* GrVkRenderTarget::inputDescSet(GrVkGpu* gpu, bool forResolve) {
    SkASSERT((forResolve && fResolveAttachment->supportsInputAttachmentUsage()) ||
             (!forResolve && fColorAttachment->supportsInputAttachmentUsage()));
    SkASSERT(this->numSamples() <= 1 || forResolve);

    if (fCachedInputDescriptorSet) {
        return fCachedInputDescriptorSet;
    }
    fCachedInputDescriptorSet = gpu->resourceProvider().getInputDescriptorSet();

    if (!fCachedInputDescriptorSet) {
        return nullptr;
    }

    VkDescriptorImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = forResolve ? this->resolveAttachmentView()->imageView()
                                     : this->colorAttachmentView()->imageView();
    imageInfo.imageLayout = forResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                       : VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet writeInfo;
    memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstSet = *fCachedInputDescriptorSet->descriptorSet();
    writeInfo.dstBinding = GrVkUniformHandler::kInputBinding;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    writeInfo.pImageInfo = &imageInfo;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(), 1, &writeInfo, 0, nullptr));

    return fCachedInputDescriptorSet;
}

GrVkRenderTarget::~GrVkRenderTarget() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fColorAttachment);
    SkASSERT(!fResolveAttachment);

    for (int i = 0; i < kNumCachedRenderPasses; ++i) {
        SkASSERT(!fCachedFramebuffers[i]);
        SkASSERT(!fCachedRenderPasses[i]);
    }

    SkASSERT(!fCachedInputDescriptorSet);
}

void GrVkRenderTarget::addResources(GrVkCommandBuffer& commandBuffer,
                                    const GrVkRenderPass& renderPass) {
    commandBuffer.addGrSurface(sk_ref_sp<const GrSurface>(this));
    commandBuffer.addResource(this->getFramebuffer(renderPass));
    commandBuffer.addResource(this->colorAttachmentView());
    commandBuffer.addResource(fColorAttachment->resource());

    if (this->stencilImageResource()) {
        commandBuffer.addResource(this->stencilImageResource());
        commandBuffer.addResource(this->stencilAttachmentView());
    }
    if (renderPass.hasResolveAttachment()) {
        SkASSERT(fResolveAttachment);
        commandBuffer.addResource(fResolveAttachment->resource());
        commandBuffer.addResource(this->resolveAttachmentView());
    }
}

void GrVkRenderTarget::releaseInternalObjects() {
    fColorAttachment.reset();
    fResolveAttachment.reset();

    for (int i = 0; i < kNumCachedRenderPasses; ++i) {
        if (fCachedFramebuffers[i]) {
            fCachedFramebuffers[i]->unref();
            fCachedFramebuffers[i] = nullptr;
        }
        if (fCachedRenderPasses[i]) {
            fCachedRenderPasses[i]->unref();
            fCachedRenderPasses[i] = nullptr;
        }
    }

    if (fCachedInputDescriptorSet) {
        fCachedInputDescriptorSet->recycle();
        fCachedInputDescriptorSet = nullptr;
    }

    for (int i = 0; i < fGrSecondaryCommandBuffers.count(); ++i) {
        SkASSERT(fGrSecondaryCommandBuffers[i]);
        fGrSecondaryCommandBuffers[i]->releaseResources();
    }
    fGrSecondaryCommandBuffers.reset();
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
