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
#include <memory>

class GrD3DGpu;
class GrD3DRootSignature;
class GrProgramInfo;

class GrD3DPipelineState {
public:
    static std::unique_ptr<GrD3DPipelineState> Make(GrD3DGpu* gpu, const GrProgramInfo&,
                                                    sk_sp<GrD3DRootSignature> rootSig,
                                                    gr_cp<ID3DBlob> vertexShader,
                                                    gr_cp<ID3DBlob> geometryShader,
                                                    gr_cp<ID3DBlob> pixelShader,
                                                    DXGI_FORMAT renderTargetFormat,
                                                    DXGI_FORMAT depthStencilFormat,
                                                    unsigned int sampleQualityLevel);

private:
    GrD3DPipelineState(gr_cp<ID3D12PipelineState> pipelineState);

    gr_cp<ID3D12PipelineState> fPipelineState;

};

#endif
