/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanTexture_DEFINED
#define skgpu_graphite_VulkanTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/Texture.h"

#include <utility>

namespace skgpu { class MutableTextureStateRef; }

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanTexture : public Texture {
    struct CreatedImageInfo {
        VkImage fImage = VK_NULL_HANDLE;
        VulkanAlloc fMemoryAlloc;
        sk_sp<MutableTextureStateRef> fMutableState;
    };

    static bool MakeVkImage(const VulkanSharedContext*,
                            SkISize dimensions,
                            const TextureInfo&,
                            CreatedImageInfo* outInfo);

    static sk_sp<Texture> Make(const VulkanSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&,
                               SkBudgeted);

    static sk_sp<Texture> MakeWrapped(const VulkanSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_sp<MutableTextureStateRef>,
                                      VkImage,
                                      const VulkanAlloc&);

    ~VulkanTexture() override {}

    VkImage vkImage() const { return fImage; }

private:
    VulkanTexture(const VulkanSharedContext* sharedContext,
                  SkISize dimensions,
                  const TextureInfo& info,
                  sk_sp<MutableTextureStateRef>,
                  VkImage,
                  const VulkanAlloc&,
                  Ownership,
                  SkBudgeted);

    void freeGpuData() override;

    VkImage fImage;
    VulkanAlloc fMemoryAlloc;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanTexture_DEFINED

