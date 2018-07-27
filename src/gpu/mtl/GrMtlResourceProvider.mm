/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlResourceProvider.h"

#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

#include "SkSLCompiler.h"

#import <simd/simd.h>

id<MTLRenderPipelineState> GrMtlResourceProvider::findOrCreateCopyPipelineState(
        id<MTLTexture> dstTex, id<MTLFunction> vertexFunction, id<MTLFunction> fragmentFunction,
        MTLVertexDescriptor* vertexDescriptor) {

    for (const auto& pipelineState: fCopyPipelineStateCache) {
        if (pipelineState.fPixelFormat == dstTex.pixelFormat) {
            SkASSERT(pipelineState.fPipelineState);
            return pipelineState.fPipelineState;
        }
    }

    // Create pipeline state for copy as draw
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineDescriptor.colorAttachments[0].pixelFormat = dstTex.pixelFormat;

    NSError* error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nil;
    }

    SkASSERT(pipelineState);
    fCopyPipelineStateCache.emplace_back(GrMtlCopyPipelineState(pipelineState, dstTex.pixelFormat));
    return pipelineState;
}
