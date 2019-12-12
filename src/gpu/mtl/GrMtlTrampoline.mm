/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/GrLegacyDirectContext.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/effects/GrSkSLFP.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue, const GrContextOptions& options) {
    sk_sp<GrContext> context = GrMetalMakeLegacyDirectContext(options);
    SkASSERT(context);
    context->fGpu = GrMtlGpu::Make(context.get(),
                                   options,
                                   (__bridge id<MTLDevice>)device,
                                   (__bridge id<MTLCommandQueue>)queue);
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps(), nullptr)) {
        return nullptr;
    }
    return context;
}
