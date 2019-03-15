/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MtlTestContext.h"

#include "GrContext.h"
#include "GrContextOptions.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

// Helper macros for autorelease pools
#define SK_BEGIN_AUTORELEASE_BLOCK @autoreleasepool {
#define SK_END_AUTORELEASE_BLOCK }

namespace {
/**
 * Implements sk_gpu_test::FenceSync for Metal.
 */

// TODO
#if 0
class MtlFenceSync : public sk_gpu_test::FenceSync {
public:
    MtlFenceSync(sk_sp<const GrVkInterface> vk, VkDevice device, VkQueue queue,
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
#endif

class MtlTestContextImpl : public sk_gpu_test::MtlTestContext {
public:
    static MtlTestContext* Create(MtlTestContext* sharedContext) {
        id<MTLDevice> device;
        id<MTLCommandQueue> queue;
        if (sharedContext) {
            MtlTestContextImpl* sharedContextImpl = (MtlTestContextImpl*) sharedContext;
            device = sharedContextImpl->device();
            queue = sharedContextImpl->queue();
        } else {
            SK_BEGIN_AUTORELEASE_BLOCK
            device = MTLCreateSystemDefaultDevice();
            queue = [device newCommandQueue];
            SK_END_AUTORELEASE_BLOCK
        }

        return new MtlTestContextImpl(device, queue);
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeMetal((__bridge_retained void*)fDevice,
                                    (__bridge_retained void*)fQueue,
                                    options);
    }

    id<MTLDevice> device() { return fDevice; }
    id<MTLCommandQueue> queue() { return fQueue; }

private:
    MtlTestContextImpl(id<MTLDevice> device, id<MTLCommandQueue> queue)
            : INHERITED(), fDevice(device), fQueue(queue) {
        fFenceSync.reset(nullptr);
    }

    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }
    void onPlatformSwapBuffers() const override {}

    id<MTLDevice>        fDevice;
    id<MTLCommandQueue>  fQueue;

    typedef sk_gpu_test::MtlTestContext INHERITED;
};

}  // anonymous namespace

namespace sk_gpu_test {

MtlTestContext* CreatePlatformMtlTestContext(MtlTestContext* sharedContext) {
    return MtlTestContextImpl::Create(sharedContext);
}

}  // namespace sk_gpu_test


#endif
