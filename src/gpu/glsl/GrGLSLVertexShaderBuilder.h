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

class GrGLSLVertexBuilder : public GrGLSLShaderBuilder {
public:
    GrGLSLVertexBuilder(GrGLSLProgramBuilder* program);

    void transformToNormalizedDeviceSpace(const GrShaderVar& posVar);
private:
    void onFinalize() override;
    
    const char* fRtAdjustName;

    friend class GrGLProgramBuilder;

    typedef GrGLSLShaderBuilder INHERITED;
};

#endif
