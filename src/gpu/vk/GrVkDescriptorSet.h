/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkDescriptorSet_DEFINED
#define GrVkDescriptorSet_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/vk/GrVkResource.h"

class GrVkDescriptorPool;
class GrVkGpu;

class GrVkDescriptorSet : public GrVkRecycledResource {
public:
    GrVkDescriptorSet(VkDescriptorSet descSet,
                      GrVkDescriptorPool* pool,
                      GrVkDescriptorSetManager::Handle handle);

    ~GrVkDescriptorSet() override {}

    const VkDescriptorSet* descriptorSet() const { return &fDescSet; }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkDescriptorSet: %d (%d refs)\n", fDescSet, this->getRefCnt());
    }
#endif

private:
    void freeGPUData(GrVkGpu* gpu) const override;
    void onRecycle(GrVkGpu* gpu) const override;

    VkDescriptorSet                          fDescSet;
    SkDEBUGCODE(mutable) GrVkDescriptorPool* fPool;
    GrVkDescriptorSetManager::Handle         fHandle;
};

#endif
