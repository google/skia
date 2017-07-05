/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTrampoline.h"

#include "GrMtlGpu.h"

GrGpu* GrMtlTrampoline::CreateGpu(GrBackendContext backendContext,
                                  const GrContextOptions& options,
                                  GrContext* context) {
    return GrMtlGpu::Create(backendContext, options, context);
}

