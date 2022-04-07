/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DPipeline_DEFINED
#define GrD3DPipeline_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/GrManagedResource.h"

class GrD3DPipeline : public GrManagedResource {
public:
    static sk_sp<GrD3DPipeline> Make(gr_cp<ID3D12PipelineState> pipelineState) {
        return sk_sp<GrD3DPipeline>(new GrD3DPipeline(std::move(pipelineState)));
    }
#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
    */
    void dumpInfo() const override {
        SkDebugf("GrD3DPipeline: %p (%d refs)\n", fPipelineState.get(), this->getRefCnt());
    }
#endif

    // This will be called right before this class is destroyed and there is no reason to explicitly
    // release the fPipelineState cause the gr_cp will handle that in the dtor.
    void freeGPUData() const override {}

    ID3D12PipelineState* d3dPipelineState() const { return fPipelineState.get(); }

private:
    GrD3DPipeline(gr_cp<ID3D12PipelineState> pipelineState)
        : fPipelineState(std::move(pipelineState)) {
    }

    gr_cp<ID3D12PipelineState> fPipelineState;

    using INHERITED = GrManagedResource;
};

#endif
