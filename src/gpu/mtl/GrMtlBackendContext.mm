/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "mtl/GrMtlBackendContext.h"

#import <Metal/Metal.h>

GrMtlBackendContext* GrMtlBackendContext::CreateDefault() {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> queue = [device newCommandQueue];
    
    GrMtlBackendContext* ctx = new GrMtlBackendContext();
    ctx->fDevice = (intptr_t)device;
    ctx->fQueue = (intptr_t)queue;
 
    return ctx;
}
