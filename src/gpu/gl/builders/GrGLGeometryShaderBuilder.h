/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryShaderBuilder_DEFINED
#define GrGLGeometryShaderBuilder_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLVarying;

class GrGLGeometryBuilder : public GrGLShaderBuilder {
public:
    GrGLGeometryBuilder(GrGLProgramBuilder* program);

private:
    /*
     * an internal call for GrGLFullProgramBuilder to add varyings
     */
    void addVarying(const char* name, GrGLVarying*);

    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds);

    friend class GrGLProgramBuilder;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
