/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryShaderBuilder_DEFINED
#define GrGLGeometryShaderBuilder_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLSLVarying;

class GrGLGeometryBuilder : public GrGLShaderBuilder {
public:
    GrGLGeometryBuilder(GrGLSLProgramBuilder* program);

private:
    /*
     * an internal call for GrGLFullProgramBuilder to add varyings
     */
    void addVarying(const char* name, GrSLPrecision precision, GrGLSLVarying*);

    void onFinalize() override {}

    friend class GrGLProgramBuilder;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
