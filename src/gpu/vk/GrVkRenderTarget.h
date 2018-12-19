/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkRenderTarget_DEFINED
#define GrVkRenderTarget_DEFINED

#include "GrVkVulkan.h"

#include "GrVkImage.h"
#include "GrRenderTarget.h"

#include "GrVkRenderPass.h"
#include "GrVkResourceProvider.h"

class GrVkCommandBuffer;
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
    static sk_sp<GrVkRenderTarget> MakeWrappedRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                           const GrVkImageInfo&,
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

    bool wrapsSecondaryCommandBuffer() const { return fSecondaryCommandBuffer != VK_NULL_HANDLE; }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (this->numColorSamples() > 1) {
            return kCanResolve_ResolveType;
        }
        return kAutoResolves_ResolveType;
    }

    bool canAttemptStencilAttachment() const override {
        return !this->wrapsSecondaryCommandBuffer();
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    void getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                  GrVkRenderPass::AttachmentFlags* flags) const;

    void addResources(GrVkCommandBuffer& commandBuffer) const;

protected:
    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
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
        int numColorSamples = this->numColorSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolved VkImage.
            numColorSamples += 1;
        }
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo);
    }

    void createFramebuffer(GrVkGpu* gpu);

    const GrVkImageView*       fColorAttachmentView;
    std::unique_ptr<GrVkImage> fMSAAImage;
    const GrVkImageView*       fResolveAttachmentView;

private:
    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
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
                     VkRenderPass renderPass,
                     uint32_t colorAttachmentIndex,
                     VkCommandBuffer secondaryCommandBuffer);

    bool completeStencilAttachment() override;

    void releaseInternalObjects();
    void abandonInternalObjects();

    const GrVkFramebuffer*     fFramebuffer;

    // This is a cached pointer to a simple render pass. The render target should unref it
    // once it is done with it.
    const GrVkRenderPass*      fCachedSimpleRenderPass;
    // This is a handle to be used to quickly get compatible GrVkRenderPasses for this render target
    GrVkResourceProvider::CompatibleRPHandle fCompatibleRPHandle;

    // Handle to an external secondary which this GrVkRenderTarget represents. If this is not
    // VK_NULL_HANDLE then the GrVkRenderTarget does not have a real VkImage backing it, and is
    // limited to what it can be used for.
    VkCommandBuffer fSecondaryCommandBuffer = VK_NULL_HANDLE;
};

#endif
