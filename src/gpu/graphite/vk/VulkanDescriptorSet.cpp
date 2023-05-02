/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fPool (pool)
        , fIsAvailable (true) {
    fPool->ref();
}

} // namespace skgpu::graphite
