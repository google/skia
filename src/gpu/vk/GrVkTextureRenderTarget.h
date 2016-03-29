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
#include "GrVkGpu.h"

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkImageView;
struct GrVkTextureInfo;

class GrVkTextureRenderTarget: public GrVkTexture, public GrVkRenderTarget {
public:
    static GrVkTextureRenderTarget* CreateNewTextureRenderTarget(GrVkGpu*, const GrSurfaceDesc&,
                                                                 GrGpuResource::LifeCycle,
                                                                 const GrVkImage::ImageDesc&);

    static GrVkTextureRenderTarget* CreateWrappedTextureRenderTarget(GrVkGpu*,
                                                                     const GrSurfaceDesc&,
                                                                     GrGpuResource::LifeCycle,
                                                                     VkFormat,
                                                                     const GrVkTextureInfo*);

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
                            const GrSurfaceDesc& desc,
                            GrGpuResource::LifeCycle lifeCycle,
                            const GrVkImage::Resource* imageResource,
                            const GrVkImageView* texView,
                            const GrVkImage::Resource* msaaResource,
                            const GrVkImageView* colorAttachmentView,
                            const GrVkImageView* resolveAttachmentView)
        : GrSurface(gpu, lifeCycle, desc)
        , GrVkImage(imageResource)
        , GrVkTexture(gpu, desc, lifeCycle, imageResource, texView, GrVkTexture::kDerived)
        , GrVkRenderTarget(gpu, desc, lifeCycle, imageResource, msaaResource, colorAttachmentView,
                           resolveAttachmentView, GrVkRenderTarget::kDerived) {
        this->registerWithCache();
    }

    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            const GrSurfaceDesc& desc,
                            GrGpuResource::LifeCycle lifeCycle,
                            const GrVkImage::Resource* imageResource,
                            const GrVkImageView* texView,
                            const GrVkImageView* colorAttachmentView)
        : GrSurface(gpu, lifeCycle, desc)
        , GrVkImage(imageResource)
        , GrVkTexture(gpu, desc, lifeCycle, imageResource, texView, GrVkTexture::kDerived)
        , GrVkRenderTarget(gpu, desc, lifeCycle, imageResource, colorAttachmentView,
                           GrVkRenderTarget::kDerived) {
        this->registerWithCache();
    }

    static GrVkTextureRenderTarget* Create(GrVkGpu*, const GrSurfaceDesc&,
                                           GrGpuResource::LifeCycle,
                                           VkFormat format,
                                           const GrVkImage::Resource* imageResource);

    // GrGLRenderTarget accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        return GrVkRenderTarget::onGpuMemorySize();
    }
};

#endif
