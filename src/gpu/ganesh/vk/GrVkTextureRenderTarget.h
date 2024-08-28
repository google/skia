/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTextureRenderTarget_DEFINED
#define GrVkTextureRenderTarget_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/vk/GrVkRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

class GrVkGpu;
class GrVkImage;
struct GrVkImageInfo;
struct SkISize;

namespace skgpu {
class MutableTextureState;
enum class Budgeted : bool;
}  // namespace skgpu

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrVkTextureRenderTarget: public GrVkTexture, public GrVkRenderTarget {
public:
    static sk_sp<GrVkTextureRenderTarget> MakeNewTextureRenderTarget(GrVkGpu* gpu,
                                                                     skgpu::Budgeted budgeted,
                                                                     SkISize dimensions,
                                                                     VkFormat format,
                                                                     uint32_t mipLevels,
                                                                     int sampleCnt,
                                                                     GrMipmapStatus mipmapStatus,
                                                                     GrProtected isProtected,
                                                                     std::string_view label);

    static sk_sp<GrVkTextureRenderTarget> MakeWrappedTextureRenderTarget(
            GrVkGpu*,
            SkISize dimensions,
            int sampleCnt,
            GrWrapOwnership,
            GrWrapCacheable,
            const GrVkImageInfo&,
            sk_sp<skgpu::MutableTextureState>);

    GrBackendFormat backendFormat() const override { return GrVkTexture::backendFormat(); }

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
    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            skgpu::Budgeted budgeted,
                            SkISize dimensions,
                            sk_sp<GrVkImage> texture,
                            sk_sp<GrVkImage> colorAttachment,
                            sk_sp<GrVkImage> resolveAttachment,
                            GrMipmapStatus,
                            std::string_view label);

    GrVkTextureRenderTarget(GrVkGpu* gpu,
                            SkISize dimensions,
                            sk_sp<GrVkImage> texture,
                            sk_sp<GrVkImage> colorAttachment,
                            sk_sp<GrVkImage> resolveAttachment,
                            GrMipmapStatus,
                            GrWrapCacheable,
                            std::string_view label);

    size_t onGpuMemorySize() const override;

    void onSetLabel() override{}

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<RefCntedReleaseProc> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        GrVkTexture::onSetRelease(std::move(releaseHelper));
    }
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
