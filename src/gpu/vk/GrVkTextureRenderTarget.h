/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkTextureRenderTarget_DEFINED
#define GrVkTextureRenderTarget_DEFINED

#include "GrVkTexture.h"
#include "GrVkRenderTarget.h"

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
    static sk_sp<GrVkTextureRenderTarget> CreateNewTextureRenderTarget(GrVkGpu*, SkBudgeted,
                                                                       const GrSurfaceDesc&,
                                                                       const GrVkImage::ImageDesc&);

    static sk_sp<GrVkTextureRenderTarget> MakeWrappedTextureRenderTarget(GrVkGpu*,
                                                                         const GrSurfaceDesc&,
                                                                         GrWrapOwnership,
                                                                         const GrVkImageInfo*);

    bool updateForMipmap(GrVkGpu* gpu, const GrVkImageInfo& newInfo);

protected:
    void onAbandon() override {
        GrVkRenderTarget::onAbandon();
        GrVkTexture::onAbandon();
    }

    void onRelease() override {
        GrVkRenderTarget::onRelease();
        GrVkTexture::onRelease();
    }

private:
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            SkBudgeted budgeted,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            const GrVkImageView* texView,
                            const GrVkImageInfo& msaaInfo,
                            const GrVkImageView* colorAttachmentView,
                            const GrVkImageView* resolveAttachmentView);

    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            SkBudgeted budgeted,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            const GrVkImageView* texView,
                            const GrVkImageView* colorAttachmentView);

    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            const GrVkImageView* texView,
                            const GrVkImageInfo& msaaInfo,
                            const GrVkImageView* colorAttachmentView,
                            const GrVkImageView* resolveAttachmentView,
                            GrVkImage::Wrapped wrapped);

    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            const GrSurfaceDesc& desc,
                            const GrVkImageInfo& info,
                            const GrVkImageView* texView,
                            const GrVkImageView* colorAttachmentView,
                            GrVkImage::Wrapped wrapped);

    static sk_sp<GrVkTextureRenderTarget> Make(GrVkGpu*,
                                               const GrSurfaceDesc&,
                                               const GrVkImageInfo&,
                                               SkBudgeted budgeted,
                                               GrVkImage::Wrapped wrapped);

    // GrGLRenderTarget accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override;
};

#endif
