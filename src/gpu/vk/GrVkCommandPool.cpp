/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkCommandPool.h"

#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    freeGPUData(gpu);
    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // sType
        nullptr,                                         // pNext
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // CmdPoolCreateFlags
        gpu->queueIndex(),                               // queueFamilyIndex
    };
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateCommandPool(gpu->device(), &cmdPoolInfo,
                                                              nullptr, &fCmdPool));
}

void GrVkCommandPool::freeGPUData(GrVkGpu* gpu) const {
    SkASSERT(fCmdPool != VK_NULL_HANDLE);
    if (fCmdPool != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyCommandPool(gpu->device(), fCmdPool, nullptr));
        fCmdPool = VK_NULL_HANDLE;
    }
}
