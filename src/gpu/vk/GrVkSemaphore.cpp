/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkSemaphore.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

std::unique_ptr<GrVkSemaphore> GrVkSemaphore::Make(GrVkGpu* gpu, bool isOwned) {
    VkSemaphoreCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkSemaphoreCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateSemaphore(gpu->device(), &createInfo, nullptr,
                                                   &semaphore));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return std::unique_ptr<GrVkSemaphore>(new GrVkSemaphore(gpu, semaphore, false, false, isOwned));
}

std::unique_ptr<GrVkSemaphore> GrVkSemaphore::MakeWrapped(GrVkGpu* gpu,
                                                          VkSemaphore semaphore,
                                                          WrapType wrapType,
                                                          GrWrapOwnership ownership) {
    if (VK_NULL_HANDLE == semaphore) {
        return nullptr;
    }
    bool prohibitSignal = WrapType::kWillWait == wrapType;
    bool prohibitWait = WrapType::kWillSignal == wrapType;
    return std::unique_ptr<GrVkSemaphore>(new GrVkSemaphore(gpu, semaphore, prohibitSignal,
                                                            prohibitWait,
                                                            kBorrow_GrWrapOwnership != ownership));
}

GrVkSemaphore::GrVkSemaphore(GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal,
                             bool prohibitWait, bool isOwned) {
    fResource = new Resource(gpu, semaphore, prohibitSignal, prohibitWait, isOwned);
}

GrVkSemaphore::~GrVkSemaphore() {
    if (fResource) {
        fResource->unref();
    }
}

void GrVkSemaphore::Resource::freeGPUData() const {
    if (fIsOwned) {
        GR_VK_CALL(fGpu->vkInterface(),
                   DestroySemaphore(fGpu->device(), fSemaphore, nullptr));
    }
}

GrBackendSemaphore GrVkSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    backendSemaphore.initVulkan(fResource->semaphore());
    return backendSemaphore;
}
