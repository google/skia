/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnProgramDataManager.h"

#include "src/gpu/dawn/GrDawnGpu.h"

GrDawnProgramDataManager::GrDawnProgramDataManager(const UniformInfoArray& uniforms,
                                                   uint32_t uniformBufferSize)
    : fUniformBufferSize(uniformBufferSize)
    , fUniformsDirty(false) {
    fUniformData.reset(uniformBufferSize);
    memset(fUniformData.get(), 0, fUniformBufferSize);
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const GrDawnUniformHandler::UniformInfo uniformInfo = uniforms[i];
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVar.getArrayCount();
            uniform.fType = uniformInfo.fVar.getType();
        );
        uniform.fOffset = uniformInfo.fUBOOffset;
    }
}

void* GrDawnProgramDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get()) + uni.fOffset;
}

void GrDawnProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &i, sizeof(int32_t));
}

void GrDawnProgramDataManager::set1iv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    uint32_t* buffer = static_cast<uint32_t*>(this->getBufferPtrAndMarkDirty(uni));
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(int32_t));
        buffer += 4;
    }
}

void GrDawnProgramDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[2] = { i0, i1 };
    memcpy(buffer, v, 2 * sizeof(int32_t));
}

void GrDawnProgramDataManager::set2iv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    uint32_t* buffer = static_cast<uint32_t*>(this->getBufferPtrAndMarkDirty(uni));
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(int32_t));
        buffer += 4;
    }
}

void GrDawnProgramDataManager::set3i(UniformHandle u, int32_t i0, int32_t i1, int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[3] = { i0, i1, i2 };
    memcpy(buffer, v, 3 * sizeof(int32_t));
}

void GrDawnProgramDataManager::set3iv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    uint32_t* buffer = static_cast<uint32_t*>(this->getBufferPtrAndMarkDirty(uni));
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(int32_t));
        buffer += 4;
    }
}

void GrDawnProgramDataManager::set4i(UniformHandle u,
                                     int32_t i0,
                                     int32_t i1,
                                     int32_t i2,
                                     int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[4] = { i0, i1, i2, i3 };
    memcpy(buffer, v, sizeof(v));
}

void GrDawnProgramDataManager::set4iv(UniformHandle u, int arrayCount, const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    uint32_t* buffer = static_cast<uint32_t*>(this->getBufferPtrAndMarkDirty(uni));
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[4 * i];
        memcpy(buffer, curVec, 4 * sizeof(int32_t));
        buffer += 4;
    }
}

void GrDawnProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &v0, sizeof(float));
}

void GrDawnProgramDataManager::set1fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrDawnProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
}

void GrDawnProgramDataManager::set2fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrDawnProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrDawnProgramDataManager::set3fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrDawnProgramDataManager::set4f(UniformHandle u,
                                     float v0,
                                     float v1,
                                     float v2,
                                     float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrDawnProgramDataManager::set4fv(UniformHandle u, int arrayCount, const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
}

void GrDawnProgramDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrDawnProgramDataManager::setMatrix2fv(UniformHandle u,
                                            int arrayCount,
                                            const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrDawnProgramDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrDawnProgramDataManager::setMatrix3fv(UniformHandle u,
                                            int arrayCount,
                                            const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrDawnProgramDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrDawnProgramDataManager::setMatrix4fv(UniformHandle u,
                                            int arrayCount,
                                            const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrDawnProgramDataManager::setMatrices(UniformHandle u,
                                                                  int arrayCount,
                                                                  const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    fUniformsDirty = true;
    set_uniform_matrix<N>::set(fUniformData.get(), uni.fOffset, arrayCount, matrices);
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

void GrDawnProgramDataManager::uploadUniformBuffers(GrDawnGpu* gpu,
                                                    GrDawnRingBuffer::Slice slice) const {
    auto copyEncoder = gpu->getCopyEncoder();
    if (slice.fBuffer && fUniformsDirty) {
        GrDawnStagingBuffer* stagingBuffer = gpu->getStagingBuffer(fUniformBufferSize);
        memcpy(stagingBuffer->fData, fUniformData.get(), fUniformBufferSize);
        stagingBuffer->fBuffer.Unmap();
        copyEncoder.CopyBufferToBuffer(
            stagingBuffer->fBuffer, 0, slice.fBuffer, slice.fOffset, fUniformBufferSize);
    }
}
