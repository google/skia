/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexture_DEFINED
#define GrVkTexture_DEFINED

#include "include/gpu/GrTexture.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkImage.h"

class GrVkGpu;
class GrVkImageView;
struct GrVkImageInfo;

class GrVkTexture : public GrTexture, public virtual GrVkImage {
public:
    static sk_sp<GrVkTexture> MakeNewTexture(GrVkGpu*,
                                             SkBudgeted budgeted,
                                             const GrSurfaceDesc&,
                                             const GrVkImage::ImageDesc&,
                                             GrMipMapsStatus);

    static sk_sp<GrVkTexture> MakeWrappedTexture(GrVkGpu*, const GrSurfaceDesc&, GrWrapOwnership,
                                                 GrWrapCacheable, GrIOType, const GrVkImageInfo&,
                                                 sk_sp<GrVkImageLayout>);

    ~GrVkTexture() override;

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    void textureParamsModified() override {}

    const GrVkImageView* textureView();

    void addIdleProc(sk_sp<GrRefCntedCallback>, IdleState) override;
    void callIdleProcsOnBehalfOfResource();

protected:
    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, const GrVkImageInfo&, sk_sp<GrVkImageLayout>,
                const GrVkImageView*, GrMipMapsStatus, GrBackendObjectOwnership);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

    void willRemoveLastRef() override;

private:
    GrVkTexture(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&, const GrVkImageInfo&,
                sk_sp<GrVkImageLayout> layout, const GrVkImageView* imageView,
                GrMipMapsStatus);
    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, const GrVkImageInfo&, sk_sp<GrVkImageLayout>,
                const GrVkImageView*, GrMipMapsStatus, GrBackendObjectOwnership, GrWrapCacheable,
                GrIOType, bool isExternal);

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }

    void removeFinishIdleProcs();

    const GrVkImageView* fTextureView;

    typedef GrTexture INHERITED;
};

#endif
