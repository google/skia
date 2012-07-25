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
#include "gl/GrGLUniformManager.h"

class GrGLContextInfo;

/**
  Contains all the incremental state of a shader as it is being built,as well as helpers to
  manipulate that state.
*/
class GrGLShaderBuilder {

public:

    enum ShaderType {
        kVertex_ShaderType   = 0x1,
        kGeometry_ShaderType = 0x2,
        kFragment_ShaderType = 0x4,
    };

    GrGLShaderBuilder(const GrGLContextInfo&, GrGLUniformManager&);

    void computeSwizzle(uint32_t configFlags);
    void computeModulate(const char* fsInColor);

    // TODO: needs a better name
    enum SamplerMode {
        kDefault_SamplerMode,
        kProj_SamplerMode,
        kExplicitDivide_SamplerMode  // must do an explicit divide
    };

    /** Determines whether we should use texture2D() or texture2Dproj(), and if an explicit divide
        is required for the sample coordinates, creates the new variable and emits the code to
        initialize it. */
    void setupTextureAccess(SamplerMode samplerMode, int stageNum);

    /** texture2D(samplerName, coordName), with projection if necessary; if coordName is not
        specified, uses fSampleCoords. */
    void emitTextureLookup(const char* samplerName,
                           const char* coordName = NULL);

    /** sets outColor to results of texture lookup, with swizzle, and/or modulate as necessary */
    void emitDefaultFetch(const char* outColor,
                          const char* samplerName);

    /** Add a uniform variable to the current program, that has visibilty in one or more shaders.
        If stageNum is specified, it is appended to the name to guarantee uniqueness; if count is
        specified, the uniform is an array. visibility is a bitfield of ShaderType values indicating
        from which shaders the uniform should be accessible. At least one bit must be set. Geometry
        shader uniforms are not supported at this time.
    */
    GrGLUniformManager::UniformHandle addUniform(uint32_t visibility,
                                                 GrSLType type,
                                                 const char* name,
                                                 int stageNum = -1,
                                                 int count = GrGLShaderVar::kNonArray);

    const GrGLShaderVar& getUniformVariable(GrGLUniformManager::UniformHandle) const;

    /**
     * Shorcut for getUniformVariable(u).c_str()
     */
    const char* getUniformCStr(GrGLUniformManager::UniformHandle u) const {
        return this->getUniformVariable(u).c_str();
    }

    /** Add a varying variable to the current program to pass values between vertex and fragment
        shaders. If the last two parameters are non-NULL, they are filled in with the name
        generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);

    /** Add a varying variable to the current program to pass values between vertex and fragment
        shaders; stageNum is appended to the name to guarantee uniqueness. If the last two
        parameters are non-NULL, they are filled in with the name generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    int stageNum,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);

    /** Called after building is complete to get the final shader string. */
    void getShader(ShaderType, SkString*) const;

    /**
     * TODO: Make this do all the compiling, linking, etc. Hide this from the custom stages
     */
    void finished(GrGLuint programID);

private:

    typedef GrTAllocator<GrGLShaderVar> VarArray;

    void appendDecls(const VarArray&, SkString*) const;
    void appendUniformDecls(ShaderType, SkString*) const;

    typedef GrGLUniformManager::BuilderUniform BuilderUniform;
    GrGLUniformManager::BuilderUniformArray fUniforms;

    // TODO: Everything below here private.
public:

    SkString    fHeader; // VS+FS, GLSL version, etc
    VarArray    fVSAttrs;
    VarArray    fVSOutputs;
    VarArray    fGSInputs;
    VarArray    fGSOutputs;
    VarArray    fFSInputs;
    SkString    fGSHeader; // layout qualifiers specific to GS
    VarArray    fFSOutputs;
    SkString    fFSFunctions;
    SkString    fVSCode;
    SkString    fGSCode;
    SkString    fFSCode;
    bool        fUsesGS;

    /// Per-stage settings - only valid while we're inside GrGLProgram::genStageCode().
    //@{

    int              fVaryingDims;
    static const int fCoordDims = 2;

    /// True if fSampleCoords is an expression; false if it's a bare
    /// variable name
    bool             fComplexCoord;
    SkString         fSampleCoords;

    SkString         fSwizzle;
    SkString         fModulate;

    SkString         fTexFunc;

    //@}

private:
    const GrGLContextInfo&  fContext;
    GrGLUniformManager&     fUniformManager;
};

#endif
