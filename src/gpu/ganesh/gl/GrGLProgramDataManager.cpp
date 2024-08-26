/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLProgramDataManager.h"

#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#define ASSERT_ARRAY_UPLOAD_IN_BOUNDS(UNI, COUNT) \
         SkASSERT((COUNT) <= (UNI).fArrayCount || \
                  (1 == (COUNT) && GrShaderVar::kNonArray == (UNI).fArrayCount))

static constexpr GrGLint kUnusedUniform = -1;

GrGLProgramDataManager::GrGLProgramDataManager(GrGLGpu* gpu, const UniformInfoArray& uniforms)
        : fGpu(gpu) {
    fUniforms.push_back_n(uniforms.count());
    int i = 0;
    for (const GLUniformInfo& builderUniform : uniforms.items()) {
        Uniform& uniform = fUniforms[i++];
        SkASSERT(GrShaderVar::kNonArray == builderUniform.fVariable.getArrayCount() ||
                 builderUniform.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = builderUniform.fVariable.getArrayCount();
            uniform.fType = builderUniform.fVariable.getType();
        )
        uniform.fLocation = builderUniform.fLocation;
    }
}

void GrGLProgramDataManager::setSamplerUniforms(const UniformInfoArray& samplers,
                                                int startUnit) const {
    int i = 0;
    for (const GLUniformInfo& sampler : samplers.items()) {
        SkASSERT(sampler.fVisibility);
        if (kUnusedUniform != sampler.fLocation) {
            GR_GL_CALL(fGpu->glInterface(), Uniform1i(sampler.fLocation, i + startUnit));
        }
        ++i;
    }
}

void GrGLProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt || uni.fType == SkSLType::kShort);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fLocation, i));
    }
}

void GrGLProgramDataManager::set1iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt || uni.fType == SkSLType::kShort);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat || uni.fType == SkSLType::kHalf);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fLocation, v0));
    }
}

void GrGLProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat || uni.fType == SkSLType::kHalf);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    // This assert fires in some instances of the two-pt gradient for its VSParams.
    // Once the uniform manager is responsible for inserting the duplicate uniform
    // arrays in VS and FS driver bug workaround, this can be enabled.
    // this->printUni(uni);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt2 || uni.fType == SkSLType::kShort2);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2i(uni.fLocation, i0, i1));
    }
}

void GrGLProgramDataManager::set2iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt2 || uni.fType == SkSLType::kShort2);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat2 || uni.fType == SkSLType::kHalf2);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fLocation, v0, v1));
    }
}

void GrGLProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat2 || uni.fType == SkSLType::kHalf2);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3i(UniformHandle u, int32_t i0, int32_t i1, int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt3 || uni.fType == SkSLType::kShort3);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3i(uni.fLocation, i0, i1, i2));
    }
}

void GrGLProgramDataManager::set3iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt3 || uni.fType == SkSLType::kShort3);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat3 || uni.fType == SkSLType::kHalf3);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fLocation, v0, v1, v2));
    }
}

void GrGLProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat3 || uni.fType == SkSLType::kHalf3);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4i(UniformHandle u,
                                   int32_t i0,
                                   int32_t i1,
                                   int32_t i2,
                                   int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt4 || uni.fType == SkSLType::kShort4);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4i(uni.fLocation, i0, i1, i2, i3));
    }
}

void GrGLProgramDataManager::set4iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kInt4 || uni.fType == SkSLType::kShort4);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4f(UniformHandle u,
                                   float v0,
                                   float v1,
                                   float v2,
                                   float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat4 || uni.fType == SkSLType::kHalf4);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fLocation, v0, v1, v2, v3));
    }
}

void GrGLProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == SkSLType::kFloat4 || uni.fType == SkSLType::kHalf4);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrGLProgramDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrGLProgramDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrGLProgramDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrGLProgramDataManager::setMatrices(UniformHandle u,
                                                                int arrayCount,
                                                                const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(static_cast<int>(uni.fType) == static_cast<int>(SkSLType::kFloat2x2) + (N - 2) ||
             static_cast<int>(uni.fType) == static_cast<int>(SkSLType::kHalf2x2) + (N - 2));
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        set_uniform_matrix<N>::set(fGpu->glInterface(), uni.fLocation, arrayCount, matrices);
    }
}

template<> struct set_uniform_matrix<2> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix2fv(loc, cnt, false, m));
    }
};

template<> struct set_uniform_matrix<3> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix3fv(loc, cnt, false, m));
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(const GrGLInterface* gli, const GrGLint loc, int cnt, const float m[]) {
        GR_GL_CALL(gli, UniformMatrix4fv(loc, cnt, false, m));
    }
};
