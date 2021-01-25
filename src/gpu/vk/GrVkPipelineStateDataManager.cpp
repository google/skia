/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkPipelineStateDataManager.h"

#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUniformBuffer.h"

GrVkPipelineStateDataManager::GrVkPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                           uint32_t uniformSize,
                                                           GrVkUniformHandler::Layout layout)
    : INHERITED(uniforms.count(), uniformSize)
    , fLayout(layout) {
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

        uniform.fOffset = uniformInfo.fOffsets[layout];
        ++i;
    }
}

void GrVkPipelineStateDataManager::setMatrix2fv(UniformHandle u, int arrayCount,
                                                const float matrices[]) const {
    if (fLayout == GrVkUniformHandler::kStd430Layout) {
        const Uniform& uni = fUniforms[u.toIndex()];
        void* buffer = fUniformData.get();
        fUniformsDirty = true;
        static_assert(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uni.fOffset;
        for (int i = 0; i < arrayCount; ++i) {
            const float* matrix = &matrices[2 * 2 * i];
            for (int j = 0; j < 2; ++j) {
                memcpy(buffer, &matrix[j * 2], 2 * sizeof(float));
                buffer = static_cast<char*>(buffer) + 2 * sizeof(float);
            }
        }
    } else {
        INHERITED::setMatrix2fv(u, arrayCount, matrices);
    }
}

bool GrVkPipelineStateDataManager::uploadUniformBuffers(GrVkGpu* gpu,
                                                        GrVkUniformBuffer* buffer) const {
    bool updatedBuffer = false;
    if (buffer && fUniformsDirty) {
        SkAssertResult(buffer->updateData(gpu, fUniformData.get(),
                                          fUniformSize, &updatedBuffer));
        fUniformsDirty = false;
    }

    return updatedBuffer;
}

void GrVkPipelineStateDataManager::uploadPushConstants(const GrVkGpu* gpu,
                                                       VkPipelineLayout layout,
                                                       GrVkCommandBuffer* commandBuffer) {
    commandBuffer->pushConstants(gpu, layout,
                                 GrPushConstantStageFlags(gpu),
                                 0, fUniformSize, fUniformData.get());
}
