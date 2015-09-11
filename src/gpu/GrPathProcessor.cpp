/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrPathProcessor.h"

#include "gl/GrGLPathProcessor.h"
#include "gl/GrGLGpu.h"

#include "glsl/GrGLSLCaps.h"

GrPathProcessor::GrPathProcessor(GrColor color,
                                 const GrPipelineOptimizations& opts,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix)
    : INHERITED(true)
    , fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(localMatrix)
    , fOpts(opts) {
    this->initClassID<GrPathProcessor>();
}

void GrPathProcessor::getGLProcessorKey(const GrGLSLCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLPathProcessor::GenKey(*this, caps, b);
}

GrGLPrimitiveProcessor* GrPathProcessor::createGLInstance(const GrGLSLCaps& caps) const {
    SkASSERT(caps.pathRenderingSupport());
    return new GrGLPathProcessor();
}
