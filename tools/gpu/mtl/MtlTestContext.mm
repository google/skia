/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/mtl/MtlTestContext.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

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
            sk_cfp<id<MTLDevice>> device;
#ifdef SK_BUILD_FOR_MAC
            sk_cfp<NSArray<id <MTLDevice>>*> availableDevices(MTLCopyAllDevices());
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
            backendContext.fDevice.retain((GrMTLHandle)device.get());
            sk_cfp<id<MTLCommandQueue>> queue([*device newCommandQueue]);
            backendContext.fQueue.retain((GrMTLHandle)queue.get());
        }

        return new MtlTestContextImpl(backendContext);
    }

    ~MtlTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContexts::MakeMetal(fMtl, options);
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
