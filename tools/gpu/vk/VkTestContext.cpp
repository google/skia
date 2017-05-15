/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VkTestContext.h"

#ifdef SK_VULKAN

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"
#include <vulkan/vulkan.h>

namespace {
/**
 * Implements sk_gpu_test::FenceSync for Vulkan. It creates a single command
 * buffer with USAGE_SIMULTANEOUS with no content . On every insertFence request
 * it submits the command buffer with a new fence.
 */
class VkFenceSync : public sk_gpu_test::FenceSync {
public:
    VkFenceSync(sk_sp<const GrVkInterface> vk, VkDevice device, VkQueue queue,
                uint32_t queueFamilyIndex)
            : fVk(std::move(vk))
            , fDevice(device)
            , fQueue(queue) {
        SkDEBUGCODE(fUnfinishedSyncs = 0;)
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        GR_VK_CALL_ERRCHECK(fVk, CreateCommandPool(fDevice, &createInfo, nullptr, &fCommandPool));

        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.commandPool = fCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        GR_VK_CALL_ERRCHECK(fVk, AllocateCommandBuffers(fDevice, &allocateInfo, &fCommandBuffer));

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        GR_VK_CALL_ERRCHECK(fVk, BeginCommandBuffer(fCommandBuffer, &beginInfo));
        GR_VK_CALL_ERRCHECK(fVk, EndCommandBuffer(fCommandBuffer));
    }

    ~VkFenceSync() override {
        SkASSERT(!fUnfinishedSyncs);
        // If the above assertion is true then the command buffer should not be in flight.
        GR_VK_CALL(fVk, FreeCommandBuffers(fDevice, fCommandPool, 1, &fCommandBuffer));
        GR_VK_CALL(fVk, DestroyCommandPool(fDevice, fCommandPool, nullptr));
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        VkFence fence;
        VkFenceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        GR_VK_CALL_ERRCHECK(fVk, CreateFence(fDevice, &info, nullptr, &fence));
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
        GR_VK_CALL_ERRCHECK(fVk, QueueSubmit(fQueue, 1, &submitInfo, fence));
        SkDEBUGCODE(++fUnfinishedSyncs;)
        return (sk_gpu_test::PlatformFence)fence;
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        VkFence fence = (VkFence)opaqueFence;
        static constexpr uint64_t kForever = ~((uint64_t)0);
        auto result = GR_VK_CALL(fVk, WaitForFences(fDevice, 1, &fence, true, kForever));
        return result != VK_TIMEOUT;
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        VkFence fence = (VkFence)opaqueFence;
        GR_VK_CALL(fVk, DestroyFence(fDevice, fence, nullptr));
        SkDEBUGCODE(--fUnfinishedSyncs;)
    }

private:
    sk_sp<const GrVkInterface>  fVk;
    VkDevice                    fDevice;
    VkQueue                     fQueue;
    VkCommandPool               fCommandPool;
    VkCommandBuffer             fCommandBuffer;
    SkDEBUGCODE(mutable int     fUnfinishedSyncs;)
    typedef sk_gpu_test::FenceSync INHERITED;
};

GR_STATIC_ASSERT(sizeof(VkFence) <= sizeof(sk_gpu_test::PlatformFence));

// TODO: Implement swap buffers and finish
class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    static VkTestContext* Create(VkTestContext* sharedContext) {
        sk_sp<const GrVkBackendContext> backendContext;
        if (sharedContext) {
            backendContext = sharedContext->getVkBackendContext();
        } else {
            backendContext.reset(GrVkBackendContext::Create(vkGetInstanceProcAddr,
                                                            vkGetDeviceProcAddr));
        }
        if (!backendContext) {
            return nullptr;
        }
        return new VkTestContextImpl(std::move(backendContext));
    }

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

protected:
    void teardown() override {
        INHERITED::teardown();
        fVk.reset(nullptr);
    }

private:
    VkTestContextImpl(sk_sp<const GrVkBackendContext> backendContext)
            : VkTestContext(std::move(backendContext)) {
        fFenceSync.reset(new VkFenceSync(fVk->fInterface, fVk->fDevice, fVk->fQueue,
                                         fVk->fGraphicsQueueIndex));
    }

    void onPlatformMakeCurrent() const override {}
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
