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

sk_sp<GrVkSemaphore> GrVkSemaphore::Make(GrVkGpu* gpu, bool isOwned) {
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

sk_sp<GrVkSemaphore> GrVkSemaphore::MakeWrapped(GrVkGpu* gpu,
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

GrVkSemaphore::GrVkSemaphore(GrVkGpu* gpu, VkSemaphore semaphore, bool prohibitSignal,
                             bool prohibitWait, bool isOwned)
        : INHERITED(gpu) {
    fResource = new Resource(semaphore, prohibitSignal, prohibitWait, isOwned);
    isOwned ? this->registerWithCache(SkBudgeted::kNo)
            : this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

void GrVkSemaphore::onRelease() {
    if (fResource) {
        fResource->unref(static_cast<GrVkGpu*>(this->getGpu()));
        fResource = nullptr;
    }
    INHERITED::onRelease();
}

void GrVkSemaphore::onAbandon() {
    if (fResource) {
        fResource->unrefAndAbandon();
        fResource = nullptr;
    }
    INHERITED::onAbandon();
}

void GrVkSemaphore::Resource::freeGPUData(GrVkGpu* gpu) const {
    if (fIsOwned) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroySemaphore(gpu->device(), fSemaphore, nullptr));
    }
}

GrBackendSemaphore GrVkSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    backendSemaphore.initVulkan(fResource->semaphore());
    return backendSemaphore;
}
