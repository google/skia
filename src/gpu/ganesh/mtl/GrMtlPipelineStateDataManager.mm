/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlPipelineStateDataManager.h"

#include "src/gpu/ganesh/mtl/GrMtlBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderCommandEncoder.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlPipelineStateDataManager::GrMtlPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                             uint32_t uniformSize)
        : INHERITED(uniforms.count(), uniformSize) {
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
        )
        uniform.fOffset = uniformInfo.fUBOffset;
        uniform.fType = uniformInfo.fVariable.getType();
        ++i;
    }

    fWrite16BitUniforms = true;
}

void GrMtlPipelineStateDataManager::set1iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt || uni.fType == SkSLType::kShort);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, v, arrayCount, uni.fType);
}

void GrMtlPipelineStateDataManager::set1fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat || uni.fType == SkSLType::kHalf);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, v, arrayCount, uni.fType);
}

void GrMtlPipelineStateDataManager::set2iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt2 || uni.fType == SkSLType::kShort2);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, v, arrayCount * 2, uni.fType);
}

void GrMtlPipelineStateDataManager::set2fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat2 || uni.fType == SkSLType::kHalf2);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, v, arrayCount * 2, uni.fType);
}

void GrMtlPipelineStateDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrix2fv(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat2x2 || uni.fType == SkSLType::kHalf2x2);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, m, arrayCount * 4, uni.fType);
}

void GrMtlPipelineStateDataManager::uploadAndBindUniformBuffers(
        GrMtlGpu* gpu,
        GrMtlRenderCommandEncoder* renderCmdEncoder) const {
    if (fUniformSize && fUniformsDirty) {
        fUniformsDirty = false;
        if (@available(macOS 10.11, iOS 8.3, tvOS 9.0, *)) {
            if (fUniformSize <= gpu->caps()->maxPushConstantsSize()) {
                renderCmdEncoder->setVertexBytes(fUniformData.get(), fUniformSize,
                                                 GrMtlUniformHandler::kUniformBinding);
                renderCmdEncoder->setFragmentBytes(fUniformData.get(), fUniformSize,
                                                   GrMtlUniformHandler::kUniformBinding);
                return;
            }
        }

        // upload the data
        GrRingBuffer::Slice slice = gpu->uniformsRingBuffer()->suballocate(fUniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, fUniformData.get(), fUniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset,
                                          GrMtlUniformHandler::kUniformBinding);
        renderCmdEncoder->setFragmentBuffer(buffer->mtlBuffer(), slice.fOffset,
                                            GrMtlUniformHandler::kUniformBinding);
    }
}

void GrMtlPipelineStateDataManager::resetDirtyBits() {
    fUniformsDirty = true;
}

GR_NORETAIN_END
