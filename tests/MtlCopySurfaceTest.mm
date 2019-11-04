/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "tests/Test.h"

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

#include "src/gpu/mtl/GrMtlCaps.h"

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlCopySurfaceTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
//    const GrMtlCaps* mtlCaps = static_cast<const GrMtlCaps*>(context->priv().caps());

    // This is a bit weird, but it's the only way to get a framebufferOnly surface
    GrMtlGpu* gpu = (GrMtlGpu*) context->priv().getGpu();

    MTKView* view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 1024, 768)
                                            device:gpu->device()];
    id<CAMetalDrawable> drawable = [view currentDrawable];
    SkASSERT(drawable.texture.framebufferOnly);
}
