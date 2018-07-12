/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VkTestContext.h"

#ifdef SK_VULKAN

#include "GrContext.h"
#include "VkTestUtils.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

namespace {

#define ACQUIRE_VK_PROC(name, device)                                               \
    f##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, nullptr, device)); \
    SkASSERT(f##name);

/**
 * Implements sk_gpu_test::FenceSync for Vulkan. It creates a single command
 * buffer with USAGE_SIMULTANEOUS with no content . On every insertFence request
 * it submits the command buffer with a new fence.
 */
class VkFenceSync : public sk_gpu_test::FenceSync {
public:
    VkFenceSync(GrVkGetProc getProc, VkDevice device, VkQueue queue,
                uint32_t queueFamilyIndex)
            : fDevice(device)
            , fQueue(queue) {
        ACQUIRE_VK_PROC(CreateCommandPool, device);
        ACQUIRE_VK_PROC(DestroyCommandPool, device);
        ACQUIRE_VK_PROC(AllocateCommandBuffers, device);
        ACQUIRE_VK_PROC(FreeCommandBuffers, device);
        ACQUIRE_VK_PROC(BeginCommandBuffer, device);
        ACQUIRE_VK_PROC(EndCommandBuffer, device);
        ACQUIRE_VK_PROC(CreateFence, device);
        ACQUIRE_VK_PROC(DestroyFence, device);
        ACQUIRE_VK_PROC(WaitForFences, device);
        ACQUIRE_VK_PROC(QueueSubmit, device);

        VkResult result;
        SkDEBUGCODE(fUnfinishedSyncs = 0;)
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        result = fCreateCommandPool(fDevice, &createInfo, nullptr, &fCommandPool);
        SkASSERT(VK_SUCCESS == result);

        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.commandPool = fCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = fAllocateCommandBuffers(fDevice, &allocateInfo, &fCommandBuffer);
        SkASSERT(VK_SUCCESS == result);

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        result = fBeginCommandBuffer(fCommandBuffer, &beginInfo);
        SkASSERT(VK_SUCCESS == result);
        result = fEndCommandBuffer(fCommandBuffer);
        SkASSERT(VK_SUCCESS == result);

    }

    ~VkFenceSync() override {
        SkASSERT(!fUnfinishedSyncs);
        // If the above assertion is true then the command buffer should not be in flight.
        fFreeCommandBuffers(fDevice, fCommandPool, 1, &fCommandBuffer);
        fDestroyCommandPool(fDevice, fCommandPool, nullptr);
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        VkResult result;

        VkFence fence;
        VkFenceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        result = fCreateFence(fDevice, &info, nullptr, &fence);
        SkASSERT(VK_SUCCESS == result);

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &fCommandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
        result = fQueueSubmit(fQueue, 1, &submitInfo, fence);
        SkASSERT(VK_SUCCESS == result);

        SkDEBUGCODE(++fUnfinishedSyncs;)
        return (sk_gpu_test::PlatformFence)fence;
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        VkFence fence = (VkFence)opaqueFence;
        static constexpr uint64_t kForever = ~((uint64_t)0);
        auto result = fWaitForFences(fDevice, 1, &fence, true, kForever);
        return result != VK_TIMEOUT;
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        VkFence fence = (VkFence)opaqueFence;
        fDestroyFence(fDevice, fence, nullptr);
        SkDEBUGCODE(--fUnfinishedSyncs;)
    }

private:
    VkDevice                    fDevice;
    VkQueue                     fQueue;
    VkCommandPool               fCommandPool;
    VkCommandBuffer             fCommandBuffer;

    PFN_vkCreateCommandPool fCreateCommandPool = nullptr;
    PFN_vkDestroyCommandPool fDestroyCommandPool = nullptr;
    PFN_vkAllocateCommandBuffers fAllocateCommandBuffers = nullptr;
    PFN_vkFreeCommandBuffers fFreeCommandBuffers = nullptr;
    PFN_vkBeginCommandBuffer fBeginCommandBuffer = nullptr;
    PFN_vkEndCommandBuffer fEndCommandBuffer = nullptr;
    PFN_vkCreateFence fCreateFence = nullptr;
    PFN_vkDestroyFence fDestroyFence = nullptr;
    PFN_vkWaitForFences fWaitForFences = nullptr;
    PFN_vkQueueSubmit fQueueSubmit = nullptr;

    SkDEBUGCODE(mutable int     fUnfinishedSyncs;)
    typedef sk_gpu_test::FenceSync INHERITED;
};

GR_STATIC_ASSERT(sizeof(VkFence) <= sizeof(sk_gpu_test::PlatformFence));

// TODO: Implement swap buffers and finish
class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    static VkTestContext* Create(VkTestContext* sharedContext) {
        GrVkBackendContext backendContext;
        bool ownsContext = true;
        VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
        if (sharedContext) {
            backendContext = sharedContext->getVkBackendContext();
            // We always delete the parent context last so make sure the child does not think they
            // own the vulkan context.
            ownsContext = false;
        } else {
            PFN_vkGetInstanceProcAddr instProc;
            PFN_vkGetDeviceProcAddr devProc;
            if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
                return nullptr;
            }
            if (!sk_gpu_test::CreateVkBackendContext(instProc, devProc, &backendContext,
                                                     &debugCallback)) {
                return nullptr;
            }
        }
        return new VkTestContextImpl(backendContext, ownsContext, debugCallback);
    }

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeVulkan(fVk, options);
    }

protected:
#define ACQUIRE_VK_PROC_LOCAL(name, inst)                                          \
    PFN_vk##name grVk##name =                                                      \
        reinterpret_cast<PFN_vk##name>(fVk.fGetProc("vk" #name, inst, nullptr));   \
    if (grVk##name == nullptr) {                                                   \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);          \
        return;                                                                    \
    }

    void teardown() override {
        INHERITED::teardown();
        fVk.fMemoryAllocator.reset();
        if (fOwnsContext) {
            ACQUIRE_VK_PROC_LOCAL(DeviceWaitIdle, fVk.fInstance);
            ACQUIRE_VK_PROC_LOCAL(DestroyDevice, fVk.fInstance);
            ACQUIRE_VK_PROC_LOCAL(DestroyInstance, fVk.fInstance);
            grVkDeviceWaitIdle(fVk.fDevice);
            grVkDestroyDevice(fVk.fDevice, nullptr);
#ifdef SK_ENABLE_VK_LAYERS
            if (fDebugCallback != VK_NULL_HANDLE) {
                ACQUIRE_VK_PROC_LOCAL(DestroyDebugReportCallbackEXT, fVk.fInstance);
                grVkDestroyDebugReportCallbackEXT(fVk.fInstance, fDebugCallback, nullptr);
            }
#endif
            grVkDestroyInstance(fVk.fInstance, nullptr);
        }
    }

private:
    VkTestContextImpl(const GrVkBackendContext& backendContext, bool ownsContext,
                      VkDebugReportCallbackEXT debugCallback)
            : VkTestContext(backendContext, ownsContext, debugCallback) {
        fFenceSync.reset(new VkFenceSync(fVk.fGetProc, fVk.fDevice, fVk.fQueue,
                                         fVk.fGraphicsQueueIndex));
    }

    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }
    void onPlatformSwapBuffers() const override {}

    typedef sk_gpu_test::VkTestContext INHERITED;
};
}  // anonymous namespace

namespace sk_gpu_test {
VkTestContext* CreatePlatformVkTestContext(VkTestContext* sharedContext) {
    return VkTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
