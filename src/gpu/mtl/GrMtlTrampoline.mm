/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlTrampoline.h"

#include "src/gpu/mtl/GrMtlGpu.h"

sk_sp<GrGpu> GrMtlTrampoline::MakeGpu(GrDirectContext* direct,
                                      const GrContextOptions& options,
                                      void* device,
                                      void* queue) {
    GrMtlBackendContext context;
    // For now we retain these to make sure we don't remove the client's ref when we go out of
    // scope. In the future the client will set this struct up.
    context.fDevice.retain((id<MTLDevice>)device);
    context.fQueue.retain((id<MTLCommandQueue>)queue);
    return GrMtlGpu::Make(direct, options, context);
}

