/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLPrimitiveProcessor.h"

#include "GrCoordTransform.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

SkMatrix GrGLSLPrimitiveProcessor::GetTransformMatrix(const SkMatrix& localMatrix,
                                                      const GrCoordTransform& coordTransform) {
    SkMatrix combined;
    combined.setConcat(coordTransform.getMatrix(), localMatrix);
    if (coordTransform.normalize()) {
        SkASSERT(coordTransform.texture());
        combined.postIDiv(coordTransform.texture()->width(), coordTransform.texture()->height());
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

void GrGLSLPrimitiveProcessor::setupUniformColor(GrGLSLPPFragmentBuilder* fragBuilder,
                                                 GrGLSLUniformHandler* uniformHandler,
                                                 const char* outputName,
                                                 UniformHandle* colorUniform) {
    SkASSERT(colorUniform);
    const char* stagedLocalVarName;
    *colorUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kVec4f_GrSLType,
                                               kDefault_GrSLPrecision,
                                               "Color",
                                               &stagedLocalVarName);
    fragBuilder->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
}

//////////////////////////////////////////////////////////////////////////////

const GrCoordTransform* GrGLSLPrimitiveProcessor::FPCoordTransformHandler::nextCoordTransform() {
#ifdef SK_DEBUG
    SkASSERT(nullptr == fCurr || fAddedCoord);
    fAddedCoord = false;
    fCurr = fIter.next();
    return fCurr;
#else
    return fIter.next();
#endif
}
