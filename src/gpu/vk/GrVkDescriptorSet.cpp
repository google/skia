/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkDescriptorSet.h"

#include "src/gpu/vk/GrVkDescriptorPool.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

GrVkDescriptorSet::GrVkDescriptorSet(VkDescriptorSet descSet,
                                     GrVkDescriptorPool* pool,
                                     GrVkDescriptorSetManager::Handle handle)
    : fDescSet(descSet)
    , fPool(pool)
    , fHandle(handle) {
    fPool->ref();
}

void GrVkDescriptorSet::freeGPUData(GrVkGpu* gpu) const {
    fPool->unref(gpu);
}

void GrVkDescriptorSet::onRecycle(GrVkGpu* gpu) const {
    gpu->resourceProvider().recycleDescriptorSet(this, fHandle);
}

void GrVkDescriptorSet::abandonGPUData() const {
    fPool->unrefAndAbandon();
}

