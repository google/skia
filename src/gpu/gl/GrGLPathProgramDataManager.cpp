 /*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLPathProgramDataManager.h"
#include "gl/GrGLPathRendering.h"
#include "gl/GrGLUniformHandle.h"
#include "gl/GrGLGpu.h"
#include "SkMatrix.h"

GrGLPathProgramDataManager::GrGLPathProgramDataManager(
    GrGLGpu* gpu, GrGLuint programID, const SeparableVaryingInfoArray& separableVaryings)
    : fGpu(gpu)
    , fProgramID(programID) {
    int count = separableVaryings.count();
    fSeparableVaryings.push_back_n(count);
    for (int i = 0; i < count; i++) {
        SeparableVarying& separableVarying = fSeparableVaryings[i];
        const SeparableVaryingInfo& builderSeparableVarying = separableVaryings[i];
        SkASSERT(GrGLShaderVar::kNonArray == builderSeparableVarying.fVariable.getArrayCount() ||
                 builderSeparableVarying.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            separableVarying.fArrayCount = builderSeparableVarying.fVariable.getArrayCount();
            separableVarying.fType = builderSeparableVarying.fVariable.getType();
        );
        separableVarying.fLocation = builderSeparableVarying.fLocation;
    }
}

void GrGLPathProgramDataManager::setPathFragmentInputTransform(SeparableVaryingHandle u,
                                                               int components,
                                                               const SkMatrix& matrix) const {
    const SeparableVarying& fragmentInput =
            fSeparableVaryings[u.toProgramDataIndex()];

    SkASSERT((components == 2 && fragmentInput.fType == kVec2f_GrSLType) ||
              (components == 3 && fragmentInput.fType == kVec3f_GrSLType));

    fGpu->glPathRendering()->setProgramPathFragmentInputTransform(fProgramID,
                                                                  fragmentInput.fLocation,
                                                                  GR_GL_OBJECT_LINEAR,
                                                                  components,
                                                                  matrix);
}

