/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryShaderBuilder_DEFINED
#define GrGLGeometryShaderBuilder_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLProgramBuilder;

class GrGLGeometryShaderBuilder : public GrGLFullShaderBuilder {
public:
    GrGLGeometryShaderBuilder(GrGLFullProgramBuilder* program);
private:
    /*
     * an internal call for GrGLFullProgramBuilder to add varyings
     */
    void addVarying(GrSLType type,
                   const char* name,
                   const char** gsOutName);

    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;

    friend class GrGLFullProgramBuilder;
    typedef GrGLFullShaderBuilder INHERITED;
};

#endif
