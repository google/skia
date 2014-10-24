/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryShaderBuilder_DEFINED
#define GrGLGeometryShaderBuilder_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLGeometryBuilder : public GrGLShaderBuilder {
public:
    GrGLGeometryBuilder(GrGLProgramBuilder* program);
    /*
     * an internal call for GrGLFullProgramBuilder to add varyings
     */
    void addVarying(GrSLType type,
                   const char* name,
                   const char** gsOutName);

    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
