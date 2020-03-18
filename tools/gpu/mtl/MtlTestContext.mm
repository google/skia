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
#ifdef SK_BUILD_FOR_MAC
            NSArray<id <MTLDevice>>* availableDevices = MTLCopyAllDevices();
            // Choose the non-integrated CPU if available
            for (id<MTLDevice> dev in availableDevices) {
                if (!dev.isLowPower) {
                    device = dev;
                    break;
                }
                if (dev.isRemovable) {
                    device = dev;
                    break;
                }
            }
            if (!device) {
                device = MTLCreateSystemDefaultDevice();
            }
#else
            device = MTLCreateSystemDefaultDevice();
#endif
            queue = [device newCommandQueue];
        }

        return new MtlTestContextImpl(device, queue);
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

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
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }

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
