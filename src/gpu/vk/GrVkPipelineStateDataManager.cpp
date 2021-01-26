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

void GrVkPipelineStateDataManager::set1iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set1iv(u, arrayCount, v);
    }

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(int32_t) == 4);
    memcpy(buffer, v, arrayCount * sizeof(int32_t));
}

void GrVkPipelineStateDataManager::set1fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set1fv(u, arrayCount, v);
    }

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * sizeof(float));
}

void GrVkPipelineStateDataManager::set2iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set2iv(u, arrayCount, v);
    }

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(int32_t) == 4);
    memcpy(buffer, v, arrayCount * 2 * sizeof(int32_t));
}

void GrVkPipelineStateDataManager::set2fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set2fv(u, arrayCount, v);
    }

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * 2 * sizeof(float));
}

void GrVkPipelineStateDataManager::set3iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set3iv(u, arrayCount, v);
    }

    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(int32_t) == 4);
    memcpy(buffer, v, arrayCount * 3 * sizeof(int32_t));
}

void GrVkPipelineStateDataManager::set3fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    //if (fLayout == GrVkUniformHandler::kStd140Layout) {
        return this->INHERITED::set3fv(u, arrayCount, v);
    //}

    //const Uniform& uni = fUniforms[u.toIndex()];
    //SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    //SkASSERT(arrayCount > 0);
    //SkASSERT(arrayCount <= uni.fArrayCount ||
    //         (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    //void* buffer = this->getBufferPtrAndMarkDirty(uni);
    //SkASSERT(sizeof(float) == 4);
    //memcpy(buffer, v, arrayCount * 3 * sizeof(float));
}

void GrVkPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                int arrayCount,
                                                const float m[]) const {
    if (fLayout == GrVkUniformHandler::kStd430Layout) {
        this->setStd430Matrices<2>(u, arrayCount, m);
    } else {
        this->INHERITED::setMatrix2fv(u, arrayCount, m);
    }
}

void GrVkPipelineStateDataManager::setMatrix3fv(UniformHandle u,
                                                int arrayCount,
                                                const float m[]) const {
    //if (fLayout == GrVkUniformHandler::kStd430Layout) {
    //    this->setStd430Matrices<3>(u, arrayCount, m);
    //} else {
        this->INHERITED::setMatrix3fv(u, arrayCount, m);
    //}
}

template<int N> struct set_std430_uniform_matrix;

template<int N> inline void GrVkPipelineStateDataManager::setStd430Matrices(
        UniformHandle u, int arrayCount, const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = fUniformData.get();
    fUniformsDirty = true;

    set_std430_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<int N> struct set_std430_uniform_matrix {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        static_assert(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * N * N * sizeof(float));
    }
};

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
