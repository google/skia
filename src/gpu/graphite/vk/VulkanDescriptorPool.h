/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "include/core/SkRefCnt.h"

#include "include/core/SkSpan.h"
#include "src/gpu/graphite/DescriptorTypes.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanDescriptorPool : public SkRefCnt {
public:
    // Conservative upper bound of number of sets supported per pool.
    static constexpr int kMaxNumSets = 512;
    /**
     * Given a span of descriptor types and counts, a descriptor pool will be created which houses
     * enough of the descriptor types and quantities requested to allocate the maximum number of
     * sets possible (kMaxNumSets). Counts must be > 0.
    */
    static sk_sp<VulkanDescriptorPool> Make(const VulkanSharedContext*,
                                            SkSpan<DescriptorData>,
                                            VkDescriptorSetLayout);

    VkDescriptorPool descPool() { return fDescPool; }

private:
    // Conservative overestimation of a maximum number of descriptors of any given type that can be
    // requested.
    static constexpr int kMaxNumDescriptors = 1024;

    VulkanDescriptorPool(const VulkanSharedContext*,
                         VkDescriptorPool,
                         VkDescriptorSetLayout);
    ~VulkanDescriptorPool() override;

    const VulkanSharedContext*       fSharedContext;
    VkDescriptorPool                 fDescPool;
    // Hang on to the VkDescSetLayout handle used to allocate sets from this pool. Pools are only
    // deleted once sets no longer need them, so we can safely destoy the layout alongside the pool.
    VkDescriptorSetLayout            fDescSetLayout;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
