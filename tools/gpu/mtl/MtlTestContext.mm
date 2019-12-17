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

namespace {
/**
 * Implements sk_gpu_test::FenceSync for Metal.
 *
 * Fences as MTLSharedEvents are not supported across all Metal platforms, so we do
 * the next best thing and submit an empty MTLCommandBuffer and track when it's complete.
 */
class MtlFenceSync : public sk_gpu_test::FenceSync {
public:
    MtlFenceSync(id<MTLCommandQueue> queue)
            : fQueue(queue) {
        SkDEBUGCODE(fUnfinishedSyncs = 0;)
    }

    ~MtlFenceSync() override {
        SkASSERT(!fUnfinishedSyncs);
    }

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override {
        id<MTLCommandBuffer> cmdBuffer = [fQueue commandBuffer];
        cmdBuffer.label = @"Fence";
        [cmdBuffer commit];

        SkDEBUGCODE(++fUnfinishedSyncs;)

        void* cfCmdBuffer = (__bridge_retained void*)cmdBuffer;
        return (sk_gpu_test::PlatformFence)cfCmdBuffer;
    }

    bool waitFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        void* cfCmdBuffer = (void*) opaqueFence;
        id<MTLCommandBuffer> cmdBuffer = (__bridge id<MTLCommandBuffer>) cfCmdBuffer;

        [cmdBuffer waitUntilCompleted];

        return (MTLCommandBufferStatusError != cmdBuffer.status);
    }

    void deleteFence(sk_gpu_test::PlatformFence opaqueFence) const override {
        CFRelease((void*) opaqueFence);
        SkDEBUGCODE(--fUnfinishedSyncs;)
    }

private:
    id<MTLCommandQueue>         fQueue;
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
            device = MTLCreateSystemDefaultDevice();
            queue = [device newCommandQueue];
        }

        return new MtlTestContextImpl(device, queue);
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to do here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeMetal((__bridge void*)fDevice,
                                    (__bridge void*)fQueue,
                                    options);
    }

    id<MTLDevice> device() { return fDevice; }
    id<MTLCommandQueue> queue() { return fQueue; }

private:
    MtlTestContextImpl(id<MTLDevice> device, id<MTLCommandQueue> queue)
            : INHERITED(), fDevice(device), fQueue(queue) {
        fFenceSync.reset(new MtlFenceSync(queue));
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
