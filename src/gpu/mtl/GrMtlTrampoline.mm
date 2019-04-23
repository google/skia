/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlTrampoline.h"

#include "src/gpu/mtl/GrMtlGpu.h"

sk_sp<GrGpu> GrMtlTrampoline::MakeGpu(GrContext* context,
                                      const GrContextOptions& options,
                                      void* device,
                                      void* queue) {
    return GrMtlGpu::Make(context,
                          options,
                          (__bridge_transfer id<MTLDevice>)device,
                          (__bridge_transfer id<MTLCommandQueue>)queue);
}

