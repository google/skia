/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramDataManager_DEFINED
#define GrDawnProgramDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrSPIRVUniformHandler.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "dawn/webgpu_cpp.h"

#include "src/core/SkAutoMalloc.h"

class GrDawnGpu;
class GrDawnUniformBuffer;

class GrDawnProgramDataManager : public GrUniformDataManager {
public:
    using UniformInfoArray = GrSPIRVUniformHandler::UniformInfoArray;
    using SamplerBinding   = GrTextureBindingRecorder<GrDawnTexture>::SamplerBinding;

    GrDawnProgramDataManager(const UniformInfoArray&,
                             uint32_t uniformBufferSize,
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

    uint32_t uniformBufferSize() const { return fUniformSize; }

    void uploadUniformBuffers(void* dest) const;

private:
    GrTextureBindingRecorder<GrDawnTexture> fTextureBindings;
};

#endif
