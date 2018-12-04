/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLPrimitiveProcessor.h"

#include "GrCoordTransform.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

SkMatrix GrGLSLPrimitiveProcessor::GetTransformMatrix(const SkMatrix& localMatrix,
                                                      const GrCoordTransform& coordTransform) {
    SkMatrix combined;
    combined.setConcat(coordTransform.getMatrix(), localMatrix);
    if (coordTransform.normalize()) {
        combined.postIDiv(coordTransform.peekTexture()->width(),
                          coordTransform.peekTexture()->height());
    }

    if (coordTransform.reverseY()) {
        if (coordTransform.normalize()) {
            // combined.postScale(1,-1);
            // combined.postTranslate(0,1);
            combined.set(SkMatrix::kMSkewY,
                         combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
            combined.set(SkMatrix::kMScaleY,
                         combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
            combined.set(SkMatrix::kMTransY,
                         combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
        } else {
            // combined.postScale(1, -1);
            // combined.postTranslate(0,1);
            SkScalar h = coordTransform.peekTexture()->height();
            combined.set(SkMatrix::kMSkewY,
                         h * combined[SkMatrix::kMPersp0] - combined[SkMatrix::kMSkewY]);
            combined.set(SkMatrix::kMScaleY,
                         h * combined[SkMatrix::kMPersp1] - combined[SkMatrix::kMScaleY]);
            combined.set(SkMatrix::kMTransY,
                         h * combined[SkMatrix::kMPersp2] - combined[SkMatrix::kMTransY]);
        }
    }
    return combined;
}

void GrGLSLPrimitiveProcessor::setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                 GrGLSLUniformHandler* uniformHandler,
                                                 const char* outputName,
                                                 UniformHandle* colorUniform) {
    SkASSERT(colorUniform);
    const char* stagedLocalVarName;
    *colorUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kHalf4_GrSLType,
                                               "Color",
                                               &stagedLocalVarName);
    fragBuilder->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
    if (fragBuilder->getProgramBuilder()->shaderCaps()->mustObfuscateUniformColor()) {
        fragBuilder->codeAppendf("%s = max(%s, half4(0, 0, 0, 0));", outputName, outputName);
    }
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
