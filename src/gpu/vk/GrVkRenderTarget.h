/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkRenderTarget_DEFINED
#define GrVkRenderTarget_DEFINED

#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/vk/GrVkImage.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

class GrVkFramebuffer;
class GrVkGpu;
class GrVkImageView;
class GrVkStencilAttachment;

struct GrVkImageInfo;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkRenderTarget: public GrRenderTarget, public virtual GrVkImage {
public:
    static sk_sp<GrVkRenderTarget> MakeWrappedRenderTarget(GrVkGpu*, SkISize, int sampleCnt,
                                                           const GrVkImageInfo&,
                                                           sk_sp<GrBackendSurfaceMutableStateImpl>);

    static sk_sp<GrVkRenderTarget> MakeSecondaryCBRenderTarget(GrVkGpu*, SkISize,
                                                               const GrVkDrawableInfo& vkInfo);

    ~GrVkRenderTarget() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrVkFramebuffer* getFramebuffer(bool withStencil, bool needsXferBarrier);
    const GrVkImageView* colorAttachmentView() const { return fColorAttachmentView; }
    const GrManagedResource* msaaImageResource() const {
        if (fMSAAImage) {
            return fMSAAImage->fResource;
        }
        return nullptr;
    }
    GrVkImage* msaaImage() { return fMSAAImage.get(); }
    const GrVkImageView* resolveAttachmentView() const { return fResolveAttachmentView; }
    const GrManagedResource* stencilImageResource() const;
    const GrVkImageView* stencilAttachmentView() const;

    const GrVkRenderPass* getSimpleRenderPass(bool withStencil, bool needsXferBarrier);
    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle(bool withStencil,
                                                                        bool needsXferBarrier) {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());

        GrVkResourceProvider::CompatibleRPHandle* pRPHandle;
        if (withStencil) {
            if (needsXferBarrier) {
                pRPHandle = &fCompatibleStencilSelfDepRPHandle;
            } else {
                pRPHandle = &fCompatibleStencilRPHandle;
            }
        } else {
            if (needsXferBarrier) {
                pRPHandle = &fCompatibleSelfDepRPHandle;
            } else {
                pRPHandle = &fCompatibleRPHandle;
            }
        }
        if (!pRPHandle->isValid()) {
            this->createSimpleRenderPass(withStencil, needsXferBarrier);
        }

#ifdef SK_DEBUG
        if (withStencil) {
            if (needsXferBarrier) {
                SkASSERT(pRPHandle->isValid() == SkToBool(fCachedStencilSelfDepRenderPass));
                SkASSERT(fCachedStencilSelfDepRenderPass->hasStencilAttachment());
                SkASSERT(fCachedStencilSelfDepRenderPass->hasSelfDependency());
            } else {
                SkASSERT(pRPHandle->isValid() == SkToBool(fCachedStencilRenderPass));
                SkASSERT(fCachedStencilRenderPass->hasStencilAttachment());
                SkASSERT(!fCachedStencilRenderPass->hasSelfDependency());
            }
        } else {
            if (needsXferBarrier) {
                SkASSERT(pRPHandle->isValid() == SkToBool(fCachedSelfDepRenderPass));
                SkASSERT(!fCachedSelfDepRenderPass->hasStencilAttachment());
                SkASSERT(fCachedSelfDepRenderPass->hasSelfDependency());
            } else {
                SkASSERT(pRPHandle->isValid() == SkToBool(fCachedSimpleRenderPass));
                SkASSERT(!fCachedSimpleRenderPass->hasStencilAttachment());
                SkASSERT(!fCachedSimpleRenderPass->hasSelfDependency());
            }
        }
#endif

        return *pRPHandle;
    }
    const GrVkRenderPass* externalRenderPass() const {
        SkASSERT(this->wrapsSecondaryCommandBuffer());
        // We use the cached simple render pass to hold the external render pass.
        return fCachedSimpleRenderPass;
    }

    bool wrapsSecondaryCommandBuffer() const { return fSecondaryCommandBuffer != VK_NULL_HANDLE; }
    VkCommandBuffer getExternalSecondaryCommandBuffer() const {
        return fSecondaryCommandBuffer;
    }

    bool canAttemptStencilAttachment() const override {
        // We don't know the status of the stencil attachment for wrapped external secondary command
        // buffers so we just assume we don't have one.
        return !this->wrapsSecondaryCommandBuffer();
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    void getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                  GrVkRenderPass::AttachmentFlags* flags,
                                  bool withStencil) const;

    // Reconstruct the render target attachment information from the programInfo. This includes
    // which attachments the render target will have (color, stencil) and the attachments' formats
    // and sample counts - cf. getAttachmentsDescriptor.
    static void ReconstructAttachmentsDescriptor(const GrVkCaps& vkCaps,
                                                 const GrProgramInfo& programInfo,
                                                 GrVkRenderPass::AttachmentsDescriptor* desc,
                                                 GrVkRenderPass::AttachmentFlags* flags);

    void addResources(GrVkCommandBuffer& commandBuffer, bool withStencil, bool needsXferBarrier);

    void addWrappedGrSecondaryCommandBuffer(std::unique_ptr<GrVkSecondaryCommandBuffer> cmdBuffer) {
        fGrSecondaryCommandBuffers.push_back(std::move(cmdBuffer));
    }

protected:
    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     int sampleCnt,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     const GrVkImageInfo& msaaInfo,
                     sk_sp<GrBackendSurfaceMutableStateImpl> msaaMutableState,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView,
                     GrBackendObjectOwnership);

    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     const GrVkImageView* colorAttachmentView,
                     GrBackendObjectOwnership);

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolved VkImage.
            numColorSamples += 1;
        }
        const GrCaps& caps = *this->getGpu()->caps();
        return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                      numColorSamples, GrMipmapped::kNo);
    }

private:
    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     int sampleCnt,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     const GrVkImageInfo& msaaInfo,
                     sk_sp<GrBackendSurfaceMutableStateImpl> msaaMutableState,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView);

    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     const GrVkImageView* colorAttachmentView);

    GrVkRenderTarget(GrVkGpu* gpu,
                     SkISize dimensions,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     const GrVkRenderPass* renderPass,
                     VkCommandBuffer secondaryCommandBuffer);

    GrVkGpu* getVkGpu() const;

    const GrVkRenderPass* createSimpleRenderPass(bool withStencil, bool needsXferBarrier);
    const GrVkFramebuffer* createFramebuffer(bool withStencil, bool needsXferBarrier);

    bool completeStencilAttachment() override;

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();

    const GrVkImageView*       fColorAttachmentView;
    std::unique_ptr<GrVkImage> fMSAAImage;
    const GrVkImageView*       fResolveAttachmentView;

    const GrVkFramebuffer*     fCachedFramebuffer = nullptr;
    const GrVkFramebuffer*     fCachedStencilFramebuffer = nullptr;
    const GrVkFramebuffer*     fCachedSelfDepFramebuffer = nullptr;
    const GrVkFramebuffer*     fCachedStencilSelfDepFramebuffer = nullptr;

    // Cached pointers to a simple, stencil, and self dependency render passes. The render target
    // should unref them once it is done with them.
    const GrVkRenderPass*      fCachedSimpleRenderPass = nullptr;
    const GrVkRenderPass*      fCachedStencilRenderPass = nullptr;
    const GrVkRenderPass*      fCachedSelfDepRenderPass = nullptr;
    const GrVkRenderPass*      fCachedStencilSelfDepRenderPass = nullptr;

    // This is a handle to be used to quickly get a GrVkRenderPass that is compatible with
    // this render target if its stencil buffer is ignored.
    GrVkResourceProvider::CompatibleRPHandle fCompatibleRPHandle;
    // Same as above but taking the render target's stencil buffer into account
    GrVkResourceProvider::CompatibleRPHandle fCompatibleStencilRPHandle;
    // RenderPass where there is also a self dependency to be used for advanced blending barriers.
    // If the the gpu doesn't support VK_EXT_blend_operation_advanced the render pass will also have
    // an input attachment to be used for dst reads.
    GrVkResourceProvider::CompatibleRPHandle fCompatibleSelfDepRPHandle;
    // Same as above but taking the render target's stencil buffer into account
    GrVkResourceProvider::CompatibleRPHandle fCompatibleStencilSelfDepRPHandle;

    // If this render target wraps an external VkCommandBuffer, then this handle will be that
    // VkCommandBuffer and not VK_NULL_HANDLE. In this case the render target will not be backed by
    // an actual VkImage and will thus be limited in terms of what it can be used for.
    VkCommandBuffer fSecondaryCommandBuffer = VK_NULL_HANDLE;
    // When we wrap a secondary command buffer, we will record GrManagedResources onto it which need
    // to be kept alive till the command buffer gets submitted and the GPU has finished. However, in
    // the wrapped case, we don't know when the command buffer gets submitted and when it is
    // finished on the GPU since the client is in charge of that. However, we do require that the
    // client keeps the GrVkSecondaryCBDrawContext alive and call releaseResources on it once the
    // GPU is finished all the work. Thus we can use this to manage the lifetime of our
    // GrVkSecondaryCommandBuffers. By storing them on the GrVkRenderTarget, which is owned by the
    // SkGpuDevice on the GrVkSecondaryCBDrawContext, we assure that the GrManagedResources held by
    // the GrVkSecondaryCommandBuffer don't get deleted before they are allowed to.
    SkTArray<std::unique_ptr<GrVkCommandBuffer>> fGrSecondaryCommandBuffers;
};

#endif
