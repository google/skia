/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineState_DEFINED
#define GrMtlPipelineState_DEFINED

#import <metal/metal.h>

class GrMtlGpu;

/**
 * Wraps a MTLRenderPipelineState object and also contains more info about the pipeline as needed
 * by Ganesh
 */
class GrMtlPipelineState {
public:
<<<<<<< HEAD
    GrMtlPipelineState(GrMtlGpu* gpu,
                       id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat,
                       uint32_t geometryUniformSize,
                       uint32_t fragmentUniformSize);
=======
    GrMtlPipelineState(id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat)
            : fPipelineState(pipelineState)
            , fPixelFormat(pixelFormat) {
        (void) fPixelFormat; // Silence unused-var warning.
}
>>>>>>> b364f6deac60b74ff1e618d5d93095ebd798e950

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState; }

private:
    GrMtlGpu* fGpu;

    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat             fPixelFormat;
    id<MTLBuffer>              fGeometryUniformBuffer;
    id<MTLBuffer>              fFragmentUniformBuffer;
};

#endif
