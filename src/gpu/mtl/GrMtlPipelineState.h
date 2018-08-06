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

class GrMtlPipelineState {
public:
    GrMtlPipelineState(id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat)
            : fPipelineState(pipelineState)
            , fPixelFormat(pixelFormat) {
        (void) fPixelFormat; // Silence unused-var warning.
}

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState; }

private:
    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat fPixelFormat;
};

#endif
