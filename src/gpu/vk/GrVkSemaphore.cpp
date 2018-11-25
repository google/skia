/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkSemaphore.h"

#include "GrBackendSemaphore.h"
#include "GrVkGpu.h"
#include "GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

SkMutex GrVkSemaphore::Resource::gMutex;

sk_sp<GrVkSemaphore> GrVkSemaphore::Make(const GrVkGpu* gpu, bool isOwned) {
    VkSemaphoreCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkSemaphoreCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(),
                        CreateSemaphore(gpu->device(), &createInfo, nullptr, &semaphore));

    return sk_sp<GrVkSemaphore>(new GrVkSemaphore(gpu, semaphore, false, false, isOwned));
}

sk_sp<GrVkSemaphore> GrVkSemaphore::MakeWrapped(const GrVkGpu* gpu,
                                                VkSemaphore semaphore,
                                                WrapType wrapType,
                                                GrWrapOwnership ownership) {
    if (VK_NULL_HANDLE == semaphore) {
        return nullptr;
    }
    bool prohibitSignal = WrapType::kWillWait == wrapType;
    bool prohibitWait = WrapType::kWillSignal == wrapType;
    return sk_sp<GrVkSemaphore>(new GrVkSemaphore(gpu, semaphore, prohibitSignal, prohibitWait,
                                                  kBorrow_GrWrapOwnership != ownership));
}

GrVkSemaphore::GrVkSemaphore(const GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal,
                             bool prohibitWait, bool isOwned)
        : INHERITED(gpu) {
    fResource = new Resource(semaphore, prohibitSignal, prohibitWait, isOwned);
}

GrVkSemaphore::~GrVkSemaphore() {
    if (fGpu) {
        fResource->unref(static_cast<const GrVkGpu*>(fGpu));
    } else {
        fResource->unrefAndAbandon();
    }
}

void GrVkSemaphore::Resource::freeGPUData(const GrVkGpu* gpu) const {
    if (fIsOwned) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroySemaphore(gpu->device(), fSemaphore, nullptr));
    }
}

void GrVkSemaphore::setBackendSemaphore(GrBackendSemaphore* backendSemaphore) const {
    backendSemaphore->initVulkan(fResource->semaphore());
}

