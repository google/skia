/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"

void GrGLSLVertexGeoBuilder::emitNormalizedSkPosition(SkString* out, const char* devPos,
                                                      SkSLType devPosType) {
    if (this->getProgramBuilder()->snapVerticesToPixelCenters()) {
        if (SkSLType::kFloat3 == devPosType) {
            const char* p = devPos;
            out->appendf("{float2 _posTmp = %s.xy / %s.z;", p, p);
        } else {
            SkASSERT(SkSLType::kFloat2 == devPosType);
            out->appendf("{float2 _posTmp = %s;", devPos);
        }
        out->appendf("_posTmp = floor(_posTmp) + float2(0.5);"
                     "sk_Position = _posTmp.xy01;}");
    } else if (SkSLType::kFloat3 == devPosType) {
        out->appendf("sk_Position = %s.xy0z;", devPos);
    } else {
        SkASSERT(SkSLType::kFloat2 == devPosType);
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
