/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkTextureRenderTarget_DEFINED
#define GrVkTextureRenderTarget_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkTexture.h"

class GrVkGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkImageView;
struct GrVkImageInfo;

class GrVkTextureRenderTarget: public GrVkTexture, public GrVkRenderTarget {
public:
    static sk_sp<GrVkTextureRenderTarget> MakeNewTextureRenderTarget(GrVkGpu*, SkBudgeted,
                                                                     const GrSurfaceDesc&,
                                                                     const GrVkImage::ImageDesc&,
                                                                     GrMipMapsStatus);

    static sk_sp<GrVkTextureRenderTarget> MakeWrappedTextureRenderTarget(GrVkGpu*,
                                                                         const GrSurfaceDesc&,
                                                                         GrWrapOwnership,
                                                                         GrWrapCacheable,
                                                                         const GrVkImageInfo&,
                                                                         sk_sp<GrVkImageLayout>);

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

protected:
    void onAbandon() override {
        // In order to correctly handle calling texture idle procs, GrVkTexture must go first.
        GrVkTexture::onAbandon();
        GrVkRenderTarget::onAbandon();
    }

    void onRelease() override {
        // In order to correctly handle calling texture idle procs, GrVkTexture must go first.
        GrVkTexture::onRelease();
        GrVkRenderTarget::onRelease();
    }

private:
    // MSAA, not-wrapped
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            SkBudgeted budgeted,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            sk_sp<GrVkImageLayout> layout,
                            const GrVkImageView* texView,
                            const GrVkImageInfo& msaaInfo,
                            sk_sp<GrVkImageLayout> msaaLayout,
                            const GrVkImageView* colorAttachmentView,
                            const GrVkImageView* resolveAttachmentView,
                            GrMipMapsStatus);

    // non-MSAA, not-wrapped
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            SkBudgeted budgeted,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            sk_sp<GrVkImageLayout> layout,
                            const GrVkImageView* texView,
                            const GrVkImageView* colorAttachmentView,
                            GrMipMapsStatus);

    // MSAA, wrapped
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            sk_sp<GrVkImageLayout> layout,
                            const GrVkImageView* texView,
                            const GrVkImageInfo& msaaInfo,
                            sk_sp<GrVkImageLayout> msaaLayout,
                            const GrVkImageView* colorAttachmentView,
                            const GrVkImageView* resolveAttachmentView,
                            GrMipMapsStatus,
                            GrBackendObjectOwnership,
                            GrWrapCacheable);

    // non-MSAA, wrapped
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            sk_sp<GrVkImageLayout> layout,
                            const GrVkImageView* texView,
                            const GrVkImageView* colorAttachmentView,
                            GrMipMapsStatus,
                            GrBackendObjectOwnership,
                            GrWrapCacheable);

    // GrGLRenderTarget accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override;

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }
};

#endif
