/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/mtl/MtlTestContext.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

// Helper macros for autorelease pools
#define SK_BEGIN_AUTORELEASE_BLOCK @autoreleasepool {
#define SK_END_AUTORELEASE_BLOCK }

namespace {
/**
 * Implements sk_gpu_test::FenceSync for Metal.
 */

class MtlFenceSync : public sk_gpu_test::FenceSync {
public:
    MtlFenceSync(id<MTLDevice> device)
            : fDevice(device)
            , fQueue(queue)
            , fLatestEvent(0) {
        SkDEBUGCODE(fUnfinishedSyncs = 0;)
        SK_BEGIN_AUTORELEASE_BLOCK
        fSharedEvent = [fDevice newSharedEvent];
        SK_END_AUTORELEASE_BLOCK
    }

    ~VkFenceSync() override {
        SkASSERT(!fUnfinishedSyncs);
        // If the above assertion is true then the command buffer should not be in flight.
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        SK_BEGIN_AUTORELEASE_BLOCK
        fCommandBuffer = [fQueue commandBuffer];
        SK_END_AUTORELEASE_BLOCK
        ++fLatestEvent;
        [fCommandBuffer encodeSignalEvent:fSharedEvent value:fLatestEvent];
        [fCommandBuffer submit];
        fCommandBuffer = nil;

        return (sk_gpu_test::PlatformFence)fLatestEvent;
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        uint64_t value = (uint64_t)opaqueFence;
        [fSharedEvent notifyListener:_sharedEventListener
                             atValue:2
                               block:^(id<MTLSharedEvent> sharedEvent, uint64_t value) {
                                   /* Do CPU work */
                                   sharedEvent.signaledValue = 3;
                               }];

    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        // TODO: really need to delete anything?
        SkDEBUGCODE(--fUnfinishedSyncs;)
    }

private:
    id<MTLDevice>               fDevice;
    id<MTLCommandQueue>         fQueue;
    id<MTLCommandBuffer>        fCommandBuffer;
    id<MTLSharedEvent>          fSharedEvent;
    mutable uint64_t            fLatestEvent;
    SkDEBUGCODE(mutable int     fUnfinishedSyncs;)
    typedef sk_gpu_test::FenceSync INHERITED;
};

GR_STATIC_ASSERT(sizeof(uint64_t) <= sizeof(sk_gpu_test::PlatformFence));

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
