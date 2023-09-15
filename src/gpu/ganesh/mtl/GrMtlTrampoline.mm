/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlTrampoline.h"

#include "src/gpu/ganesh/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

std::unique_ptr<GrGpu> GrMtlTrampoline::MakeGpu(const GrMtlBackendContext& backendContext,
                                                const GrContextOptions& options,
                                                GrDirectContext* direct) {
    return GrMtlGpu::Make(backendContext, options, direct);
}

GR_NORETAIN_END
