/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLUniformHandle.h"
#include "gl/GrGpuGL.h"
#include "SkMatrix.h"

#define ASSERT_ARRAY_UPLOAD_IN_BOUNDS(UNI, COUNT) \
         SkASSERT(arrayCount <= uni.fArrayCount || \
                  (1 == arrayCount && GrGLShaderVar::kNonArray == uni.fArrayCount))

GrGLUniformManager::GrGLUniformManager(GrGpuGL* gpu) : fGpu(gpu) {
    fUsingBindUniform = fGpu->glInterface()->fBindUniformLocation != NULL;
}

GrGLUniformManager::UniformHandle GrGLUniformManager::appendUniform(GrSLType type, int arrayCount) {
    int idx = fUniforms.count();
    Uniform& uni = fUniforms.push_back();
    SkASSERT(GrGLShaderVar::kNonArray == arrayCount || arrayCount > 0);
    uni.fArrayCount = arrayCount;
    uni.fType = type;
    uni.fVSLocation = kUnusedUniform;
    uni.fFSLocation = kUnusedUniform;
    return GrGLUniformManager::UniformHandle::CreateFromUniformIndex(idx);
}

void GrGLUniformManager::setSampler(UniformHandle u, GrGLint texUnit) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kSampler2D_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    // FIXME: We still insert a single sampler uniform for every stage. If the shader does not
    // reference the sampler then the compiler may have optimized it out. Uncomment this assert
    // once stages insert their own samplers.
    // SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fFSLocation, texUnit));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fVSLocation, texUnit));
    }
}

void GrGLUniformManager::set1f(UniformHandle u, GrGLfloat v0) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fFSLocation, v0));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fVSLocation, v0));
    }
}

void GrGLUniformManager::set1fv(UniformHandle u,
                                int arrayCount,
                                const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    // This assert fires in some instances of the two-pt gradient for its VSParams.
    // Once the uniform manager is responsible for inserting the duplicate uniform
    // arrays in VS and FS driver bug workaround, this can be enabled.
    //SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLUniformManager::set2f(UniformHandle u, GrGLfloat v0, GrGLfloat v1) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fFSLocation, v0, v1));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fVSLocation, v0, v1));
    }
}

void GrGLUniformManager::set2fv(UniformHandle u,
                                int arrayCount,
                                const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLUniformManager::set3f(UniformHandle u, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fFSLocation, v0, v1, v2));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fVSLocation, v0, v1, v2));
    }
}

void GrGLUniformManager::set3fv(UniformHandle u,
                                int arrayCount,
                                const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLUniformManager::set4f(UniformHandle u,
                               GrGLfloat v0,
                               GrGLfloat v1,
                               GrGLfloat v2,
                               GrGLfloat v3) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fFSLocation, v0, v1, v2, v3));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fVSLocation, v0, v1, v2, v3));
    }
}

void GrGLUniformManager::set4fv(UniformHandle u,
                                int arrayCount,
                                const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLUniformManager::setMatrix3f(UniformHandle u, const GrGLfloat matrix[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kMat33f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    // TODO: Re-enable this assert once texture matrices aren't forced on all effects
    // SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix3fv(uni.fFSLocation, 1, false, matrix));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix3fv(uni.fVSLocation, 1, false, matrix));
    }
}

void GrGLUniformManager::setMatrix4f(UniformHandle u, const GrGLfloat matrix[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kMat44f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix4fv(uni.fFSLocation, 1, false, matrix));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix4fv(uni.fVSLocation, 1, false, matrix));
    }
}

void GrGLUniformManager::setMatrix3fv(UniformHandle u,
                                      int arrayCount,
                                      const GrGLfloat matrices[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kMat33f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix3fv(uni.fFSLocation, arrayCount, false, matrices));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix3fv(uni.fVSLocation, arrayCount, false, matrices));
    }
}

void GrGLUniformManager::setMatrix4fv(UniformHandle u,
                                      int arrayCount,
                                      const GrGLfloat matrices[]) const {
    const Uniform& uni = fUniforms[u.toUniformIndex()];
    SkASSERT(uni.fType == kMat44f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkASSERT(kUnusedUniform != uni.fFSLocation || kUnusedUniform != uni.fVSLocation);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix4fv(uni.fFSLocation, arrayCount, false, matrices));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix4fv(uni.fVSLocation, arrayCount, false, matrices));
    }
}

void GrGLUniformManager::setSkMatrix(UniformHandle u, const SkMatrix& matrix) const {
//    GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
    GrGLfloat mt[] = {
        matrix.get(SkMatrix::kMScaleX),
        matrix.get(SkMatrix::kMSkewY),
        matrix.get(SkMatrix::kMPersp0),
        matrix.get(SkMatrix::kMSkewX),
        matrix.get(SkMatrix::kMScaleY),
        matrix.get(SkMatrix::kMPersp1),
        matrix.get(SkMatrix::kMTransX),
        matrix.get(SkMatrix::kMTransY),
        matrix.get(SkMatrix::kMPersp2),
    };
    this->setMatrix3f(u, mt);
}


void GrGLUniformManager::getUniformLocations(GrGLuint programID, const BuilderUniformArray& uniforms) {
    SkASSERT(uniforms.count() == fUniforms.count());
    int count = fUniforms.count();
    for (int i = 0; i < count; ++i) {
        SkASSERT(uniforms[i].fVariable.getType() == fUniforms[i].fType);
        SkASSERT(uniforms[i].fVariable.getArrayCount() == fUniforms[i].fArrayCount);
        GrGLint location;
        // TODO: Move the Xoom uniform array in both FS and VS bug workaround here.
        if (fUsingBindUniform) {
            location = i;
            GR_GL_CALL(fGpu->glInterface(),
                       BindUniformLocation(programID, location, uniforms[i].fVariable.c_str()));
        } else {
            GR_GL_CALL_RET(fGpu->glInterface(), location,
                       GetUniformLocation(programID, uniforms[i].fVariable.c_str()));
        }
        if (GrGLShaderBuilder::kVertex_Visibility & uniforms[i].fVisibility) {
            fUniforms[i].fVSLocation = location;
        }
        if (GrGLShaderBuilder::kFragment_Visibility & uniforms[i].fVisibility) {
            fUniforms[i].fFSLocation = location;
        }
    }
}

const GrGLUniformManager::BuilderUniform&
GrGLUniformManager::getBuilderUniform(const BuilderUniformArray& array, UniformHandle handle) const {
    return array[handle.toUniformIndex()];
}
