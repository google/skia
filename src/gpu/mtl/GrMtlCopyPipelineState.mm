/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlCopyPipelineState.h"
#include "GrMtlGpu.h"

GrMtlCopyPipelineState* GrMtlCopyPipelineState::CreateCopyPipelineState(
        GrMtlGpu* gpu,
        MTLPixelFormat dstPixelFormat,
        id<MTLFunction> vertexFunction,
        id<MTLFunction> fragmentFunction,
        MTLVertexDescriptor* vertexDescriptor) {

    // Create pipeline state for copy as draw
    MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineDescriptor.colorAttachments[0].pixelFormat = dstPixelFormat;

    NSError* error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [gpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                          error: &error];
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nil;
    }

    SkASSERT(pipelineState);
    return new GrMtlCopyPipelineState(pipelineState, dstPixelFormat);
}
