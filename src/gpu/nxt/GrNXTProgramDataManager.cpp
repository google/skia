/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTProgramDataManager.h"

#include "GrNXTGpu.h"

GrNXTProgramDataManager::GrNXTProgramDataManager(const UniformInfoArray& uniforms,
                                                 uint32_t geometryUniformSize,
                                                 uint32_t fragmentUniformSize)
    : fGeometryUniformSize(geometryUniformSize)
    , fFragmentUniformSize(fragmentUniformSize)
    , fGeometryUniformsDirty(false)
    , fFragmentUniformsDirty(false) {
    fGeometryUniformData.reset(geometryUniformSize);
    fFragmentUniformData.reset(fragmentUniformSize);
    memset(fGeometryUniformData.get(), 0, fGeometryUniformSize);
    memset(fFragmentUniformData.get(), 0, fFragmentUniformSize);
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const GrNXTUniformHandler::UniformInfo uniformInfo = uniforms[i];
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVar.getArrayCount();
            uniform.fType = uniformInfo.fVar.getType();
        );

        if (!(kFragment_GrShaderFlag & uniformInfo.fVisibility)) {
            uniform.fBinding = GrNXTUniformHandler::kGeometryBinding;
        } else {
            SkASSERT(kFragment_GrShaderFlag == uniformInfo.fVisibility);
            uniform.fBinding = GrNXTUniformHandler::kFragBinding;
        }
        uniform.fOffset = uniformInfo.fUBOOffset;
    }
}

void* GrNXTProgramDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    void* buffer;
    if (GrNXTUniformHandler::kGeometryBinding == uni.fBinding) {
        buffer = fGeometryUniformData.get();
        fGeometryUniformsDirty = true;
    } else {
        SkASSERT(GrNXTUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
        fFragmentUniformsDirty = true;
    }
    buffer = static_cast<char*>(buffer)+uni.fOffset;
    return buffer;
}

void GrNXTProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &i, sizeof(int32_t));
}

void GrNXTProgramDataManager::set1iv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrNXTProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &v0, sizeof(float));
}

void GrNXTProgramDataManager::set1fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrNXTProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
}

void GrNXTProgramDataManager::set2fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrNXTProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrNXTProgramDataManager::set3fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrNXTProgramDataManager::set4f(UniformHandle u, float v0, float v1, float v2, float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrNXTProgramDataManager::set4fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
}

void GrNXTProgramDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrNXTProgramDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrNXTProgramDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrNXTProgramDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrNXTProgramDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrNXTProgramDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrNXTProgramDataManager::setMatrices(UniformHandle u,
                                                                 int arrayCount,
                                                                 const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer;
    if (GrNXTUniformHandler::kGeometryBinding == uni.fBinding) {
        buffer = fGeometryUniformData.get();
        fGeometryUniformsDirty = true;
    } else {
        SkASSERT(GrNXTUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
        fFragmentUniformsDirty = true;
    }

    set_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<int N> struct set_uniform_matrix {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        for (int i = 0; i < count; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                memcpy(buffer, &matrix[j * N], N * sizeof(float));
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

void GrNXTProgramDataManager::uploadUniformBuffers(GrNXTGpu* gpu,
                                                   GrNXTRingBuffer::Slice geometryBuffer,
                                                   GrNXTRingBuffer::Slice fragmentBuffer) const {

    nxt::Buffer geom = geometryBuffer.fBuffer.Clone();
    uint32_t geomOffset = geometryBuffer.fOffset;
    nxt::Buffer frag = fragmentBuffer.fBuffer.Clone();
    uint32_t fragOffset = fragmentBuffer.fOffset;
    auto copyBuilder = gpu->getCopyBuilder();
    if (geom && fGeometryUniformsDirty) {
        GrNXTStagingBuffer* stagingBuffer = gpu->getStagingBuffer(fGeometryUniformSize);
        memcpy(stagingBuffer->fData, fGeometryUniformData.get(), fGeometryUniformSize);
        stagingBuffer->fBuffer.Unmap();
        copyBuilder
            .CopyBufferToBuffer(stagingBuffer->fBuffer, 0, geom, geomOffset, fGeometryUniformSize);
    }
    if (frag && fFragmentUniformsDirty) {
        GrNXTStagingBuffer* stagingBuffer = gpu->getStagingBuffer(fFragmentUniformSize);
        memcpy(stagingBuffer->fData, fFragmentUniformData.get(), fFragmentUniformSize);
        stagingBuffer->fBuffer.Unmap();
        copyBuilder
            .CopyBufferToBuffer(stagingBuffer->fBuffer, 0, frag, fragOffset, fFragmentUniformSize);
    }
}
