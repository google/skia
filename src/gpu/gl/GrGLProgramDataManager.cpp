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
        );
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
        );
        pathProcVarying.fLocation = builderPathProcVarying.fLocation;
    }
}

void GrGLProgramDataManager::setSamplers(const UniformInfoArray& samplers) const {
    for (int i = 0; i < samplers.count(); ++i) {
        const UniformInfo& sampler = samplers[i];
        SkASSERT(sampler.fVisibility);
        if (kUnusedUniform != sampler.fLocation) {
            GR_GL_CALL(fGpu->glInterface(), Uniform1i(sampler.fLocation, i));
        }
    }
}

void GrGLProgramDataManager::setImageStorages(const UniformInfoArray& images) const {
    for (int i = 0; i < images.count(); ++i) {
        const UniformInfo& image = images[i];
        SkASSERT(image.fVisibility);
        if (kUnusedUniform != image.fLocation) {
            GR_GL_CALL(fGpu->glInterface(), Uniform1i(image.fLocation, i));
        }
    }
}

void GrGLProgramDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fLocation, i));
    }
}

void GrGLProgramDataManager::set1iv(UniformHandle u,
                                    int arrayCount,
                                    const int v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1iv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fLocation, v0));
    }
}

void GrGLProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
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

void GrGLProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fLocation, v0, v1));
    }
}

void GrGLProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fLocation, v0, v1, v2));
    }
}

void GrGLProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4f(UniformHandle u,
                                   float v0,
                                   float v1,
                                   float v2,
                                   float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    if (kUnusedUniform != uni.fLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fLocation, v0, v1, v2, v3));
    }
}

void GrGLProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
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
    SkASSERT(uni.fType == kMat22f_GrSLType + (N - 2));
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

    SkASSERT((components == 2 && fragmentInput.fType == kVec2f_GrSLType) ||
              (components == 3 && fragmentInput.fType == kVec3f_GrSLType));

    fGpu->glPathRendering()->setProgramPathFragmentInputTransform(fProgramID,
                                                                  fragmentInput.fLocation,
                                                                  GR_GL_OBJECT_LINEAR,
                                                                  components,
                                                                  matrix);
}
