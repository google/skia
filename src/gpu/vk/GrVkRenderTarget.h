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
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

class GrVkCommandBuffer;
class GrVkFramebuffer;
class GrVkGpu;
class GrVkImageView;
class GrVkSecondaryCommandBuffer;
class GrVkStencilAttachment;

struct GrVkImageInfo;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkRenderTarget: public GrRenderTarget, public virtual GrVkImage {
public:
    static sk_sp<GrVkRenderTarget> MakeWrappedRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                           int sampleCnt, const GrVkImageInfo&,
                                                           sk_sp<GrVkImageLayout>);

    static sk_sp<GrVkRenderTarget> MakeSecondaryCBRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                               const GrVkDrawableInfo& vkInfo);

    ~GrVkRenderTarget() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrVkFramebuffer* framebuffer() const { return fFramebuffer; }
    const GrVkImageView* colorAttachmentView() const { return fColorAttachmentView; }
    const GrVkResource* msaaImageResource() const {
        if (fMSAAImage) {
            return fMSAAImage->fResource;
        }
        return nullptr;
    }
    GrVkImage* msaaImage() { return fMSAAImage.get(); }
    const GrVkImageView* resolveAttachmentView() const { return fResolveAttachmentView; }
    const GrVkResource* stencilImageResource() const;
    const GrVkImageView* stencilAttachmentView() const;

    const GrVkRenderPass* simpleRenderPass() const { return fCachedSimpleRenderPass; }
    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle() const {
        SkASSERT(!this->wrapsSecondaryCommandBuffer());
        return fCompatibleRPHandle;
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
                                  GrVkRenderPass::AttachmentFlags* flags) const;

    void addResources(GrVkCommandBuffer& commandBuffer) const;

protected:
    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     int sampleCnt,
                     const GrVkImageInfo& info,
                     sk_sp<GrVkImageLayout> layout,
                     const GrVkImageInfo& msaaInfo,
                     sk_sp<GrVkImageLayout> msaaLayout,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView,
                     GrBackendObjectOwnership);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     sk_sp<GrVkImageLayout> layout,
                     const GrVkImageView* colorAttachmentView,
                     GrBackendObjectOwnership);

    GrVkGpu* getVkGpu() const;

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
                                      numColorSamples, GrMipMapped::kNo);
    }

    void createFramebuffer(GrVkGpu* gpu);

    const GrVkImageView*       fColorAttachmentView;
    std::unique_ptr<GrVkImage> fMSAAImage;
    const GrVkImageView*       fResolveAttachmentView;

private:
    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     int sampleCnt,
                     const GrVkImageInfo& info,
                     sk_sp<GrVkImageLayout> layout,
                     const GrVkImageInfo& msaaInfo,
                     sk_sp<GrVkImageLayout> msaaLayout,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     sk_sp<GrVkImageLayout> layout,
                     const GrVkImageView* colorAttachmentView);


    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     sk_sp<GrVkImageLayout> layout,
                     const GrVkRenderPass* renderPass,
                     VkCommandBuffer secondaryCommandBuffer);

    bool completeStencilAttachment() override;

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();
    void abandonInternalObjects();

    const GrVkFramebuffer*     fFramebuffer;

    // This is a cached pointer to a simple render pass. The render target should unref it
    // once it is done with it.
    const GrVkRenderPass*      fCachedSimpleRenderPass;
    // This is a handle to be used to quickly get compatible GrVkRenderPasses for this render target
    GrVkResourceProvider::CompatibleRPHandle fCompatibleRPHandle;

    // If this render target wraps an external VkCommandBuffer, then this handle will be that
    // VkCommandBuffer and not VK_NULL_HANDLE. In this case the render target will not be backed by
    // an actual VkImage and will thus be limited in terms of what it can be used for.
    VkCommandBuffer fSecondaryCommandBuffer = VK_NULL_HANDLE;
};

#endif
