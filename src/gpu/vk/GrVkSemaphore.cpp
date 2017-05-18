/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkSemaphore.h"

#include "GrVkGpu.h"
#include "GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

sk_sp<GrVkSemaphore> GrVkSemaphore::Make(const GrVkGpu* gpu) {
    VkSemaphoreCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkFenceCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(),
                        CreateSemaphore(gpu->device(), &createInfo, nullptr, &semaphore));

    return sk_sp<GrVkSemaphore>(new GrVkSemaphore(gpu, semaphore));
}

GrVkSemaphore::GrVkSemaphore(const GrVkGpu* gpu, VkSemaphore semaphore) : INHERITED(gpu) {
    fResource = new Resource(semaphore);
}

GrVkSemaphore::~GrVkSemaphore() {
    if (fGpu) {
        fResource->unref(static_cast<const GrVkGpu*>(fGpu));
    } else {
        fResource->unrefAndAbandon();
    }
}

void GrVkSemaphore::Resource::freeGPUData(const GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(),
               DestroySemaphore(gpu->device(), fSemaphore, nullptr));
}

