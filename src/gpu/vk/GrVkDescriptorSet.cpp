/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkDescriptorSet.h"

#include "GrVkDescriptorPool.h"
#include "GrVkGpu.h"
#include "GrVkResourceProvider.h"

GrVkDescriptorSet::GrVkDescriptorSet(VkDescriptorSet descSet,
                                     GrVkDescriptorPool* pool,
                                     GrVkDescriptorSetManager::Handle handle)
    : fDescSet(descSet)
    , fPool(pool)
    , fHandle(handle) {
    fPool->ref();
}

void GrVkDescriptorSet::freeGPUData(const GrVkGpu* gpu) const {
    fPool->unref(gpu);
}

void GrVkDescriptorSet::onRecycle(GrVkGpu* gpu) const {
    gpu->resourceProvider().recycleDescriptorSet(this, fHandle);
}

void GrVkDescriptorSet::abandonGPUData() const {
    fPool->unrefAndAbandon();
}

