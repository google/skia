/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "metal_context_helper.h"

#import "include/gpu/ganesh/mtl/GrMtlTypes.h"
#import "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

GrMtlBackendContext GetMetalContext() {
    GrMtlBackendContext backendContext = {};
    sk_cfp<id<MTLDevice>> device;
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    device.reset(MTLCreateSystemDefaultDevice());
#else
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
#endif

    backendContext.fDevice.retain((GrMTLHandle)device.get());
    sk_cfp<id<MTLCommandQueue>> queue([*device newCommandQueue]);
    backendContext.fQueue.retain((GrMTLHandle)queue.get());
    return backendContext;
}
