/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrD3DPipelineStateDataManager_DEFINED
#define GrD3DPipelineStateDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DTexture.h"
#include "src/gpu/GrSPIRVUniformHandler.h"

class GrD3DConstantRingBuffer;
class GrD3DGpu;

class GrD3DPipelineStateDataManager : public GrUniformDataManager {
public:
    using UniformInfoArray = GrSPIRVUniformHandler::UniformInfoArray;
    using SamplerBinding   = GrTextureBindingRecorder<GrD3DTexture>::SamplerBinding;

    GrD3DPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t uniformSize,
                                  int primitiveProcessorSamplerCnt,
                                  int textureEffectSamplerCnt);

    void bindTextureEffectSampler(const GrTextureEffect& effect,
                                  SamplerHandle sampler) const override {
        fTextureBindings.bindTextureEffectSampler(effect, sampler);
    }

    int numTextureEffectSamplers() const { return fTextureBindings.numTextureEffectSamplers(); }

    SamplerBinding textureEffectSamplerBinding(int index) const {
        return fTextureBindings.textureEffectSamplerBinding(index);
    }

    D3D12_GPU_VIRTUAL_ADDRESS uploadConstants(GrD3DGpu* gpu);

private:
    GrTextureBindingRecorder<GrD3DTexture> fTextureBindings;
    D3D12_GPU_VIRTUAL_ADDRESS fConstantBufferAddress;

    typedef GrUniformDataManager INHERITED;
};

#endif
