/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkManagedResource.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

class GrVkPrimaryCommandBuffer;
class GrVkSecondaryCommandBuffer;
class GrVkGpu;

class GrVkCommandPool : public GrVkRecycledResource {
public:
    static GrVkCommandPool* Create(GrVkGpu* gpu);

    VkCommandPool vkCommandPool() const {
        return fCommandPool;
    }

    void allocatePrimaryCommandBuffer();

    void reset(GrVkGpu* gpu);

    void releaseResources();

    GrVkPrimaryCommandBuffer* getPrimaryCommandBuffer() {
        return fActivePrimaryCommandBuffers.back().get();
    }

    std::unique_ptr<GrVkSecondaryCommandBuffer> findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

    void recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* buffer);

    // marks that we are finished with this command pool; it is not legal to continue creating or
    // writing to command buffers in a closed pool
    void close();

    // returns true if close() has not been called
    bool isOpen() const { return fOpen; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %p (%d refs)\n", fCommandPool, this->getRefCnt());
    }
#endif

private:
    GrVkCommandPool() = delete;

    GrVkCommandPool(GrVkGpu* gpu, VkCommandPool commandPool);

    void freeGPUData() const override;
    void onRecycle() const override;

    bool fOpen = true;

    VkCommandPool fCommandPool;

    SkSTArray<4, std::unique_ptr<GrVkPrimaryCommandBuffer>> fActivePrimaryCommandBuffers;
    SkSTArray<4, std::unique_ptr<GrVkPrimaryCommandBuffer>> fAvaliablePrimaryCommandBuffers;

    // Array of available secondary command buffers that are not in flight
    SkSTArray<4, std::unique_ptr<GrVkSecondaryCommandBuffer>, true> fAvailableSecondaryBuffers;
    int fMaxCachedSecondaryCommandBuffers;
};

#endif
