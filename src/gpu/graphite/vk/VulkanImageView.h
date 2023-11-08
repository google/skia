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
#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"

namespace skgpu::graphite {

class VulkanSharedContext;
class VulkanResourceProvider;

/*
 * VulkanImageView is not derived from Resource as its lifetime is dependent on the lifetime of
 * its associated VulkanTexture. Hence VulkanTexture will act as a container for its ImageViews
 * w.r.t. the ResourceCache and CommandBuffer, and is responsible for deleting its ImageView
 * children when freeGpuData() is called.
 */
class VulkanImageView {
public:
    enum class Usage {
        kShaderInput,
        kAttachment
    };

    static std::unique_ptr<const VulkanImageView> Make(
            const VulkanSharedContext* sharedContext,
            VkImage image,
            VkFormat format,
            Usage usage,
            uint32_t miplevels,
            sk_sp<VulkanSamplerYcbcrConversion>);
    ~VulkanImageView();

    VkImageView imageView() const { return fImageView; }
    Usage usage() const { return fUsage; }

private:
    VulkanImageView(const VulkanSharedContext*, VkImageView, Usage,
                    sk_sp<VulkanSamplerYcbcrConversion>);

    // Since we're not derived from Resource we need to store the context for deletion later
    const VulkanSharedContext* fSharedContext;
    VkImageView  fImageView;
    Usage fUsage;
    sk_sp<VulkanSamplerYcbcrConversion> fYcbcrConversion;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanImageView_DEFINED
