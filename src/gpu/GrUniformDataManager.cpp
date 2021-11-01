/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "include/private/SkHalf.h"
#include "src/gpu/GrShaderVar.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);
static_assert(sizeof(short) == 2);
static_assert(sizeof(SkHalf) == 2);

GrUniformDataManager::GrUniformDataManager(uint32_t uniformCount, uint32_t uniformSize)
    : fUniformSize(uniformSize)
    , fUniformsDirty(false) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniformCount);
    // subclasses fill in the uniforms in their constructor
}

void* GrUniformDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get()) + uni.fOffset;
}

int GrUniformDataManager::copyUniforms(void* dest,
                                       const void* src,
                                       int numUniforms,
                                       GrSLType uniformType) const {
    if (fWrite16BitUniforms) {
        switch (uniformType) {
            case kHalf_GrSLType:
            case kHalf2_GrSLType:
            case kHalf3_GrSLType:
            case kHalf4_GrSLType:
            case kHalf2x2_GrSLType:
            case kHalf3x3_GrSLType:
            case kHalf4x4_GrSLType: {
                const float* floatBits = static_cast<const float*>(src);
                SkHalf* halfBits = static_cast<SkHalf*>(dest);
                while (numUniforms-- > 0) {
                    *halfBits++ = SkFloatToHalf(*floatBits++);
                }
                return 2;
            }

            case kShort_GrSLType:
            case kShort2_GrSLType:
            case kShort3_GrSLType:
            case kShort4_GrSLType:
            case kUShort_GrSLType:
            case kUShort2_GrSLType:
            case kUShort3_GrSLType:
            case kUShort4_GrSLType: {
                const int32_t* intBits = static_cast<const int32_t*>(src);
                short* shortBits = static_cast<short*>(dest);
                while (numUniforms-- > 0) {
                    *shortBits++ = (short)(*intBits++);
                }
                return 2;
            }

            default:
                // Fall through to memcpy below.
                break;
        }
    }

    memcpy(dest, src, numUniforms * 4);
    return 4;
}

template <int N, GrSLType FullType, GrSLType HalfType>
void GrUniformDataManager::set(UniformHandle u, const void* v) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    this->copyUniforms(buffer, v, N, uni.fType);
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
        this->copyUniforms(buffer, v, arrayCount * 4, uni.fType);
    } else {
        for (int i = 0; i < arrayCount; ++i) {
            int uniformSize = this->copyUniforms(buffer, v, N, uni.fType);
            buffer = SkTAddOffset<void>(buffer, /*numUniforms*/4 * uniformSize);
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
    this->setMatrices<2, kFloat2x2_GrSLType, kHalf2x2_GrSLType>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2, kFloat2x2_GrSLType, kHalf2x2_GrSLType>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3, kFloat3x3_GrSLType, kHalf3x3_GrSLType>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3, kFloat3x3_GrSLType, kHalf3x3_GrSLType>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4, kFloat4x4_GrSLType, kHalf4x4_GrSLType>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4, kFloat4x4_GrSLType, kHalf4x4_GrSLType>(u, arrayCount, m);
}

template <int N, GrSLType FullType, GrSLType HalfType>
inline void GrUniformDataManager::setMatrices(UniformHandle u,
                                              int arrayCount,
                                              const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == FullType || uni.fType == HalfType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    if constexpr (N == 4) {
        this->copyUniforms(buffer, matrices, arrayCount * 16, uni.fType);
    } else {
        for (int i = 0; i < arrayCount; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                int uniformSize = this->copyUniforms(buffer, &matrix[j * N], N, uni.fType);
                buffer = SkTAddOffset<void>(buffer, /*numUniforms*/4 * uniformSize);
            }
        }
    }
}
