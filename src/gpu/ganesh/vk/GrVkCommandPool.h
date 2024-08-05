/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCommandPool_DEFINED
#define GrVkCommandPool_DEFINED

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include <stdint.h>
#include <cinttypes>
#include <memory>

class GrVkGpu;

class GrVkCommandPool : public GrVkManagedResource {
public:
    static GrVkCommandPool* Create(GrVkGpu* gpu);

    VkCommandPool vkCommandPool() const {
        return fCommandPool;
    }

    void reset(GrVkGpu* gpu);


    GrVkPrimaryCommandBuffer* getPrimaryCommandBuffer() { return fPrimaryCommandBuffer.get(); }

    std::unique_ptr<GrVkSecondaryCommandBuffer> findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

    void recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* buffer);

    // marks that we are finished with this command pool; it is not legal to continue creating or
    // writing to command buffers in a closed pool
    void close();

    // returns true if close() has not been called
    bool isOpen() const { return fOpen; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %" PRIdPTR " (%d refs)\n",
                 (intptr_t)fCommandPool, this->getRefCnt());
    }
#endif

private:
    GrVkCommandPool() = delete;

    GrVkCommandPool(GrVkGpu* gpu, VkCommandPool commandPool, GrVkPrimaryCommandBuffer*);

    void releaseResources();

    void freeGPUData() const override;

    bool fOpen = true;

    VkCommandPool fCommandPool;

    std::unique_ptr<GrVkPrimaryCommandBuffer> fPrimaryCommandBuffer;

    // Array of available secondary command buffers that are not in flight
    skia_private::STArray<4,
        std::unique_ptr<GrVkSecondaryCommandBuffer>, true> fAvailableSecondaryBuffers;
    int fMaxCachedSecondaryCommandBuffers;
};

#endif
