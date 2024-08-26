/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrD3DPipelineStateDataManager_DEFINED
#define GrD3DPipelineStateDataManager_DEFINED

#include "src/gpu/ganesh/GrUniformDataManager.h"

#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/GrSPIRVUniformHandler.h"

class GrD3DConstantRingBuffer;
class GrD3DGpu;

class GrD3DPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrSPIRVUniformHandler::UniformInfoArray UniformInfoArray;

    GrD3DPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t uniformSize);

    D3D12_GPU_VIRTUAL_ADDRESS uploadConstants(GrD3DGpu* gpu);

private:
    D3D12_GPU_VIRTUAL_ADDRESS fConstantBufferAddress;

    using INHERITED = GrUniformDataManager;
};

#endif
