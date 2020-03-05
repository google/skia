/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

SkMatrix GrGLSLPrimitiveProcessor::GetTransformMatrix(const GrCoordTransform& coordTransform,
                                                      const SkMatrix& preMatrix) {
    SkMatrix combined;
    combined.setConcat(coordTransform.matrix(), preMatrix);
    if (coordTransform.normalize()) {
        SkMatrixPriv::PostIDiv(&combined, coordTransform.peekTexture()->width(),
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

GrGLSLPrimitiveProcessor::FPCoordTransformHandler::FPCoordTransformHandler(
        const GrPipeline& pipeline, SkTArray<TransformVar>* transformedCoordVars)
        : fIter(pipeline), fTransformedCoordVars(transformedCoordVars) {}

std::pair<const GrCoordTransform&, const GrFragmentProcessor&>
GrGLSLPrimitiveProcessor::FPCoordTransformHandler::get() const {
    return *fIter;
}

GrGLSLPrimitiveProcessor::FPCoordTransformHandler&
GrGLSLPrimitiveProcessor::FPCoordTransformHandler::operator++() {
    SkASSERT(fAddedCoord);
    ++fIter;
    SkDEBUGCODE(fAddedCoord = false;)
    return *this;
}
