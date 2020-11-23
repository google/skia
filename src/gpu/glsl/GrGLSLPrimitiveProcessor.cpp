/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

void GrGLSLPrimitiveProcessor::setupUniformColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                 GrGLSLUniformHandler* uniformHandler,
                                                 const char* outputName,
                                                 UniformHandle* colorUniform) {
    SkASSERT(colorUniform);
    const char* stagedLocalVarName;
    *colorUniform = uniformHandler->addUniform(nullptr,
                                               kFragment_GrShaderFlag,
                                               kHalf4_GrSLType,
                                               "Color",
                                               &stagedLocalVarName);
    fragBuilder->codeAppendf("%s = %s;", outputName, stagedLocalVarName);
    if (fragBuilder->getProgramBuilder()->shaderCaps()->mustObfuscateUniformColor()) {
        fragBuilder->codeAppendf("%s = max(%s, half4(0));", outputName, outputName);
    }
}

//////////////////////////////////////////////////////////////////////////////

GrGLSLPrimitiveProcessor::FPCoordTransformHandler::FPCoordTransformHandler(
        const GrPipeline& pipeline, SkTArray<GrShaderVar>* transformedCoordVars)
        : fIter(pipeline), fTransformedCoordVars(transformedCoordVars) {
    while (fIter && !fIter->usesVaryingCoordsDirectly()) {
        ++fIter;
    }
}

const GrFragmentProcessor& GrGLSLPrimitiveProcessor::FPCoordTransformHandler::get() const {
    return *fIter;
}

GrGLSLPrimitiveProcessor::FPCoordTransformHandler&
GrGLSLPrimitiveProcessor::FPCoordTransformHandler::operator++() {
    SkASSERT(fAddedCoord);
    do {
        ++fIter;
    } while (fIter && !fIter->usesVaryingCoordsDirectly());
    SkDEBUGCODE(fAddedCoord = false;)
    return *this;
}
