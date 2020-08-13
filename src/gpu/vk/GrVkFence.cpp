/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkFence.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

// static
GrVkFence* GrVkFence::CreateResource(GrVkGpu* gpu) {
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence = VK_NULL_HANDLE;
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateFence(gpu->device(), &fenceInfo, nullptr, &fence));
    if (err != VK_SUCCESS) {
        return nullptr;
    }

    return new GrVkFence(gpu, fence);
}

bool GrVkFence::isSignaled(GrVkGpu* gpu) {
    VkResult err;
    GR_VK_CALL_RESULT_NOCHECK(gpu, err, GetFenceStatus(gpu->device(), fFence));
    switch (err) {
        case VK_SUCCESS:
        case VK_ERROR_DEVICE_LOST:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SkDebugf("Error getting fence status: %d\n", err);
            SK_ABORT("Got an invalid fence status");
            return false;
    }
}

void GrVkFence::wait(GrVkGpu* gpu) {
    GR_VK_CALL_ERRCHECK(gpu, WaitForFences(gpu->device(), 1, &fFence, true, UINT64_MAX));
}

void GrVkFence::reset(GrVkGpu* gpu) {
    VkResult err;
    // This cannot return DEVICE_LOST so we assert we succeeded.
    GR_VK_CALL_RESULT(gpu, err, ResetFences(gpu->device(), 1, &fFence));
    SkASSERT(err == VK_SUCCESS);
}

void GrVkFence::freeGPUData() const {
    SkASSERT(fFence);
    GR_VK_CALL(fGpu->vkInterface(), DestroyFence(fGpu->device(), fFence, nullptr));
}

void GrVkFence::onRecycle() const {
    fGpu->resourceProvider().recycleFence(const_cast<GrVkFence*>(this));
}
