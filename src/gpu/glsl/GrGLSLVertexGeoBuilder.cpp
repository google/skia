/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLVertexGeoBuilder.h"

#include "GrGLSLProgramBuilder.h"
#include "GrGLSLVarying.h"
#include "GrTypes.h"

void GrGLSLVertexGeoBuilder::emitNormalizedSkPosition(SkString* out, const char* devPos,
                                                      const char* rtAdjustName,
                                                      GrSLType devPosType) {
    if (this->getProgramBuilder()->desc()->header().fSnapVerticesToPixelCenters) {
        if (kFloat3_GrSLType == devPosType) {
            const char* p = devPos;
            out->appendf("{float2 _posTmp = float2(%s.x/%s.z, %s.y/%s.z);", p, p, p, p);
        } else {
            SkASSERT(kFloat2_GrSLType == devPosType);
            out->appendf("{float2 _posTmp = %s;", devPos);
        }
        out->appendf("_posTmp = floor(_posTmp) + half2(0.5, 0.5);"
                     "sk_Position = float4(_posTmp, 0, 1);}");
    } else if (kFloat3_GrSLType == devPosType) {
        out->appendf("sk_Position = float4(%s.x , %s.y, 0, %s.z);",
                     devPos, devPos, devPos);
    } else {
        SkASSERT(kFloat2_GrSLType == devPosType);
        out->appendf("sk_Position = float4(%s.x , %s.y, 0, 1);",
                     devPos, devPos);
    }
}

void GrGLSLVertexBuilder::onFinalize() {
    // We could have the GrGeometryProcessor do this, but its just easier to have it performed
    // here. If we ever need to set variable pointsize, then we can reinvestigate.
    if (this->getProgramBuilder()->desc()->header().fHasPointSize) {
        this->codeAppend("sk_PointSize = 1.0;");
    }
    fProgramBuilder->varyingHandler()->getVertexDecls(&this->inputs(), &this->outputs());
}

static const char* input_type_name(GrGLSLGeometryBuilder::InputType in) {
    using InputType = GrGLSLGeometryBuilder::InputType;
    switch (in) {
        case InputType::kPoints: return "points";
        case InputType::kLines: return "lines";
        case InputType::kLinesAdjacency: return "lines_adjacency";
        case InputType::kTriangles: return "triangles";
        case InputType::kTrianglesAdjacency: return "triangles_adjacency";
    }
    SK_ABORT("invalid input type");
    return "unknown_input";
}

static const char* output_type_name(GrGLSLGeometryBuilder::OutputType out) {
    using OutputType = GrGLSLGeometryBuilder::OutputType;
    switch (out) {
        case OutputType::kPoints: return "points";
        case OutputType::kLineStrip: return "line_strip";
        case OutputType::kTriangleStrip: return "triangle_strip";
    }
    SK_ABORT("invalid output type");
    return "unknown_output";
}

void GrGLSLGeometryBuilder::configure(InputType inputType, OutputType outputType, int maxVertices,
                                      int numInvocations) {
    SkASSERT(!this->isConfigured());
    fNumInvocations = numInvocations;
    this->addLayoutQualifier(input_type_name(inputType), kIn_InterfaceQualifier);
    this->addLayoutQualifier(SkStringPrintf("invocations = %i", numInvocations).c_str(),
                             kIn_InterfaceQualifier);
    this->addLayoutQualifier(output_type_name(outputType), kOut_InterfaceQualifier);
    this->addLayoutQualifier(SkStringPrintf("max_vertices = %i", maxVertices).c_str(),
                             kOut_InterfaceQualifier);
}

void GrGLSLGeometryBuilder::emitVertex(SkString* out, const char* devPos, const char* rtAdjustName,
                                       GrSLType devPosType) {
    this->emitNormalizedSkPosition(out, devPos, rtAdjustName, devPosType);
    out->append("EmitVertex();");
}

void GrGLSLGeometryBuilder::endPrimitive() {
    this->codeAppend("EndPrimitive();");
}

void GrGLSLGeometryBuilder::onFinalize() {
    SkASSERT(this->isConfigured());
    fProgramBuilder->varyingHandler()->getGeomDecls(&this->inputs(), &this->outputs());
}
