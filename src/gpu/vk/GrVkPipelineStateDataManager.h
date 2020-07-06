/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateDataManager_DEFINED
#define GrVkPipelineStateDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkUniformHandler.h"
#include "src/gpu/vk/GrVkTexture.h"

class GrVkGpu;
class GrVkUniformBuffer;
class GrVkPipelineState;

class GrVkPipelineStateDataManager : public GrUniformDataManager {
public:
    using UniformInfoArray = GrVkUniformHandler::UniformInfoArray;
    using SamplerBinding   = GrTextureBindingRecorder<GrVkTexture>::SamplerBinding;

    GrVkPipelineStateDataManager(const UniformInfoArray&,
                                 uint32_t uniformSize,
                                 int primitiveProcessorSamplerCnt,
                                 int textureEffectSamplerCnt);

    // Returns true if either the geometry or fragment buffers needed to generate a new underlying
    // VkBuffer object in order upload data. If true is returned, this is a signal to the caller
    // that they will need to update the descriptor set that is using these buffers.
    bool uploadUniformBuffers(GrVkGpu* gpu, GrVkUniformBuffer* buffer) const;

    void bindTextureEffectSampler(const GrTextureEffect& effect,
                                  SamplerHandle sampler) const override {
        fTextureBindings.bindTextureEffectSampler(effect, sampler);
    }

    int numTextureEffectSamplers() const { return fTextureBindings.numTextureEffectSamplers(); }

    SamplerBinding textureEffectSamplerBinding(int index) const {
        return fTextureBindings.textureEffectSamplerBinding(index);
    }

private:
    GrTextureBindingRecorder<GrVkTexture> fTextureBindings;

    typedef GrUniformDataManager INHERITED;
};

#endif
