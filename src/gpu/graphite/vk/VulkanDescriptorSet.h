/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorSet_DEFINED
#define skgpu_graphite_VulkanDescriptorSet_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/DescriptorTypes.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanDescriptorPool;
class VulkanSharedContext;

/**
 * Wrapper around VkDescriptorSet which maintains a reference to its descriptor pool. Once the ref
 * count on that pool is 0, it will be destroyed.
*/
class VulkanDescriptorSet : public Resource {
public:
    static sk_sp<VulkanDescriptorSet> Make(const VulkanSharedContext*,
                                           sk_sp<VulkanDescriptorPool>,
                                           const VkDescriptorSetLayout);

    VulkanDescriptorSet(const VulkanSharedContext*,
                        VkDescriptorSet,
                        sk_sp<VulkanDescriptorPool>);

    const VkDescriptorSet* descriptorSet() { return &fDescSet; }

private:
    void freeGpuData() override;

    VkDescriptorSet             fDescSet;
    // Have this class hold on to a reference of the descriptor pool. When a pool's reference count
    // is 0, that means all the descriptor sets that came from that pool are no longer needed, so
    // the pool can safely be destroyed.
    sk_sp<VulkanDescriptorPool> fPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorSet_DEFINED
