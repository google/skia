/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexture_DEFINED
#define GrVkTexture_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/vk/GrVkImage.h"

class GrVkDescriptorSet;
class GrVkGpu;
class GrVkImageView;
struct GrVkImageInfo;

class GrVkTexture : public GrTexture, public virtual GrVkImage {
public:
    static sk_sp<GrVkTexture> MakeNewTexture(GrVkGpu*,
                                             SkBudgeted budgeted,
                                             SkISize dimensions,
                                             const GrVkImage::ImageDesc&,
                                             GrMipmapStatus);

    static sk_sp<GrVkTexture> MakeWrappedTexture(GrVkGpu*,
                                                 SkISize dimensions,
                                                 GrWrapOwnership,
                                                 GrWrapCacheable,
                                                 GrIOType,
                                                 const GrVkImageInfo&,
                                                 sk_sp<GrBackendSurfaceMutableStateImpl>);

    ~GrVkTexture() override;

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    void textureParamsModified() override {}

    const GrVkImageView* textureView();

    void addIdleProc(sk_sp<GrRefCntedCallback>) override;
    void callIdleProcsOnBehalfOfResource() override;

    // For each GrVkTexture, there is a cache of GrVkDescriptorSets which only contain a single
    // texture/sampler descriptor. If there is a cached descriptor set that matches the passed in
    // GrSamplerState, then a pointer to it is returned. The ref count is not incremented on the
    // returned pointer, thus the caller must call ref it if they wish to keep ownership of the
    // GrVkDescriptorSet.
    const GrVkDescriptorSet* cachedSingleDescSet(GrSamplerState);

    void addDescriptorSetToCache(const GrVkDescriptorSet*, GrSamplerState);

protected:
    GrVkTexture(GrVkGpu*,
                SkISize dimensions,
                const GrVkImageInfo&,
                sk_sp<GrBackendSurfaceMutableStateImpl>,
                sk_sp<const GrVkImageView>,
                GrMipmapStatus,
                GrBackendObjectOwnership);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

    void willRemoveLastRef() override;

private:
    GrVkTexture(GrVkGpu*, SkBudgeted, SkISize, const GrVkImageInfo&,
                sk_sp<GrBackendSurfaceMutableStateImpl>, sk_sp<const GrVkImageView> imageView,
                GrMipmapStatus);
    GrVkTexture(GrVkGpu*, SkISize, const GrVkImageInfo&, sk_sp<GrBackendSurfaceMutableStateImpl>,
                sk_sp<const GrVkImageView>, GrMipmapStatus, GrBackendObjectOwnership,
                GrWrapCacheable, GrIOType, bool isExternal);

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }

    void removeFinishIdleProcs();

    sk_sp<const GrVkImageView> fTextureView;

    struct SamplerHash {
        uint32_t operator()(GrSamplerState state) const { return state.asIndex(); }
    };
    struct DescriptorCacheEntry;
    SkLRUCache<const GrSamplerState, std::unique_ptr<DescriptorCacheEntry>, SamplerHash>
            fDescSetCache;
    static constexpr int kMaxCachedDescSets = 8;

    using INHERITED = GrTexture;
};

#endif
