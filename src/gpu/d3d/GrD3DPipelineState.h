/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DPipelineState_DEFINED
#define GrD3DPipelineState_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"

class GrD3DGpu;
class GrD3DRootSignature;
class GrProgramInfo;

class GrD3DPipelineState : public GrManagedResource {
public:
    static sk_sp<GrD3DPipelineState> Make(GrD3DGpu* gpu, const GrProgramInfo&,
                                          sk_sp<GrD3DRootSignature> rootSig,
                                          gr_cp<ID3DBlob> vertexShader,
                                          gr_cp<ID3DBlob> geometryShader,
                                          gr_cp<ID3DBlob> pixelShader,
                                          DXGI_FORMAT renderTargetFormat,
                                          DXGI_FORMAT depthStencilFormat,
                                          unsigned int sampleQualityLevel);

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
    */
    void dumpInfo() const override {
        SkDebugf("GrD3DPipelineState: %p (%d refs)\n", fPipelineState.get(), this->getRefCnt());
    }
#endif

    // This will be called right before this class is destroyed and there is no reason to explicitly
    // release the fPipelineState cause the gr_cp will handle that in the dtor.
    void freeGPUData() const override {}

    ID3D12PipelineState* pipelineState() const { return fPipelineState.get(); }

private:
    GrD3DPipelineState(gr_cp<ID3D12PipelineState> pipelineState);

    gr_cp<ID3D12PipelineState> fPipelineState;

};

#endif
