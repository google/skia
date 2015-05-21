/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPrimitiveProcessor.h"

#include "builders/GrGLProgramBuilder.h"

SkMatrix GrGLPrimitiveProcessor::GetTransformMatrix(const SkMatrix& localMatrix,
                                                    const GrCoordTransform& coordTransform) {
    SkMatrix combined;
    // We only apply the localmatrix to localcoords
    if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
        combined.setConcat(coordTransform.getMatrix(), localMatrix);
    } else {
        combined = coordTransform.getMatrix();
    }
    if (coordTransform.reverseY()) {
        // combined.postScale(1,-1);
        // combined.postTranslate(0,1);
        combined.set(SkMatrix::kMSkewY,
            combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
        combined.set(SkMatrix::kMScaleY,
            combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
        combined.set(SkMatrix::kMTransY,
            combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
    }
    return combined;
}

void GrGLPrimitiveProcessor::setupUniformColor(GrGLGPBuilder* pb,
                                               const char* outputName,
                                               UniformHandle* colorUniform) {
    GrGLFragmentBuilder* fs = pb->getFragmentShaderBuilder();
    SkASSERT(colorUniform);
    const char* stagedLocalVarName;
    *colorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                   kVec4f_GrSLType,
                                   kDefault_GrSLPrecision,
                                   "Color",
                                   &stagedLocalVarName);
    fs->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
}
