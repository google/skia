/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/GrGLGpu.h"
#include "glsl/GrGLSLUniformHandler.h"

#define ASSERT_ARRAY_UPLOAD_IN_BOUNDS(UNI, COUNT) \
         SkASSERT((COUNT) <= (UNI).fArrayCount || \
                  (1 == (COUNT) && GrShaderVar::kNonArray == (UNI).fArrayCount))

GrGLProgramDataManager::GrGLProgramDataManager(GrGLGpu* gpu, GrGLuint programID,
                                               const UniformInfoArray& uniforms,
                                               const VaryingInfoArray& pathProcVaryings)
    : fGpu(gpu)
    , fProgramID(programID) {
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const UniformInfo& builderUniform = uniforms[i];
        SkASSERT(GrShaderVar::kNonArray == builderUniform.fVariable.getArrayCount() ||
                 builderUniform.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = builderUniform.fVariable.getArrayCount();
            uniform.fType = builderUniform.fVariable.getType();
        )
        uniform.fLocation = builderUniform.fLocation;
    }

    // NVPR programs have separable varyings
    count = pathProcVaryings.count();
    fPathProcVaryings.push_back_n(count);
    for (int i = 0; i < count; i++) {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        PathProcVarying& pathProcVarying = fPathProcVaryings[i];
        const VaryingInfo& builderPathProcVarying = pathProcVaryings[i];
        SkASSERT(GrShaderVar::kNonArray == builderPathProcVarying.fVariable.getArrayCount() ||
                 builderPathProcVarying.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            pathProcVarying.fArrayCount = builderPathProcVarying.fVariable.getArrayCount();
            pathProcVarying.fType = builderPathProcVarying.fVariable.getType();
        )
        pathProcVarying.fLocation = builderPathProcVarying.fLocation;
    }
}

void GrGLProgramDataManager::setSamplerUniforms(const UniformInfoArray& samplers,
                                                int startUnit) const {
    for (int i = 0; i < samplers.count(); ++i) {
        const UniformInfo& sampler = samplers[i];
        SkASSERT(sampler.fVisibility);
        if (kUnusedUniform != sampler.fLocation) {
            GR_GL_CALL(fGpu->glInterface(), Uniform1i(sampler.fLocation, i + startUnit));
        }
    }
}

void GrGLProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fLocation, i));
    }
}

void GrGLProgramDataManager::set1iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fLocation, v0));
    }
}

void GrGLProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
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
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2i(uni.fLocation, i0, i1));
    }
}

void GrGLProgramDataManager::set2iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fLocation, v0, v1));
    }
}

void GrGLProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3i(UniformHandle u, int32_t i0, int32_t i1, int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3i(uni.fLocation, i0, i1, i2));
    }
}

void GrGLProgramDataManager::set3iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fLocation, v0, v1, v2));
    }
}

void GrGLProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
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
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4i(uni.fLocation, i0, i1, i2, i3));
    }
}

void GrGLProgramDataManager::set4iv(UniformHandle u,
                                    int arrayCount,
                                    const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
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
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fLocation, v0, v1, v2, v3));
    }
}

void GrGLProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
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
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
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

void GrGLProgramDataManager::setPathFragmentInputTransform(VaryingHandle u,
                                                           int components,
                                                           const SkMatrix& matrix) const {
    SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
    const PathProcVarying& fragmentInput = fPathProcVaryings[u.toIndex()];

    SkASSERT((components == 2 && (fragmentInput.fType == kFloat2_GrSLType ||
                                  fragmentInput.fType == kHalf2_GrSLType)) ||
              (components == 3 && (fragmentInput.fType == kFloat3_GrSLType ||
                                   fragmentInput.fType == kHalf3_GrSLType)));

    fGpu->glPathRendering()->setProgramPathFragmentInputTransform(fProgramID,
                                                                  fragmentInput.fLocation,
                                                                  GR_GL_OBJECT_LINEAR,
                                                                  components,
                                                                  matrix);
}
