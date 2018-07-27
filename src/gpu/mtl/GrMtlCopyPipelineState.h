/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCopyPipelineState_DEFINED
#define GrMtlCopyPipelineState_DEFINED

#import <metal/metal.h>

class GrMtlGpu;

class GrMtlCopyPipelineState {
public:
    GrMtlCopyPipelineState(id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat)
            : fPipelineState(pipelineState)
            , fPixelFormat(pixelFormat) {}

    id<MTLRenderPipelineState> mtlCopyPipelineState() { return fPipelineState; }

private:
    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat fPixelFormat;

    friend class GrMtlResourceProvider;
};

#endif
