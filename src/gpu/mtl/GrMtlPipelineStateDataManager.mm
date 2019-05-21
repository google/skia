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
                                                             uint32_t geometryUniformSize,
                                                             uint32_t fragmentUniformSize)
        : fGeometryUniformSize(geometryUniformSize)
        , fFragmentUniformSize(fragmentUniformSize)
        , fGeometryUniformsDirty(false)
        , fFragmentUniformsDirty(false) {
    fGeometryUniformData.reset(geometryUniformSize);
    fFragmentUniformData.reset(fragmentUniformSize);
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const GrMtlUniformHandler::UniformInfo uniformInfo = uniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        );

        if (!(kFragment_GrShaderFlag & uniformInfo.fVisibility)) {
            uniform.fBinding = GrMtlUniformHandler::kGeometryBinding;
        } else {
            SkASSERT(kFragment_GrShaderFlag == uniformInfo.fVisibility);
            uniform.fBinding = GrMtlUniformHandler::kFragBinding;
        }
        uniform.fOffset = uniformInfo.fUBOffset;
    }
}

void* GrMtlPipelineStateDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    void* buffer;
    if (GrMtlUniformHandler::kGeometryBinding == uni.fBinding) {
        buffer = fGeometryUniformData.get();
        fGeometryUniformsDirty = true;
    } else {
        SkASSERT(GrMtlUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
        fFragmentUniformsDirty = true;
    }
    buffer = static_cast<char*>(buffer)+uni.fOffset;
    return buffer;
}

void GrMtlPipelineStateDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &i, sizeof(int32_t));
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
    SkASSERT(sizeof(int32_t) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrMtlPipelineStateDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, &v0, sizeof(float));
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
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * sizeof(float));
}

void GrMtlPipelineStateDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[2] = { i0, i1 };
    memcpy(buffer, v, 2 * sizeof(int32_t));
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
    SkASSERT(sizeof(int32_t) == 4);
    memcpy(buffer, v, arrayCount*sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
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
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * 2 * sizeof(float));
}

void GrMtlPipelineStateDataManager::set3i(UniformHandle u,
                                          int32_t i0,
                                          int32_t i1,
                                          int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[3] = { i0, i1, i2 };
    memcpy(buffer, v, 3 * sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set3iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(int32_t) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrMtlPipelineStateDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrMtlPipelineStateDataManager::set3fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrMtlPipelineStateDataManager::set4i(UniformHandle u,
                                          int32_t i0,
                                          int32_t i1,
                                          int32_t i2,
                                          int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[4] = { i0, i1, i2, i3 };
    memcpy(buffer, v, 4 * sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set4iv(UniformHandle u,
                                           int arrayCount,
                                           const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(int32_t) == 4);
    memcpy(buffer, v, arrayCount * 4 * sizeof(int32_t));
}

void GrMtlPipelineStateDataManager::set4f(UniformHandle u,
                                          float v0,
                                          float v1,
                                          float v2,
                                          float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrMtlPipelineStateDataManager::set4fv(UniformHandle u,
                                           int arrayCount,
                                           const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
}

void GrMtlPipelineStateDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrMtlPipelineStateDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix3fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrMtlPipelineStateDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrMtlPipelineStateDataManager::setMatrix4fv(UniformHandle u,
                                                 int arrayCount,
                                                 const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrMtlPipelineStateDataManager::setMatrices(
        UniformHandle u,
        int arrayCount,
        const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer;
    if (GrMtlUniformHandler::kGeometryBinding == uni.fBinding) {
        buffer = fGeometryUniformData.get();
        fGeometryUniformsDirty = true;
    } else {
        SkASSERT(GrMtlUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
        fFragmentUniformsDirty = true;
    }

    set_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<> struct set_uniform_matrix<2> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * 4 * sizeof(float));
    }
};

template<> struct set_uniform_matrix<3> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        for (int i = 0; i < count; ++i) {
            const float* matrix = &matrices[3 * 3 * i];
            for (int j = 0; j < 3; ++j) {
                memcpy(buffer, &matrix[j * 3], 3 * sizeof(float));
                buffer = static_cast<char*>(buffer) + 4 * sizeof(float);
            }
        }
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * 16 * sizeof(float));
    }
};

void GrMtlPipelineStateDataManager::uploadAndBindUniformBuffers(
        GrMtlGpu* gpu,
        id<MTLRenderCommandEncoder> renderCmdEncoder) const {
    if (fGeometryUniformSize && fGeometryUniformsDirty) {
        SkASSERT(fGeometryUniformSize < 4*1024);
        [renderCmdEncoder setVertexBytes: fGeometryUniformData.get()
                                  length: fGeometryUniformSize
                                 atIndex: GrMtlUniformHandler::kGeometryBinding];
        fGeometryUniformsDirty = false;
    }
    if (fFragmentUniformSize && fFragmentUniformsDirty) {
        SkASSERT(fFragmentUniformSize < 4*1024);
        [renderCmdEncoder setFragmentBytes: fFragmentUniformData.get()
                                    length: fFragmentUniformSize
                                   atIndex: GrMtlUniformHandler::kFragBinding];
        fFragmentUniformsDirty = false;
    }
}

void GrMtlPipelineStateDataManager::resetDirtyBits() {
    fGeometryUniformsDirty = true;
    fFragmentUniformsDirty = true;
}
