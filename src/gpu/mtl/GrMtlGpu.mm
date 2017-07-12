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

GrGpu* GrMtlGpu::Create(GrBackendContext backendContext, const GrContextOptions& options,
                        GrContext* context) {
    return nullptr;
}

