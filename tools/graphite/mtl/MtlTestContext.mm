/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/mtl/GraphiteMtlTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/mtl/MtlTypes.h"
#include "include/gpu/graphite/mtl/MtlUtils.h"

#import <Metal/Metal.h>

namespace skiatest::graphite {

std::unique_ptr<GraphiteTestContext> MtlTestContext::Make() {
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

    skgpu::graphite::MtlBackendContext backendContext = {};
    backendContext.fDevice.retain(device.get());
    backendContext.fQueue.reset([*device newCommandQueue]);

    return std::unique_ptr<GraphiteTestContext>(new MtlTestContext(backendContext));
}

std::unique_ptr<skgpu::graphite::Context> MtlTestContext::makeContext() {
    skgpu::graphite::ContextOptions contextOptions;
    contextOptions.fStoreContextRefInRecorder = true;
    return skgpu::graphite::ContextFactory::MakeMetal(fMtl, contextOptions);
}

}  // namespace skiatest::graphite

