/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "GrVkGpu.h"
#include "GrVkResource.h"
#include "GrVkSemaphore.h"
#include "GrVkUtil.h"
#include "vk/GrVkDefines.h"
#include "GrVkInterface.h"

class GrVkCommandBuffer;
class GrVkGpu;

class GrVkCommandPool : public GrVkResource {
public:
    GrVkCommandPool() = default;

    GrVkCommandPool(VkCommandPool cmdPool)
        : fCmdPool(cmdPool) {}

    ~GrVkCommandPool() override {
        SkASSERT(fCmdPool == VK_NULL_HANDLE);
    }

    VkCommandPool vkCommandPool() const {
        return fCmdPool;
    }

    void reset(GrVkGpu* gpu);

    void freeGPUData(GrVkGpu* gpu) const override;

#ifdef SK_DEBUG
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %p (%d refs)\n", fCmdPool, this->getRefCnt());
    }
#endif

private:
    mutable VkCommandPool fCmdPool;
};

#endif
