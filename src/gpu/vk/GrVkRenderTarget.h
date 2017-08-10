/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkRenderTarget_DEFINED
#define GrVkRenderTarget_DEFINED

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
    static GrVkRenderTarget* CreateNewRenderTarget(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&,
                                                   const GrVkImage::ImageDesc&);

    static sk_sp<GrVkRenderTarget> MakeWrappedRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                           const GrVkImageInfo*);

    ~GrVkRenderTarget() override;

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
        return fCompatibleRPHandle;
    }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (this->numColorSamples() > 1) {
            return kCanResolve_ResolveType;
        }
        return kAutoResolves_ResolveType;
    }

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    GrBackendObject getRenderTargetHandle() const override;

    void getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                  GrVkRenderPass::AttachmentFlags* flags) const;

    void addResources(GrVkCommandBuffer& commandBuffer) const;

protected:
    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     const GrVkImageInfo& msaaInfo,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView,
                     GrVkImage::Wrapped wrapped);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     const GrVkImageView* colorAttachmentView,
                     GrVkImage::Wrapped wrapped);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        // The plus 1 is to account for the resolve texture.
        // TODO: is this still correct?
        int numColorSamples = this->numColorSamples() + 1;
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, false);
    }

    void createFramebuffer(GrVkGpu* gpu);

    const GrVkImageView*       fColorAttachmentView;
    std::unique_ptr<GrVkImage> fMSAAImage;
    const GrVkImageView*       fResolveAttachmentView;

private:
    GrVkRenderTarget(GrVkGpu* gpu,
                     SkBudgeted,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     const GrVkImageInfo& msaaInfo,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView,
                     GrVkImage::Wrapped wrapped);

    GrVkRenderTarget(GrVkGpu* gpu,
                     SkBudgeted,
                     const GrSurfaceDesc& desc,
                     const GrVkImageInfo& info,
                     const GrVkImageView* colorAttachmentView,
                     GrVkImage::Wrapped wrapped);

    static GrVkRenderTarget* Create(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&,
                                    const GrVkImageInfo&, GrVkImage::Wrapped wrapped);

    bool completeStencilAttachment() override;

    void releaseInternalObjects();
    void abandonInternalObjects();

    const GrVkFramebuffer*     fFramebuffer;

    // This is a cached pointer to a simple render pass. The render target should unref it
    // once it is done with it.
    const GrVkRenderPass*      fCachedSimpleRenderPass;
    // This is a handle to be used to quickly get compatible GrVkRenderPasses for this render target
    GrVkResourceProvider::CompatibleRPHandle fCompatibleRPHandle;
};

#endif
