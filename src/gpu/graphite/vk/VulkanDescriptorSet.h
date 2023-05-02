/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorSet_DEFINED
#define skgpu_graphite_VulkanDescriptorSet_DEFINED

#include "src/gpu/graphite/Resource.h"

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
    VulkanDescriptorSet(const VulkanSharedContext*, sk_sp<VulkanDescriptorPool>);

    VkDescriptorSetLayout layout() const { return fDescLayout; }

    bool isAvailable() const { return fIsAvailable; }

    VkDescriptorSet descriptorSet() {
        SkASSERT(fIsAvailable);
        fIsAvailable = false;
        return fDescSet;
    }

    void setAvailability(bool isAvailable) {
        fIsAvailable = isAvailable;
    }

private:
    void freeGpuData() override {
        //TODO: Implement.
    }

    VkDescriptorSet             fDescSet;
    sk_sp<VulkanDescriptorPool> fPool;
    bool                        fIsAvailable;
    VkDescriptorSetLayout       fDescLayout;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorSet_DEFINED
