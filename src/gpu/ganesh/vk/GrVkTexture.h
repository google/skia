/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexture_DEFINED
#define GrVkTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

class GrVkDescriptorSet;
class GrVkGpu;
class GrVkImageView;
struct GrVkImageInfo;
struct SkISize;

namespace skgpu {
class MutableTextureState;
enum class Budgeted : bool;
}  // namespace skgpu

class GrVkTexture : public GrTexture {
public:
    static sk_sp<GrVkTexture> MakeNewTexture(GrVkGpu*,
                                             skgpu::Budgeted budgeted,
                                             SkISize dimensions,
                                             VkFormat format,
                                             uint32_t mipLevels,
                                             GrProtected,
                                             GrMipmapStatus,
                                             std::string_view label);

    static sk_sp<GrVkTexture> MakeWrappedTexture(GrVkGpu*,
                                                 SkISize dimensions,
                                                 GrWrapOwnership,
                                                 GrWrapCacheable,
                                                 GrIOType,
                                                 const GrVkImageInfo&,
                                                 sk_sp<skgpu::MutableTextureState>);

    ~GrVkTexture() override;

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override { return fTexture->backendFormat(); }

    void textureParamsModified() override {}

    GrVkImage* textureImage() const { return fTexture.get(); }
    const GrVkImageView* textureView();

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
                sk_sp<GrVkImage> texture,
                GrMipmapStatus,
                std::string_view label);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImages::BackendTextureReleaseProc*) override {
        return false;
    }

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<RefCntedReleaseProc> releaseHelper) override {
        // Forward the release proc onto the fTexture's GrVkImage
        fTexture->setResourceRelease(std::move(releaseHelper));
    }

private:
    GrVkTexture(GrVkGpu*,
                skgpu::Budgeted,
                SkISize,
                sk_sp<GrVkImage> texture,
                GrMipmapStatus,
                std::string_view label);
    GrVkTexture(GrVkGpu*, SkISize, sk_sp<GrVkImage> texture, GrMipmapStatus,
                GrWrapCacheable,
                GrIOType,
                bool isExternal,
                std::string_view label);

    void onSetLabel() override{}

    sk_sp<GrVkImage> fTexture;

    struct SamplerHash {
        uint32_t operator()(GrSamplerState state) const {
            // In VK the max aniso value is specified in addition to min/mag/mip filters and the
            // driver is encouraged to consider the other filter settings when doing aniso.
            return state.asKey(/*anisoIsOrthogonal=*/true);
        }
    };
    struct DescriptorCacheEntry;
    SkLRUCache<const GrSamplerState, std::unique_ptr<DescriptorCacheEntry>, SamplerHash>
            fDescSetCache;
    static constexpr int kMaxCachedDescSets = 8;
};

#endif
