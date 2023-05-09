/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanImageView_DEFINED
#define skgpu_graphite_VulkanImageView_DEFINED

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanImageView : public Resource {
public:
    enum class Type {
        kColor,
        kStencil
    };

    static sk_sp<const VulkanImageView> Make(VulkanSharedContext* sharedContext,
                                             VkImage image,
                                             VkFormat format,
                                             Type viewType,
                                             uint32_t miplevels);

    VkImageView imageView() const { return fImageView; }

private:
    VulkanImageView(const VulkanSharedContext* sharedContext, VkImageView imageView);

    void freeGpuData() override;

    VkImageView  fImageView;
    // TODO: track associated SamplerYcbcrConversion
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanImageView_DEFINED
