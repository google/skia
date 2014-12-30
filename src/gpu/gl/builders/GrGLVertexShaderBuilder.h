/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexShader_DEFINED
#define GrGLVertexShader_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLVarying;

class GrGLVertexBuilder : public GrGLShaderBuilder {
public:
    GrGLVertexBuilder(GrGLProgramBuilder* program);

    /** returns the expected position output */
    const char* glPosition() const { return "pos3"; }
    const char* positionCoords() const { return "position"; }
    const char* localCoords() const { return "localCoords"; }

    void addAttribute(const GrGeometryProcessor::GrAttribute* attr) {
        this->addAttribute(GrShaderVar(attr->fName,
                                       GrVertexAttribTypeToSLType(attr->fType),
                                       GrShaderVar::kAttribute_TypeModifier));
    }

private:
    /*
     * Internal call for GrGLProgramBuilder.addVarying
     */
    void addVarying(const char* name, GrGLVarying*);

    /*
     * private helpers for compilation by GrGLProgramBuilder
     */
    void transformToNormalizedDeviceSpace();
    void emitAttributes(const GrGeometryProcessor& gp);
    void bindVertexAttributes(GrGLuint programID);
    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;

    // an internal call which checks for uniquness of a var before adding it to the list of inputs
    bool addAttribute(const GrShaderVar& var);

    const char* fRtAdjustName;

    friend class GrGLProgramBuilder;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
