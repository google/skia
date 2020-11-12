/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/mtl/MtlTestContext.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"

#include "src/gpu/mtl/GrMtlUtil.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

namespace {
class MtlTestContextImpl : public sk_gpu_test::MtlTestContext {
public:
    static MtlTestContext* Create(MtlTestContext* sharedContext) {
        GrMtlBackendContext backendContext = {};
        if (sharedContext) {
            MtlTestContextImpl* sharedContextImpl = (MtlTestContextImpl*) sharedContext;
            backendContext = sharedContextImpl->getMtlBackendContext();
        } else {
            id<MTLDevice> device;
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
            backendContext.fDevice.retain((__bridge GrMTLHandle)device);
            id<MTLCommandQueue> queue = [device newCommandQueue];
            backendContext.fQueue.retain((__bridge GrMTLHandle)queue);
        }

        return new MtlTestContextImpl(backendContext);
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    void finish() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContext::MakeMetal(fMtl, options);
    }

private:
    MtlTestContextImpl(const GrMtlBackendContext& mtl)
            : INHERITED(mtl) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override { return nullptr; }

    using INHERITED = sk_gpu_test::MtlTestContext;
};

}  // anonymous namespace

namespace sk_gpu_test {

MtlTestContext* CreatePlatformMtlTestContext(MtlTestContext* sharedContext) {
    return MtlTestContextImpl::Create(sharedContext);
}

}  // namespace sk_gpu_test


#endif
