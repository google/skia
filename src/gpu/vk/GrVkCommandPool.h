/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "GrVkInterface.h"
#include "GrVkResource.h"
#include "GrVkResourceProvider.h"

#include <mutex>

class GrVkPrimaryCommandBuffer;
class GrVkSecondaryCommandBuffer;
class GrVkGpu;

class GrVkCommandPool : public GrVkResource {
public:
    GrVkCommandPool() = delete;

    GrVkCommandPool(VkCommandPool commandPool)
        : fCommandPool(commandPool) {
    }

    ~GrVkCommandPool() override {
        SkASSERT(!fActivePrimaryBuffers.count());
        SkASSERT(!fAvailablePrimaryBuffers.count());
        SkASSERT(!fAvailableSecondaryBuffers.count());
    }

    VkCommandPool vkCommandPool() const {
        return fCommandPool;
    }

    void reset(GrVkGpu* gpu);

    void releaseResources(GrVkGpu* gpu);

    void abandonGPUData() const override;

    void freeGPUData(GrVkGpu* gpu) const override;

    GrVkPrimaryCommandBuffer* findOrCreatePrimaryCommandBuffer(GrVkGpu* gpu);

    GrVkSecondaryCommandBuffer* findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

    void recyclePrimaryCommandBuffer(GrVkGpu* gpu, GrVkPrimaryCommandBuffer* buffer);

    void recycleSecondaryCommandBuffer(GrVkGpu* gpu, GrVkSecondaryCommandBuffer* buffer);

    void checkCommandBuffers(GrVkResourceProvider* provider, GrVkGpu* gpu);

    void close();

    bool isFinished() const;

    std::recursive_mutex& mutex() { return fMutex; }

#ifdef SK_DEBUG
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %p (%d refs)\n", this, this->getRefCnt());
    }
#endif

private:
    bool fOpen = true;

    VkCommandPool fCommandPool;

    // Array of PrimaryCommandBuffers that are currently in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true> fActivePrimaryBuffers;

    // Array of available primary command buffers that are not in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true> fAvailablePrimaryBuffers;

    // Array of available secondary command buffers that are not in flight
    SkSTArray<4, GrVkSecondaryCommandBuffer*, true> fAvailableSecondaryBuffers;

    std::recursive_mutex fMutex;
};

#endif
