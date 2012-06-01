/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderBuilder_DEFINED
#define GrGLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "gl/GrGLShaderVar.h"
#include "gl/GrGLSL.h"

typedef GrTAllocator<GrGLShaderVar> VarArray;

/**
  Containts all the incremental state of a shader as it is being built,
  as well as helpers to manipulate that state.
  TODO: migrate CompileShaders() here?
*/

class GrGLShaderBuilder {

public:

    GrGLShaderBuilder();

    void computeSwizzle(uint32_t configFlags);
    void computeModulate(const char* fsInColor);

    // TODO: needs a better name
    enum SamplerMode {
        kDefault_SamplerMode,
        kProj_SamplerMode,
        kExplicitDivide_SamplerMode  // must do an explicit divide
    };

    /** Determines whether we should use texture2D() or texture2Dproj(),
        and if an explicit divide is required for the sample coordinates,
        creates the new variable and emits the code to initialize it. */
    void setupTextureAccess(SamplerMode samplerMode, int stageNum);

    /** texture2D(samplerName, coordName), with projection
        if necessary; if coordName is not specified,
        uses fSampleCoords. */
    void emitTextureLookup(const char* samplerName,
                           const char* coordName = NULL);

    /** sets outColor to results of texture lookup, with
        swizzle, and/or modulate as necessary */
    void emitDefaultFetch(const char* outColor,
                          const char* samplerName);

    /* TODO: can't arbitrarily OR together enum components, so
       VariableLifetime will need to be reworked if we add
       Geometry shaders. */
    enum VariableLifetime {
        kVertex_VariableLifetime = 1,
        kFragment_VariableLifetime = 2,
        kBoth_VariableLifetime = 3
    };

    /** Add a uniform variable to the current program, accessed
       in vertex, fragment, or both stages. If stageNum is
       specified, it is appended to the name to guarantee uniqueness;
       if count is specified, the uniform is an array.
    */
    const GrGLShaderVar& addUniform(VariableLifetime lifetime,
        GrSLType type,
        const char* name,
        int stageNum = -1,
        int count = GrGLShaderVar::kNonArray);

    /** Add a varying variable to the current program to pass
        values between vertex and fragment shaders.
        If the last two parameters are non-NULL, they are filled
        in with the name generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);

    /** Add a varying variable to the current program to pass
        values between vertex and fragment shaders;
        stageNum is appended to the name to guarantee uniqueness.
        If the last two parameters are non-NULL, they are filled
        in with the name generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    int stageNum,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);


    GrStringBuilder fHeader; // VS+FS, GLSL version, etc
    VarArray        fVSUnis;
    VarArray        fVSAttrs;
    VarArray        fVSOutputs;
    VarArray        fGSInputs;
    VarArray        fGSOutputs;
    VarArray        fFSInputs;
    GrStringBuilder fGSHeader; // layout qualifiers specific to GS
    VarArray        fFSUnis;
    VarArray        fFSOutputs;
    GrStringBuilder fFSFunctions;
    GrStringBuilder fVSCode;
    GrStringBuilder fGSCode;
    GrStringBuilder fFSCode;
    bool            fUsesGS;

    /// Per-stage settings - only valid while we're inside
    /// GrGLProgram::genStageCode().
    //@{

    int              fVaryingDims;
    static const int fCoordDims = 2;

    /// True if fSampleCoords is an expression; false if it's a bare
    /// variable name
    bool             fComplexCoord;
    GrStringBuilder  fSampleCoords;

    GrStringBuilder  fSwizzle;
    GrStringBuilder  fModulate;

    GrStringBuilder  fTexFunc;

    //@}

};

#endif
