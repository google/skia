/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/mtl/MtlTestContext.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"

#include "src/gpu/mtl/GrMtlUtil.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

// Helper macros for autorelease pools
#define SK_BEGIN_AUTORELEASE_BLOCK @autoreleasepool {
#define SK_END_AUTORELEASE_BLOCK }

namespace {
#if GR_METAL_SDK_VERSION >= 200
/**
 * Implements sk_gpu_test::FenceSync for Metal.
 *
 * Uses a single MTLSharedEvent, and inserts a GPU command to increment the value
 * each time we call insertFence(). On the CPU side we use a MTLSharedEventListener to
 * wait for the new value to be signaled on the GPU. Since the event listener is handled
 * on a separate thread, we communicate completion to the main thread via a semaphore.
 */
class MtlFenceSync : public sk_gpu_test::FenceSync {
public:
    MtlFenceSync(id<MTLDevice> device, id<MTLCommandQueue> queue)
            : fDevice(device)
            , fQueue(queue)
            , fLatestEvent(0) {
        SkDEBUGCODE(fUnfinishedSyncs = 0;)
        fSharedEvent = [fDevice newSharedEvent];
        dispatch_queue_t dispatchQueue = dispatch_queue_create("MTLFenceSync", NULL);
        fSharedEventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:dispatchQueue];
    }

    ~MtlFenceSync() override {
        SkASSERT(!fUnfinishedSyncs);
        // If the above assertion is true then the command buffer should not be in flight.
        // ARC should take care of these:
        fSharedEventListener = nil;
        fSharedEvent = nil;
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
        ++fLatestEvent;
        [cmdBuffer encodeSignalEvent:fSharedEvent value:fLatestEvent];
        [cmdBuffer commit];

        SkDEBUGCODE(++fUnfinishedSyncs;)
        return (sk_gpu_test::PlatformFence)fLatestEvent;
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        uint64_t value = (uint64_t)opaqueFence;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

        // Add listener for this particular value or greater
        __block dispatch_semaphore_t block_sema = semaphore;
        [fSharedEvent notifyListener: fSharedEventListener
                             atValue: value
                               block: ^(id<MTLSharedEvent> sharedEvent, uint64_t value) {
                                   dispatch_semaphore_signal(block_sema);
                               }];

        long result = dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

        return !result;
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        // Nothing to delete
        SkDEBUGCODE(--fUnfinishedSyncs;)
    }

private:
    id<MTLDevice>               fDevice;
    id<MTLCommandQueue>         fQueue;
    id<MTLSharedEvent>          fSharedEvent;
    MTLSharedEventListener*     fSharedEventListener;
    mutable uint64_t            fLatestEvent;
    SkDEBUGCODE(mutable int     fUnfinishedSyncs;)
    typedef sk_gpu_test::FenceSync INHERITED;
};

GR_STATIC_ASSERT(sizeof(uint64_t) <= sizeof(sk_gpu_test::PlatformFence));
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
#if GR_METAL_SDK_VERSION >= 200
        // TODO: I believe we can just check whether creating a MTLSharedEvent returns nil,
        // but this needs to be tested on an old OS.
        NSOperatingSystemVersion osVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
#ifdef SK_BUILD_FOR_MAC
        bool supportsFenceSync = (osVersion.majorVersion > 10 ||
                                  (osVersion.majorVersion == 10 && osVersion.minorVersion >= 14));
#else
        bool supportsFenceSync = (osVersion.majorVersion >= 12);
#endif
        if (supportsFenceSync ) {
            fFenceSync.reset(new MtlFenceSync(device, queue));
        } else {
            fFenceSync.reset(nullptr);
        }
#else
        fFenceSync.reset(nullptr);
#endif
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
