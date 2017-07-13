/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrGpu* GrMtlGpu::Create(GrContext* context, const GrContextOptions& options,
                        id<MTLDevice> device, id<MTLCommandQueue> queue) {
    if (!device || !queue) {
        return nullptr;
    }
    return new GrMtlGpu(context, options, device, queue);
}

GrMtlGpu::GrMtlGpu(GrContext* context, const GrContextOptions& options,
                   id<MTLDevice> device, id<MTLCommandQueue> queue)
        : INHERITED(context)
        , fDevice(device)
        , fQueue(queue) {
    MTLTextureDescriptor* txDesc = [[MTLTextureDescriptor alloc] init];
    txDesc.textureType = MTLTextureType3D;
    txDesc.height = 64;
    txDesc.width = 64;
    txDesc.depth = 64;
    txDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    txDesc.arrayLength = 1;
    txDesc.mipmapLevelCount = 1;
    id<MTLTexture> testTexture = [fDevice newTextureWithDescriptor:txDesc];
    // To get ride of unused var warning
    int width = [testTexture width];
    SkDebugf("width: %d\n", width);
    // Unused queue warning fix
    SkDebugf("ptr to queue: %p\n", fQueue);
}
