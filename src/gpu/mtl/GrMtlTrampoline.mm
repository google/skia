/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTrampoline.h"

#include "GrMtlGpu.h"

GrGpu* GrMtlTrampoline::CreateGpu(GrContext* context,
                                  const GrContextOptions& options,
                                  void* device,
                                  void* queue) {
    return GrMtlGpu::Create(context,
                            options,
                            (__bridge_transfer id<MTLDevice>)device,
                            (__bridge_transfer id<MTLCommandQueue>)queue);
}

