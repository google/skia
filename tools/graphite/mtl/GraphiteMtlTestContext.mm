/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/mtl/GraphiteMtlTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/TestOptions.h"

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

skgpu::ContextType MtlTestContext::contextType() {
    return skgpu::ContextType::kMetal;
}

std::unique_ptr<skgpu::graphite::Context> MtlTestContext::makeContext(const TestOptions& options) {
    SkASSERT(!options.fNeverYieldToWebGPU);
    skgpu::graphite::ContextOptions revisedContextOptions(options.fContextOptions);
    skgpu::graphite::ContextOptionsPriv contextOptionsPriv;
    if (!options.fContextOptions.fOptionsPriv) {
        revisedContextOptions.fOptionsPriv = &contextOptionsPriv;
    }
    // Needed to make synchronous readPixels work
    revisedContextOptions.fOptionsPriv->fStoreContextRefInRecorder = true;

    return skgpu::graphite::ContextFactory::MakeMetal(fMtl, revisedContextOptions);
}

}  // namespace skiatest::graphite
