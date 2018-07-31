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
    static GrMtlCopyPipelineState* CreateCopyPipelineState(GrMtlGpu* gpu,
                                                           MTLPixelFormat dstPixelFormat,
                                                           id<MTLFunction> vertexFunction,
                                                           id<MTLFunction> fragmentFunction,
                                                           MTLVertexDescriptor* vertexDescriptor);

    id<MTLRenderPipelineState> mtlCopyPipelineState() { return fPipelineState; }

private:
    GrMtlCopyPipelineState(id<MTLRenderPipelineState> pipelineState,
                       MTLPixelFormat pixelFormat)
            : fPipelineState(pipelineState)
            , fPixelFormat(pixelFormat) {}

    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat fPixelFormat;

    friend class GrMtlCopyManager;
};

#endif
