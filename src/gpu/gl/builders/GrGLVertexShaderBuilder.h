/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexShader_DEFINED
#define GrGLVertexShader_DEFINED
#include "GrGLShaderBuilder.h"

class GrGLProgramBuilder;

class GrGLVertexShaderBuilder : public GrGLFullShaderBuilder {
public:
    GrGLVertexShaderBuilder(GrGLFullProgramBuilder* program);

    /*
     * this call is only for GrGLProgramEffects' internal use
     */
    void emitAttributes(const GrGeometryProcessor& gp);

    /**
     * Are explicit local coordinates provided as input to the vertex shader.
     */
    bool hasExplicitLocalCoords() const { return (fLocalCoordsVar != fPositionVar); }

    const SkString* getEffectAttributeName(int attributeIndex) const;

    /** Returns a vertex attribute that represents the local coords in the VS. This may be the same
        as positionAttribute() or it may not be. It depends upon whether the rendering code
        specified explicit local coords or not in the GrDrawState. */
    const GrGLShaderVar& localCoordsAttribute() const { return *fLocalCoordsVar; }

    /** Returns a vertex attribute that represents the vertex position in the VS. This is the
        pre-matrix position and is commonly used by effects to compute texture coords via a matrix.
      */
    const GrGLShaderVar& positionAttribute() const { return *fPositionVar; }

private:
    /*
     * Add attribute will push a new attribute onto the end.  It will also assert if there is
     * a duplicate attribute
     */
    bool addAttribute(const GrShaderVar& var);

    /*
     * Internal call for GrGLFullProgramBuilder.addVarying
     */
    void addVarying(GrSLType type,
                   const char* name,
                   const char** vsOutName);

    /*
     * private helpers for compilation by GrGLProgramBuilder
     */
    void bindProgramLocations(GrGLuint programId);
    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;
    void emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage);
    void emitCodeAfterEffects();

    struct AttributePair {
        void set(int index, const SkString& name) {
            fIndex = index; fName = name;
        }
        int      fIndex;
        SkString fName;
    };

    GrGLShaderVar*                      fPositionVar;
    GrGLShaderVar*                      fLocalCoordsVar;
    int                                 fEffectAttribOffset;

    friend class GrGLFullProgramBuilder;

    typedef GrGLFullShaderBuilder INHERITED;
};

#endif
