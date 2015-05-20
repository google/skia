/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexShader_DEFINED
#define GrGLVertexShader_DEFINED

#include "GrGLShaderBuilder.h"
#include "GrGeometryProcessor.h"

class GrGLVarying;

class GrGLVertexBuilder : public GrGLShaderBuilder {
public:
    GrGLVertexBuilder(GrGLProgramBuilder* program);

    void transformToNormalizedDeviceSpace(const GrShaderVar& posVar);
    void emitAttributes(const GrGeometryProcessor& gp);

    void addAttribute(const GrGeometryProcessor::Attribute* attr) {
        this->addAttribute(GrShaderVar(attr->fName,
                                       GrVertexAttribTypeToSLType(attr->fType),
                                       GrShaderVar::kAttribute_TypeModifier,
                                       GrShaderVar::kNonArray,
                                       attr->fPrecision));
    }

private:
    /*
     * Internal call for GrGLProgramBuilder.addVarying
     */
    void addVarying(const char* name, GrGLVarying*);

    /*
     * private helpers for compilation by GrGLProgramBuilder
     */
    void bindVertexAttributes(GrGLuint programID);
    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds);

    // an internal call which checks for uniquness of a var before adding it to the list of inputs
    bool addAttribute(const GrShaderVar& var);

    const char* fRtAdjustName;

    friend class GrGLProgramBuilder;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
