/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipeline_DEFINED
#define GrMtlPipeline_DEFINED

#include "src/gpu/ganesh/GrManagedResource.h"

#import <Metal/Metal.h>

/**
 * Wraps a MTLRenderPipelineState object
 */
class GrMtlRenderPipeline : public GrManagedResource {
public:
    static sk_sp<GrMtlRenderPipeline> Make(id<MTLRenderPipelineState> pso) {
        return sk_sp<GrMtlRenderPipeline>(new GrMtlRenderPipeline(pso));
    }
#ifdef SK_TRACE_MANAGED_RESOURCES
    /** output a human-readable dump of this resource's information
     */
    void dumpInfo() const override {
        SkDebugf("GrMtlRenderPipeline: %p (%ld refs)\n", fPipelineState,
                 CFGetRetainCount((CFTypeRef)fPipelineState));
    }
#endif

    void freeGPUData() const override {
        fPipelineState = nil;
    }

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState; }

private:
    GrMtlRenderPipeline(id<MTLRenderPipelineState> pso)
        : GrManagedResource()
        , fPipelineState(pso) {
    }

    mutable id<MTLRenderPipelineState> fPipelineState;
};

#endif
