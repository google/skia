/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "GrVkGpuCommandBuffer.h"
#include "GrVkInterface.h"
#include "GrVkResource.h"
#include "GrVkResourceProvider.h"

class GrVkPrimaryCommandBuffer;
class GrVkSecondaryCommandBuffer;
class GrVkGpu;

class GrVkCommandPool : public GrVkResource {
public:
    static GrVkCommandPool* Create(const GrVkGpu* gpu);

    VkCommandPool vkCommandPool() const {
        return fCommandPool;
    }

    void reset(GrVkGpu* gpu);

    void releaseResources(GrVkGpu* gpu);

    GrVkPrimaryCommandBuffer* getPrimaryCommandBuffer() { return fPrimaryCommandBuffer; }

    GrVkSecondaryCommandBuffer* findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

    void recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* buffer);

    // marks that we are finished with this command pool; it is not legal to continue creating or
    // writing to command buffers in a closed pool
    void close();

    // returns true if close() has not been called
    bool isOpen() const { return fOpen; }

#ifdef SK_DEBUG
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %p (%d refs)\n", fCommandPool, this->getRefCnt());
    }
#endif

private:
    GrVkCommandPool() = delete;

    GrVkCommandPool(const GrVkGpu* gpu, VkCommandPool commandPool);

    void abandonGPUData() const override;

    void freeGPUData(GrVkGpu* gpu) const override;

    bool fOpen = true;

    VkCommandPool fCommandPool;

    GrVkPrimaryCommandBuffer* fPrimaryCommandBuffer;

    // Array of available secondary command buffers that are not in flight
    SkSTArray<4, GrVkSecondaryCommandBuffer*, true> fAvailableSecondaryBuffers;
};

#endif
