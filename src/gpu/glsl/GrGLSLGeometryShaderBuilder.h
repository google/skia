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

private:
    void onFinalize() override;

    friend class GrGLProgramBuilder;

    typedef GrGLSLShaderBuilder INHERITED;
};

#endif
