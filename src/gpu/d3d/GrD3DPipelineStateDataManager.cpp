/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/d3d/GrD3DPipelineStateDataManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"

GrD3DPipelineStateDataManager::GrD3DPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                             uint32_t uniformSize)
    : INHERITED(uniforms.count(), uniformSize) {
    // We must add uniforms in same order as the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        )

        uniform.fOffset = uniformInfo.fUBOOffset;
        ++i;
    }
}

D3D12_GPU_VIRTUAL_ADDRESS GrD3DPipelineStateDataManager::uploadConstants(GrD3DGpu* gpu) {
    if (fUniformsDirty) {
        fConstantBufferAddress = gpu->resourceProvider().uploadConstantData(fUniformData.get(),
                                                                            fUniformSize);
        fUniformsDirty = false;
    }

    return fConstantBufferAddress;
}
