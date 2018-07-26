/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlResourceProvider_DEFINED
#define GrMtlResourceProvider_DEFINED

#include "SkTarray.h"

#import <metal/metal.h>

class GrMtlGpu;

class GrMtlResourceProvider {
public:
    GrMtlResourceProvider(GrMtlGpu* gpu) : fGpu(gpu) {}

    id<MTLRenderPipelineState> findOrCreateCopyPipelineState(id<MTLTexture> dstTex);

private:
    struct MtlCopyPipelineStateInfo {
        id<MTLRenderPipelineState> fPipelineState;
        MTLPixelFormat             fPixelFormat;
    };

    struct MtlCopyPipelineResources {
        id<MTLFunction>      fVertexFunction;
        id<MTLFunction>      fFragmentFunction;
        MTLVertexDescriptor* fVertexDescriptor;
    } fCopyPipelineResources;

    void initCopyPipelineResources();

    SkTArray<MtlCopyPipelineStateInfo> fCopyPipelineStateCache;

    GrMtlGpu* fGpu;
};

#endif
