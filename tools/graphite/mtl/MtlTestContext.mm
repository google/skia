/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/mtl/GraphiteMtlTestContext.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"

#ifdef SK_METAL

#import <Metal/Metal.h>

namespace skiatest::graphite::mtl {

std::unique_ptr<GraphiteTestContext> TestContext::Make() {
    sk_cfp<id<MTLDevice>> device;
#ifdef SK_BUILD_FOR_MAC
    sk_cfp<NSArray<id <MTLDevice>>*> availableDevices(MTLCopyAllDevices());
    // Choose the non-integrated CPU if available
    for (id<MTLDevice> dev in availableDevices.get()) {
        if (!dev.isLowPower) {
            // This retain is necessary because when the NSArray goes away it will delete the
            // device entry otherwise.
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

    skgpu::mtl::BackendContext backendContext = {};
    backendContext.fDevice.retain(device.get());
    backendContext.fQueue.reset([*device newCommandQueue]);

    return std::unique_ptr<GraphiteTestContext>(new TestContext(backendContext));
}

sk_sp<skgpu::Context> TestContext::makeContext() {
    return skgpu::Context::MakeMetal(fMtl);
}

}  // namespace skiatest::graphite::mtl

#endif // SK_METAL
