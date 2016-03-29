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

class GrVkCommandBuffer;
class GrVkFramebuffer;
class GrVkGpu;
class GrVkImageView;
class GrVkStencilAttachment;

struct GrVkTextureInfo;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkRenderTarget: public GrRenderTarget, public virtual GrVkImage {
public:
    static GrVkRenderTarget* CreateNewRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                   GrGpuResource::LifeCycle,
                                                   const GrVkImage::ImageDesc&);

    static GrVkRenderTarget* CreateWrappedRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                       GrGpuResource::LifeCycle,
                                                       const GrVkTextureInfo*);

    ~GrVkRenderTarget() override;

    const GrVkFramebuffer* framebuffer() const { return fFramebuffer; }
    const GrVkImageView* colorAttachmentView() const { return fColorAttachmentView; }
    const GrVkImage::Resource* msaaImageResource() const { return fMSAAImageResource; }
    const GrVkImageView* resolveAttachmentView() const { return fResolveAttachmentView; }
    const GrVkImage::Resource* stencilImageResource() const;
    const GrVkImageView* stencilAttachmentView() const;

    const GrVkRenderPass* simpleRenderPass() const { return fCachedSimpleRenderPass; }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        return kCanResolve_ResolveType;
    }

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    GrBackendObject getRenderTargetHandle() const override;

    // Returns the total number of attachments
    void getAttachmentsDescriptor(GrVkRenderPass::AttachmentsDescriptor* desc,
                                  GrVkRenderPass::AttachmentFlags* flags) const;

    void addResources(GrVkCommandBuffer& commandBuffer) const;

protected:
    enum Derived { kDerived };

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     GrGpuResource::LifeCycle,
                     const GrVkImage::Resource* imageResource,
                     const GrVkImage::Resource* msaaImageResource,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     GrGpuResource::LifeCycle,
                     const GrVkImage::Resource* imageResource,
                     const GrVkImage::Resource* msaaImageResource,
                     const GrVkImageView* colorAttachmentView,
                     const GrVkImageView* resolveAttachmentView,
                     Derived);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     GrGpuResource::LifeCycle,
                     const GrVkImage::Resource* imageResource,
                     const GrVkImageView* colorAttachmentView);

    GrVkRenderTarget(GrVkGpu* gpu,
                     const GrSurfaceDesc& desc,
                     GrGpuResource::LifeCycle,
                     const GrVkImage::Resource* imageResource,
                     const GrVkImageView* colorAttachmentView,
                     Derived);

    static GrVkRenderTarget* Create(GrVkGpu*, const GrSurfaceDesc&,
                                    GrGpuResource::LifeCycle,
                                    const GrVkImage::Resource* imageResource);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        SkASSERT(kUnknown_GrPixelConfig != fDesc.fConfig);
        SkASSERT(!GrPixelConfigIsCompressed(fDesc.fConfig));
        size_t colorBytes = GrBytesPerPixel(fDesc.fConfig);
        SkASSERT(colorBytes > 0);
        return fColorValuesPerPixel * fDesc.fWidth * fDesc.fHeight * colorBytes;
    }

private:
    bool completeStencilAttachment() override;

    void createFramebuffer(GrVkGpu* gpu);

    void releaseInternalObjects();
    void abandonInternalObjects();

    const GrVkFramebuffer*     fFramebuffer;
    const GrVkImageView*       fColorAttachmentView;
    const GrVkImage::Resource* fMSAAImageResource;
    const GrVkImageView*       fResolveAttachmentView;
    int                        fColorValuesPerPixel;

    // This is a cached pointer to a simple render pass. The render target should unref it
    // once it is done with it.
    const GrVkRenderPass*      fCachedSimpleRenderPass;
};

#endif
