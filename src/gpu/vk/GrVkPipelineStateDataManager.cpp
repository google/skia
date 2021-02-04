/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkPipelineStateDataManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkGpu.h"

GrVkPipelineStateDataManager::GrVkPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                           uint32_t uniformSize,
                                                           GrVkUniformHandler::Layout memLayout)
    : INHERITED(uniforms.count(), uniformSize)
    , fMemLayout(memLayout) {
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

        uniform.fOffset = uniformInfo.fOffsets[memLayout];
        ++i;
    }
}

std::pair<sk_sp<GrGpuBuffer>, bool> GrVkPipelineStateDataManager::uploadUniformBuffers(
        GrVkGpu* gpu) {
    if (fUniformSize == 0) {
        return std::make_pair(nullptr, true);
    }
    if (fUniformsDirty) {
        GrResourceProvider* resourceProvider = gpu->getContext()->priv().resourceProvider();
        fUniformBuffer = resourceProvider->createBuffer(
                fUniformSize, GrGpuBufferType::kUniform, kDynamic_GrAccessPattern,
                 fUniformData.get());
        if (!fUniformBuffer) {
            return std::make_pair(nullptr, false);
        }
        fUniformsDirty = false;
    }

    return std::make_pair(fUniformBuffer, true);
}

void GrVkPipelineStateDataManager::uploadPushConstants(const GrVkGpu* gpu,
                                                       VkPipelineLayout layout,
                                                       GrVkCommandBuffer* commandBuffer) const {
    commandBuffer->pushConstants(gpu, layout,
                                 gpu->vkCaps().getPushConstantStageFlags(),
                                 0, fUniformSize, fUniformData.get());
}

void GrVkPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                int arrayCount,
                                                const float m[]) const {
    if (fMemLayout == GrVkUniformHandler::kStd430Layout) {
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == kFloat2x2_GrSLType || uni.fType == kHalf2x2_GrSLType);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = fUniformData.get();
        fUniformsDirty = true;

        static_assert(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uni.fOffset;
        memcpy(buffer, m, arrayCount * 2 * 2 * sizeof(float));
    } else {
        this->INHERITED::setMatrix2fv(u, arrayCount, m);
    }
}

void GrVkPipelineStateDataManager::releaseData() { fUniformBuffer.reset(); }
