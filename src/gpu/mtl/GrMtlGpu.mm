/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlGpu.h"

#include "mtl/GrMtlBackendContext.h"

GrGpu* GrMtlGpu::Create(GrBackendContext backendContext, const GrContextOptions& options,
                        GrContext* context) {
    const GrMtlBackendContext* mtlBackendContext =
            reinterpret_cast<const GrMtlBackendContext*>(backendContext);
    
    if (!mtlBackendContext) {
        return nullptr;
    } else {
        mtlBackendContext->ref();
    }
    
    return new GrMtlGpu(context, options, mtlBackendContext);
}


GrMtlGpu::GrMtlGpu(GrContext* context, const GrContextOptions& options,
                   const GrMtlBackendContext* backendCtx)
        : INHERITED(context)
        , fDevice((id<MTLDevice>)backendCtx->fDevice)
        , fQueue((id<MTLCommandQueue>)backendCtx->fQueue) {
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
