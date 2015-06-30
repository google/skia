/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLProgramDataManager.h"
#include "gl/GrGLUniformHandle.h"
#include "gl/GrGLGpu.h"
#include "SkMatrix.h"

#define ASSERT_ARRAY_UPLOAD_IN_BOUNDS(UNI, COUNT) \
         SkASSERT(arrayCount <= uni.fArrayCount || \
                  (1 == arrayCount && GrGLShaderVar::kNonArray == uni.fArrayCount))

GrGLProgramDataManager::GrGLProgramDataManager(GrGLGpu* gpu, const UniformInfoArray& uniforms)
    : fGpu(gpu) {
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const UniformInfo& builderUniform = uniforms[i];
        SkASSERT(GrGLShaderVar::kNonArray == builderUniform.fVariable.getArrayCount() ||
                 builderUniform.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = builderUniform.fVariable.getArrayCount();
            uniform.fType = builderUniform.fVariable.getType();
        );
        // TODO: Move the Xoom uniform array in both FS and VS bug workaround here.

        if (GrGLProgramBuilder::kVertex_Visibility & builderUniform.fVisibility) {
            uniform.fVSLocation = builderUniform.fLocation;
        } else {
            uniform.fVSLocation = kUnusedUniform;
        }
        if (GrGLProgramBuilder::kFragment_Visibility & builderUniform.fVisibility) {
            uniform.fFSLocation = builderUniform.fLocation;
        } else {
            uniform.fFSLocation = kUnusedUniform;
        }
    }
}

void GrGLProgramDataManager::setSampler(UniformHandle u, GrGLint texUnit) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kSampler2D_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    // FIXME: We still insert a single sampler uniform for every stage. If the shader does not
    // reference the sampler then the compiler may have optimized it out. Uncomment this assert
    // once stages insert their own samplers.
    // this->printUnused(uni);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fFSLocation, texUnit));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1i(uni.fVSLocation, texUnit));
    }
}

void GrGLProgramDataManager::set1f(UniformHandle u, GrGLfloat v0) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fFSLocation, v0));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1f(uni.fVSLocation, v0));
    }
}

void GrGLProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    // This assert fires in some instances of the two-pt gradient for its VSParams.
    // Once the uniform manager is responsible for inserting the duplicate uniform
    // arrays in VS and FS driver bug workaround, this can be enabled.
    // this->printUni(uni);
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform1fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set2f(UniformHandle u, GrGLfloat v0, GrGLfloat v1) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fFSLocation, v0, v1));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2f(uni.fVSLocation, v0, v1));
    }
}

void GrGLProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform2fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set3f(UniformHandle u, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fFSLocation, v0, v1, v2));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3f(uni.fVSLocation, v0, v1, v2));
    }
}

void GrGLProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform3fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::set4f(UniformHandle u,
                                   GrGLfloat v0,
                                   GrGLfloat v1,
                                   GrGLfloat v2,
                                   GrGLfloat v3) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fFSLocation, v0, v1, v2, v3));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4f(uni.fVSLocation, v0, v1, v2, v3));
    }
}

void GrGLProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const GrGLfloat v[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fFSLocation, arrayCount, v));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), Uniform4fv(uni.fVSLocation, arrayCount, v));
    }
}

void GrGLProgramDataManager::setMatrix3f(UniformHandle u, const GrGLfloat matrix[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kMat33f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix3fv(uni.fFSLocation, 1, false, matrix));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix3fv(uni.fVSLocation, 1, false, matrix));
    }
}

void GrGLProgramDataManager::setMatrix4f(UniformHandle u, const GrGLfloat matrix[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kMat44f_GrSLType);
    SkASSERT(GrGLShaderVar::kNonArray == uni.fArrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix4fv(uni.fFSLocation, 1, false, matrix));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(), UniformMatrix4fv(uni.fVSLocation, 1, false, matrix));
    }
}

void GrGLProgramDataManager::setMatrix3fv(UniformHandle u,
                                          int arrayCount,
                                          const GrGLfloat matrices[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kMat33f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix3fv(uni.fFSLocation, arrayCount, false, matrices));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix3fv(uni.fVSLocation, arrayCount, false, matrices));
    }
}

void GrGLProgramDataManager::setMatrix4fv(UniformHandle u,
                                          int arrayCount,
                                          const GrGLfloat matrices[]) const {
    const Uniform& uni = fUniforms[u.toProgramDataIndex()];
    SkASSERT(uni.fType == kMat44f_GrSLType);
    SkASSERT(arrayCount > 0);
    ASSERT_ARRAY_UPLOAD_IN_BOUNDS(uni, arrayCount);
    SkDEBUGCODE(this->printUnused(uni);)
    if (kUnusedUniform != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix4fv(uni.fFSLocation, arrayCount, false, matrices));
    }
    if (kUnusedUniform != uni.fVSLocation && uni.fVSLocation != uni.fFSLocation) {
        GR_GL_CALL(fGpu->glInterface(),
                   UniformMatrix4fv(uni.fVSLocation, arrayCount, false, matrices));
    }
}

void GrGLProgramDataManager::setSkMatrix(UniformHandle u, const SkMatrix& matrix) const {
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

#ifdef SK_DEBUG
void GrGLProgramDataManager::printUnused(const Uniform& uni) const {
    if (kUnusedUniform == uni.fFSLocation && kUnusedUniform == uni.fVSLocation) {
        GrCapsDebugf(fGpu->caps(), "Unused uniform in shader\n");
    }
}
#endif
