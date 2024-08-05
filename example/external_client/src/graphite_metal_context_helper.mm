/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "graphite_metal_context_helper.h"

#import "include/gpu/ganesh/mtl/GrMtlTypes.h"
#import "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

skgpu::graphite::MtlBackendContext GetMetalContext() {
    skgpu::graphite::MtlBackendContext backendContext = {};
    sk_cfp<id<MTLDevice>> device;
    device.reset(MTLCreateSystemDefaultDevice());

    sk_cfp<id<MTLCommandQueue>> queue;
    queue.reset([*device newCommandQueue]);

    backendContext.fDevice.retain((CFTypeRef)device.get());
    backendContext.fQueue.retain((CFTypeRef)queue.get());
    return backendContext;
}
