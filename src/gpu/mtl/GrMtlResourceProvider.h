/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlResourceProvider_DEFINED
#define GrMtlResourceProvider_DEFINED

#include "GrMtlPipelineState.h"
#include "SkTArray.h"

#import <metal/metal.h>

class GrMtlGpu;

class GrMtlResourceProvider {
public:
    GrMtlResourceProvider(GrMtlGpu* gpu) : fGpu(gpu) {}

    id<MTLRenderPipelineState> findOrCreateCopyPipelineState(id<MTLTexture> dstTex,
                                                             id<MTLFunction> vertexFunction,
                                                             id<MTLFunction> fragmentFunction,
                                                             MTLVertexDescriptor*);

private:
    SkTArray<GrMtlPipelineState> fCopyPipelineStateCache;

    GrMtlGpu* fGpu;
};

#endif
