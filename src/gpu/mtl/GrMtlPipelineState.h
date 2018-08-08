/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineState_DEFINED
#define GrMtlPipelineState_DEFINED

#include "GrMtlBuffer.h"

#import <metal/metal.h>

class GrMtlGpu;

/**
 * Wraps a MTLRenderPipelineState object and also contains more info about the pipeline as needed
 * by Ganesh
 */
class GrMtlPipelineState {
public:
    GrMtlPipelineState(GrMtlGpu* gpu,
                       id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat,
                       GrMtlBuffer* geometryUniformBuffer,
                       GrMtlBuffer* fragmentUniformBuffer);

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState; }

private:
    GrMtlGpu* fGpu;

    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat             fPixelFormat;
    sk_sp<GrMtlBuffer>         fGeometryUniformBuffer;
    sk_sp<GrMtlBuffer>         fFragmentUniformBuffer;
};

#endif
