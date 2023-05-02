/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "include/core/SkRefCnt.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include <map>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * TODO: Implement methods of this wrapper class. Defining the header to allow us to reference this
 * class in VulkanDescriptorSet.
 */
class VulkanDescriptorPool : public SkRefCnt {
public:
    // TODO: Finalize argument list.
    static sk_sp<VulkanDescriptorPool> Make(const VulkanSharedContext*);

    VkDescriptorPool descPool() const { return fDescPool; }

private:
    // Conservative upper bound of number of sets supported per pool.
    static constexpr int kMaxNumSets = 512;

    VulkanDescriptorPool(const VulkanSharedContext*,
                         VkDescriptorPool);
    ~VulkanDescriptorPool() override;

    const VulkanSharedContext*       fSharedContext;
    VkDescriptorPool                 fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
