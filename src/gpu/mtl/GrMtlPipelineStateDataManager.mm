/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateDataManager.h"

#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

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
            uniform.fType = uniformInfo.fVariable.getType();
        )
        uniform.fOffset = uniformInfo.fUBOffset;
        ++i;
    }
}

void GrMtlPipelineStateDataManager::set1iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set1fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * sizeof(float));
}

void GrMtlPipelineStateDataManager::set2iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 2 * sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set2fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 2 * sizeof(float));
}

void GrMtlPipelineStateDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrix2fv(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType || uni.fType == kHalf2x2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, m, arrayCount * 4 * sizeof(float));
}

void GrMtlPipelineStateDataManager::uploadAndBindUniformBuffers(
        GrMtlGpu* gpu,
        id<MTLRenderCommandEncoder> renderCmdEncoder) const {
    if (fUniformSize && fUniformsDirty) {
        if (@available(macOS 10.11, iOS 8.3, *)) {
            SkASSERT(fUniformSize <= gpu->caps()->maxPushConstantsSize());
            [renderCmdEncoder setVertexBytes: fUniformData.get()
                                      length: fUniformSize
                                     atIndex: GrMtlUniformHandler::kUniformBinding];
            [renderCmdEncoder setFragmentBytes: fUniformData.get()
                                        length: fUniformSize
                                       atIndex: GrMtlUniformHandler::kUniformBinding];
        } else {
            // We only support iOS 9.0+, so we should never hit this
            SK_ABORT("Missing interface. Skia only supports Metal on iOS 9.0 and higher");
        }
        fUniformsDirty = false;
    }
}

void GrMtlPipelineStateDataManager::resetDirtyBits() {
    fUniformsDirty = true;
}
