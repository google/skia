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

class GrVkGpu;

class GrVkCommandPool : public GrVkResource {
public:
    GrVkCommandPool() = default;

    GrVkCommandPool(VkCommandPool cmdPool)
        : fCmdPool(cmdPool) {}

    ~GrVkCommandPool() override {
        SkASSERT(fBufferCount == 0);
        SkASSERT(fCmdPool == VK_NULL_HANDLE);
    }

    VkCommandPool vkCommandPool() const {
        return fCmdPool;
    }

    void reset(const GrVkGpu* gpu) {
        freeGPUData(gpu);
        const VkCommandPoolCreateInfo cmdPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // sType
            nullptr,                                         // pNext
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // CmdPoolCreateFlags
            gpu->backendContext().fGraphicsQueueIndex,              // queueFamilyIndex
        };
        GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateCommandPool(gpu->device(), &cmdPoolInfo,
                                                                  nullptr, &fCmdPool));
    }

    void bufferCreated() {
        ++fBufferCount;
    }

    void bufferDestroyed() {
        --fBufferCount;
    }

    int bufferCount() {
        return fBufferCount;
    }

    void freeGPUData(const GrVkGpu* gpu) const override {
        SkASSERT(!fBufferCount);
        if (fCmdPool != VK_NULL_HANDLE) {
            GR_VK_CALL(gpu->vkInterface(),
                       DestroyCommandPool(gpu->device(), fCmdPool, nullptr));
            fCmdPool = VK_NULL_HANDLE;
        }
    }

#ifdef SK_DEBUG
    void dumpInfo() const override {}
#endif

private:
    int fBufferCount = 0;

    mutable VkCommandPool fCmdPool;
};

#endif
