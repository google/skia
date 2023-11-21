/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkRenderTarget_DEFINED
#define GrVkRenderTarget_DEFINED

#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/ganesh/vk/GrVkRenderPass.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"

class GrVkCaps;
class GrVkFramebuffer;
class GrVkGpu;
class GrVkImageView;

struct GrVkImageInfo;

class GrVkRenderTarget : public GrRenderTarget {
public:
    static sk_sp<GrVkRenderTarget> MakeWrappedRenderTarget(GrVkGpu*,
                                                           SkISize,
                                                           int sampleCnt,
                                                           const GrVkImageInfo&,
                                                           sk_sp<skgpu::MutableTextureState>);

    static sk_sp<GrVkRenderTarget> MakeSecondaryCBRenderTarget(GrVkGpu*,
                                                               SkISize,
                                                               const GrVkDrawableInfo& vkInfo);

    ~GrVkRenderTarget() override;

    GrBackendFormat backendFormat() const override;

    using SelfDependencyFlags = GrVkRenderPass::SelfDependencyFlags;
    using LoadFromResolve = GrVkRenderPass::LoadFromResolve;

    const GrVkFramebuffer* getFramebuffer(bool withResolve,
                                          bool withStencil,
                                          SelfDependencyFlags selfDepFlags,
                                          LoadFromResolve);
    const GrVkFramebuffer* getFramebuffer(const GrVkRenderPass& renderPass) {
        return this->getFramebuffer(renderPass.hasResolveAttachment(),
                                    renderPass.hasStencilAttachment(),
                                    renderPass.selfDependencyFlags(),
                                    renderPass.loadFromResolve());
    }

    GrVkImage* colorAttachment() const {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());
        return fColorAttachment.get();
    }
    const GrVkImageView* colorAttachmentView() const {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());
        return this->colorAttachment()->framebufferView();
    }

    GrVkImage* resolveAttachment() const {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());
        return fResolveAttachment.get();
    }
    const GrVkImageView* resolveAttachmentView() const {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());
        return fResolveAttachment->framebufferView();
    }

    // Returns the GrVkImage of the non-msaa attachment. If the color attachment has 1 sample,
    // then the color attachment will be returned. Otherwise, the resolve attachment is returned.
    // Note that in this second case the resolve attachment may be null if this was created by
    // wrapping an msaa VkImage.
    GrVkImage* nonMSAAAttachment() const;

    // Returns the attachment that is used for all external client facing operations. This will be
    // either a wrapped color attachment or the resolve attachment for created VkImages.
    GrVkImage* externalAttachment() const {
        return fResolveAttachment ? fResolveAttachment.get() : fColorAttachment.get();
    }

    const GrVkRenderPass* getSimpleRenderPass(
            bool withResolve,
            bool withStencil,
            SelfDependencyFlags selfDepFlags,
            LoadFromResolve);
    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle(
            bool withResolve,
            bool withStencil,
            SelfDependencyFlags selfDepFlags,
            LoadFromResolve);

    bool wrapsSecondaryCommandBuffer() const { return SkToBool(fExternalFramebuffer); }
    sk_sp<GrVkFramebuffer> externalFramebuffer() const;

    bool canAttemptStencilAttachment(bool useMSAASurface) const override;

    GrBackendRenderTarget getBackendRenderTarget() const override;

    bool getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                  GrVkRenderPass::AttachmentFlags* flags,
                                  bool withResolve,
                                  bool withStencil);

    // Reconstruct the render target attachment information from the programInfo. This includes
    // which attachments the render target will have (color, stencil) and the attachments' formats
    // and sample counts - cf. getAttachmentsDescriptor.
    static void ReconstructAttachmentsDescriptor(const GrVkCaps& vkCaps,
                                                 const GrProgramInfo& programInfo,
                                                 GrVkRenderPass::AttachmentsDescriptor* desc,
                                                 GrVkRenderPass::AttachmentFlags* flags);

protected:
    enum class CreateType {
        kDirectlyWrapped, // We need to register this in the ctor
        kFromTextureRT,   // Skip registering this to cache since TexRT will handle it
    };

    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     sk_sp<GrVkImage> colorAttachment,
                     sk_sp<GrVkImage> resolveImage,
                     CreateType createType,
                     std::string_view label);

    void onAbandon() override;
    void onRelease() override;

    // This returns zero since the memory should all be handled by the attachments
    size_t onGpuMemorySize() const override { return 0; }

    void onSetLabel() override{}

private:
    // For external framebuffers that wrap a secondary command buffer
    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     sk_sp<GrVkFramebuffer> externalFramebuffer,
                     std::string_view label);

    void setFlags();

    GrVkGpu* getVkGpu() const;

    GrVkImage* dynamicMSAAAttachment();
    GrVkImage* msaaAttachment();

    std::pair<const GrVkRenderPass*, GrVkResourceProvider::CompatibleRPHandle>
        createSimpleRenderPass(bool withResolve,
                               bool withStencil,
                               SelfDependencyFlags selfDepFlags,
                               LoadFromResolve);
    void createFramebuffer(bool withResolve,
                           bool withStencil,
                           SelfDependencyFlags selfDepFlags,
                           LoadFromResolve);

    bool completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) override;

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<RefCntedReleaseProc> releaseHelper) override {
        // Forward the release proc on to the GrVkImage of the resolve attachment if we have one,
        // otherwise the color attachment.
        GrVkImage* attachment =
                fResolveAttachment ? fResolveAttachment.get() : fColorAttachment.get();
        attachment->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();

    sk_sp<GrVkImage> fColorAttachment;
    sk_sp<GrVkImage> fResolveAttachment;
    sk_sp<GrVkImage> fDynamicMSAAAttachment;

    // We can have a renderpass with and without resolve attachment, stencil attachment,
    // input attachment dependency, advanced blend dependency, and loading from resolve. All 5 of
    // these being completely orthogonal. Thus we have a total of 32 types of render passes. We then
    // cache a framebuffer for each type of these render passes.
    static constexpr int kNumCachedFramebuffers = 32;

    sk_sp<const GrVkFramebuffer> fCachedFramebuffers[kNumCachedFramebuffers];

    const GrVkDescriptorSet* fCachedInputDescriptorSet = nullptr;

    sk_sp<GrVkFramebuffer> fExternalFramebuffer;
};

#endif
