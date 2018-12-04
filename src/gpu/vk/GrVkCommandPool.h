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

    VkCommandPool vkCommandPool() const {
        return fCommandPool;
    }

    void reset(GrVkGpu* gpu);

    void releaseResources(GrVkGpu* gpu);

    void checkCommandBuffers(GrVkResourceProvider* provider, GrVkGpu* gpu);

    GrVkGpuRTCommandBuffer* getCommandBuffer(GrVkGpu*,
                                             GrRenderTarget*,
                                             GrSurfaceOrigin,
                                             const SkRect&,
                                             const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                             const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&);

    GrVkGpuRTCommandBuffer* getCachedRTCommandBuffer();

    GrVkGpuTextureCommandBuffer* getCommandBuffer(GrVkGpu*, GrTexture*, GrSurfaceOrigin);

    GrVkGpuTextureCommandBuffer* getCachedTextureCommandBuffer();

    // marks that we are finished with this command pool; it is not legal to continue creating or
    // writing to command buffers in a closed pool
    void close();

    // returns true if close() has not been called
    bool isOpen() const { return fOpen; }

    // returns true if this pool is closed and all of the buffers have finished rendering
    bool isFinished() const;

    std::recursive_mutex& mutex() { return fMutex; }

#ifdef SK_DEBUG
    void dumpInfo() const override {
        SkDebugf("GrVkCommandPool: %p (%d refs)\n", fCommandPool, this->getRefCnt());
    }
#endif

private:
    GrVkPrimaryCommandBuffer* findOrCreatePrimaryCommandBuffer(GrVkGpu* gpu);

    GrVkSecondaryCommandBuffer* findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu);

    void recyclePrimaryCommandBuffer(GrVkGpu* gpu, GrVkPrimaryCommandBuffer* buffer);

    void recycleSecondaryCommandBuffer(GrVkGpu* gpu, GrVkSecondaryCommandBuffer* buffer);

    void abandonGPUData() const override;

    void freeGPUData(GrVkGpu* gpu) const override;

    bool fOpen = true;

    VkCommandPool fCommandPool;

    // Array of PrimaryCommandBuffers that are currently in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true>   fActivePrimaryBuffers;

    // Array of available primary command buffers that are not in flight
    SkSTArray<4, GrVkPrimaryCommandBuffer*, true>   fAvailablePrimaryBuffers;

    // Array of available secondary command buffers that are not in flight
    SkSTArray<4, GrVkSecondaryCommandBuffer*, true> fAvailableSecondaryBuffers;

    std::unique_ptr<GrVkGpuRTCommandBuffer>         fCachedRTCommandBuffer;

    std::unique_ptr<GrVkGpuTextureCommandBuffer>    fCachedTexCommandBuffer;

    std::recursive_mutex fMutex;

    friend class GrVkResourceProvider;
};

#endif
