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

GrVkDescriptorSet::GrVkDescriptorSet(GrVkGpu* gpu,
                                     VkDescriptorSet descSet,
                                     GrVkDescriptorPool* pool,
                                     GrVkDescriptorSetManager::Handle handle)
    : INHERITED(gpu)
    , fDescSet(descSet)
    , fPool(pool)
    , fHandle(handle) {
    fPool->ref();
}

void GrVkDescriptorSet::freeGPUData() const {
    fPool->unref();
}

void GrVkDescriptorSet::onRecycle() const {
    fGpu->resourceProvider().recycleDescriptorSet(this, fHandle);
}

