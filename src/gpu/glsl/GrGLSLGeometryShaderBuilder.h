/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLGeometryShaderBuilder_DEFINED
#define GrGLSLGeometryShaderBuilder_DEFINED

#include "GrGLSLShaderBuilder.h"

class GrGLSLVarying;

class GrGLSLGeometryBuilder : public GrGLSLShaderBuilder {
public:
    GrGLSLGeometryBuilder(GrGLSLProgramBuilder* program);

    enum class InputType {
        kPoints,
        kLines,
        kLinesAdjacency,
        kTriangles,
        kTrianglesAdjacency
    };

    enum class OutputType {
        kPoints,
        kLineStrip,
        kTriangleStrip
    };

    void configure(InputType, OutputType, int maxVertices, int numInvocations = 1);
    bool isConfigured() const { return fNumInvocations; }

private:
    void onFinalize() override;

    int fNumInvocations;

    friend class GrGLProgramBuilder;

    typedef GrGLSLShaderBuilder INHERITED;
};

#endif
