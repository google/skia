/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLGeometryShaderBuilder.h"
#include "GrGLSLProgramBuilder.h"
#include "GrGLSLVarying.h"

static const char* input_type_name(GrGLSLGeometryBuilder::InputType in) {
    using InputType = GrGLSLGeometryBuilder::InputType;
    SkASSERT(in >= InputType::kPoints && in <= InputType::kTrianglesAdjacency);

    static const char* kInputTypeNames[] = {
        "points",
        "lines",
        "lines_adjacency",
        "triangles",
        "triangles_adjacency"
    };
    return kInputTypeNames[(int)in];

    GR_STATIC_ASSERT(0 == (int)InputType::kPoints);
    GR_STATIC_ASSERT(1 == (int)InputType::kLines);
    GR_STATIC_ASSERT(2 == (int)InputType::kLinesAdjacency);
    GR_STATIC_ASSERT(3 == (int)InputType::kTriangles);
    GR_STATIC_ASSERT(4 == (int)InputType::kTrianglesAdjacency);
}

static const char* output_type_name(GrGLSLGeometryBuilder::OutputType out) {
    using OutputType = GrGLSLGeometryBuilder::OutputType;
    SkASSERT(out >= OutputType::kPoints && out <= OutputType::kTriangleStrip);

    static const char* kInputTypeNames[] = {
        "points",
        "line_strip",
        "triangle_strip"
    };
    return kInputTypeNames[(int)out];

    GR_STATIC_ASSERT(0 == (int)OutputType::kPoints);
    GR_STATIC_ASSERT(1 == (int)OutputType::kLineStrip);
    GR_STATIC_ASSERT(2 == (int)OutputType::kTriangleStrip);
}

GrGLSLGeometryBuilder::GrGLSLGeometryBuilder(GrGLSLProgramBuilder* program)
    : INHERITED(program)
    , fIsConfigured(false) {
}

void GrGLSLGeometryBuilder::configure(InputType inputType, OutputType outputType, int maxVertices,
                                      int numInvocations) {
    SkASSERT(!fIsConfigured);
    this->addLayoutQualifier(input_type_name(inputType), kIn_InterfaceQualifier);
    this->addLayoutQualifier(SkStringPrintf("invocations = %i", numInvocations).c_str(),
                             kIn_InterfaceQualifier);
    this->addLayoutQualifier(output_type_name(outputType), kOut_InterfaceQualifier);
    this->addLayoutQualifier(SkStringPrintf("max_vertices = %i", maxVertices).c_str(),
                             kOut_InterfaceQualifier);
    fIsConfigured = true;
}

void GrGLSLGeometryBuilder::onFinalize() {
    SkASSERT(fIsConfigured);
    fProgramBuilder->varyingHandler()->getGeomDecls(&this->inputs(), &this->outputs());
}
