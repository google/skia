/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkFence.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

sk_sp<GrVkFence> GrVkFence::Make(GrVkGpu* gpu, bool isSignaled) {
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (isSignaled) fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkFence fence = VK_NULL_HANDLE;
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateFence(gpu->device(), &fenceInfo, nullptr, &fence));
    if (err != VK_SUCCESS) {
        return nullptr;
    }

    return sk_sp<GrVkFence>(new GrVkFence(gpu, fence));
}

GrVkFence::GrVkFence(GrVkGpu* gpu, VkFence fence) : fGpu(gpu), fFence(fence) {}

GrVkFence::~GrVkFence() {
    GR_VK_CALL(fGpu->vkInterface(), DestroyFence(fGpu->device(), fFence, nullptr));
}

bool GrVkFence::isSignaled() {
    VkResult err;
    GR_VK_CALL_RESULT_NOCHECK(fGpu, err, GetFenceStatus(fGpu->device(), fFence));
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

void GrVkFence::wait() {
    GR_VK_CALL_ERRCHECK(fGpu, WaitForFences(fGpu->device(), 1, &fFence, true, UINT64_MAX));
}

void GrVkFence::reset() {
    VkResult err;
    // This cannot return DEVICE_LOST so we assert we succeeded.
    GR_VK_CALL_RESULT(fGpu, err, ResetFences(fGpu->device(), 1, &fFence));
    SkASSERT(err == VK_SUCCESS);
}
