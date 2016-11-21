/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLVertexShader_DEFINED
#define GrGLSLVertexShader_DEFINED

#include "GrGLSLShaderBuilder.h"
#include "GrGeometryProcessor.h"

class GrGLSLVarying;

// Enough precision to represent 1 / 2048 accurately in printf
#define GR_SIGNIFICANT_POW2_DECIMAL_DIG 11

class GrGLSLVertexBuilder : public GrGLSLShaderBuilder {
public:
    GrGLSLVertexBuilder(GrGLSLProgramBuilder* program);

    void appendTransformToWindowSpace2D(const char* coord2D);
    void appendTransformToWindowSpace3D(const char* coord3D);

    void emitVertexPosition(const GrShaderVar& posVar);

private:

    void onFinalize() override;

    friend class GrGLProgramBuilder;

    typedef GrGLSLShaderBuilder INHERITED;
};

#endif
