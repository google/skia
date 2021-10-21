/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrShaderVar.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

GrUniformDataManager::GrUniformDataManager(uint32_t uniformCount, uint32_t uniformSize)
    : fUniformSize(uniformSize)
    , fUniformsDirty(false) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniformCount);
    // subclasses fill in the uniforms in their constructor
}

void* GrUniformDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get())+uni.fOffset;
}

template <int N, GrSLType FullType, GrSLType HalfType>
void GrUniformDataManager::set(UniformHandle u, const void* v) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, N * 4);
}

template <int N, GrSLType FullType, GrSLType HalfType>
void GrUniformDataManager::setv(UniformHandle u, int arrayCount, const void* v) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        memcpy(buffer, v, arrayCount * 16);
    }
    else {
        for (int i = 0; i < arrayCount; ++i) {
            memcpy(buffer, v, N * 4);
            buffer = static_cast<char*>(buffer) + 16;
            v = static_cast<const char*>(v) + N * 4;
        }
    }
}

void GrUniformDataManager::set1i(UniformHandle u, int32_t i0) const {
    this->set<1, kInt_GrSLType, kShort_GrSLType>(u, &i0);
}

void GrUniformDataManager::set1iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<1, kInt_GrSLType, kShort_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set1f(UniformHandle u, float v0) const {
    this->set<1, kFloat_GrSLType, kHalf_GrSLType>(u, &v0);
}

void GrUniformDataManager::set1fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<1, kFloat_GrSLType, kHalf_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    int32_t v[2] = { i0, i1 };
    this->set<2, kInt2_GrSLType, kShort2_GrSLType>(u, v);
}

void GrUniformDataManager::set2iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<2, kInt2_GrSLType, kShort2_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set2f(UniformHandle u, float v0, float v1) const {
    float v[2] = { v0, v1 };
    this->set<2, kFloat2_GrSLType, kHalf2_GrSLType>(u, v);
}

void GrUniformDataManager::set2fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<2, kFloat2_GrSLType, kHalf2_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set3i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2) const {
    int32_t v[3] = { i0, i1, i2 };
    this->set<3, kInt3_GrSLType, kShort3_GrSLType>(u, v);
}

void GrUniformDataManager::set3iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<3, kInt3_GrSLType, kShort3_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    float v[3] = { v0, v1, v2 };
    this->set<3, kFloat3_GrSLType, kHalf3_GrSLType>(u, v);
}

void GrUniformDataManager::set3fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<3, kFloat3_GrSLType, kHalf3_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set4i(UniformHandle u,
                                 int32_t i0,
                                 int32_t i1,
                                 int32_t i2,
                                 int32_t i3) const {
    int32_t v[4] = { i0, i1, i2, i3 };
    this->set<4, kInt4_GrSLType, kShort4_GrSLType>(u, v);
}

void GrUniformDataManager::set4iv(UniformHandle u,
                                  int arrayCount,
                                  const int32_t v[]) const {
    this->setv<4, kInt4_GrSLType, kShort4_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::set4f(UniformHandle u,
                                 float v0,
                                 float v1,
                                 float v2,
                                 float v3) const {
    float v[4] = { v0, v1, v2, v3 };
    this->set<4, kFloat4_GrSLType, kHalf4_GrSLType>(u, v);
}

void GrUniformDataManager::set4fv(UniformHandle u,
                                  int arrayCount,
                                  const float v[]) const {
    this->setv<4, kFloat4_GrSLType, kHalf4_GrSLType>(u, arrayCount, v);
}

void GrUniformDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template <int N> inline void GrUniformDataManager::setMatrices(UniformHandle u,
                                                               int arrayCount,
                                                               const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        memcpy(buffer, matrices, arrayCount * 16 * sizeof(float));
    } else {
        for (int i = 0; i < arrayCount; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                memcpy(buffer, &matrix[j * N], N * sizeof(float));
                buffer = static_cast<char*>(buffer) + 4 * sizeof(float);
            }
        }
    }
}
