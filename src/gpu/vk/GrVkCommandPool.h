/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "src/gpu/vk/GrVkGpuCommandBuffer.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkResource.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

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

    GrVkPrimaryCommandBuffer* getPrimaryCommandBuffer() { return fPrimaryCommandBuffer.get(); }

    std::unique_ptr<GrVkSecondaryCommandBuffer> findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

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

    std::unique_ptr<GrVkPrimaryCommandBuffer> fPrimaryCommandBuffer;

    // Array of available secondary command buffers that are not in flight
    SkSTArray<4, std::unique_ptr<GrVkSecondaryCommandBuffer>, true> fAvailableSecondaryBuffers;
};

#endif
