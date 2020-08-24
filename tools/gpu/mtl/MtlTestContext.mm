/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/mtl/MtlTestContext.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"

#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

namespace {
class MtlTestContextImpl : public sk_gpu_test::MtlTestContext {
public:
    static MtlTestContext* Create(MtlTestContext* sharedContext) {
        sk_cf_obj<id<MTLDevice>> device;
        sk_cf_obj<id<MTLCommandQueue>> queue;
        if (sharedContext) {
            MtlTestContextImpl* sharedContextImpl = (MtlTestContextImpl*) sharedContext;
            device.retain(sharedContextImpl->device());
            queue.retain(sharedContextImpl->queue());
        } else {
#ifdef SK_BUILD_FOR_MAC
            sk_cf_obj<NSArray<id <MTLDevice>>*> availableDevices(MTLCopyAllDevices());
            // Choose the non-integrated CPU if available
            for (id<MTLDevice> dev in availableDevices.get()) {
                if (!dev.isLowPower) {
                    device.retain(dev);
                    break;
                }
                if (dev.isRemovable) {
                    device.retain(dev);
                    break;
                }
            }
            if (!device) {
                device.reset(MTLCreateSystemDefaultDevice());
            }
#else
            device.reset(MTLCreateSystemDefaultDevice());
#endif
            queue.reset([*device newCommandQueue]);
        }

        return new MtlTestContextImpl(std::move(device), std::move(queue));
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    void finish() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContext::MakeMetal((void*)fDevice.get(),
                                          (void*)fQueue.get(),
                                          options);
    }

    id<MTLDevice> device() { return fDevice.get(); }
    id<MTLCommandQueue> queue() { return fQueue.get(); }

private:
    MtlTestContextImpl(sk_cf_obj<id<MTLDevice>> device, sk_cf_obj<id<MTLCommandQueue>> queue)
            : INHERITED(), fDevice(std::move(device)), fQueue(std::move(queue)) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }

    sk_cf_obj<id<MTLDevice>>        fDevice;
    sk_cf_obj<id<MTLCommandQueue>>  fQueue;

    typedef sk_gpu_test::MtlTestContext INHERITED;
};

}  // anonymous namespace

namespace sk_gpu_test {

MtlTestContext* CreatePlatformMtlTestContext(MtlTestContext* sharedContext) {
    return MtlTestContextImpl::Create(sharedContext);
}

}  // namespace sk_gpu_test


#endif
