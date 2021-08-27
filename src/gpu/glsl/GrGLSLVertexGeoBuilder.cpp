/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

#include "include/gpu/GrTypes.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

void GrGLSLVertexGeoBuilder::emitNormalizedSkPosition(SkString* out, const char* devPos,
                                                      GrSLType devPosType) {
    if (this->getProgramBuilder()->snapVerticesToPixelCenters()) {
        if (kFloat3_GrSLType == devPosType) {
            const char* p = devPos;
            out->appendf("{float2 _posTmp = %s.xy / %s.z;", p, p);
        } else {
            SkASSERT(kFloat2_GrSLType == devPosType);
            out->appendf("{float2 _posTmp = %s;", devPos);
        }
        out->appendf("_posTmp = floor(_posTmp) + float2(0.5);"
                     "sk_Position = _posTmp.xy01;}");
    } else if (kFloat3_GrSLType == devPosType) {
        out->appendf("sk_Position = %s.xy0z;", devPos);
    } else {
        SkASSERT(kFloat2_GrSLType == devPosType);
        out->appendf("sk_Position = %s.xy01;", devPos);
    }
}

void GrGLSLVertexBuilder::onFinalize() {
    // We could have the GrGeometryProcessor do this, but its just easier to have it performed
    // here. If we ever need to set variable pointsize, then we can reinvestigate.
    if (this->getProgramBuilder()->hasPointSize()) {
        this->codeAppend("sk_PointSize = 1.0;");
    }
    fProgramBuilder->varyingHandler()->getVertexDecls(&this->inputs(), &this->outputs());
}
