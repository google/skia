/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderBuilder_DEFINED
#define GrGLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "GrCustomStage.h"
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

    /** Determines whether we should use texture2D() or texture2Dproj(), and if an explicit divide
        is required for the sample coordinates, creates the new variable and emits the code to
        initialize it. This should only be called by GrGLProgram.*/
    void setupTextureAccess(const char* varyingFSName, GrSLType varyingType);

    /** texture2D(samplerName, coordName), with projection if necessary; if coordName is not
        specified, uses fSampleCoords. coordType must either be Vec2f or Vec3f. The latter is
        interpreted as projective texture coords. */
    void emitTextureLookup(const char* samplerName,
                           const char* coordName = NULL,
                           GrSLType coordType = kVec2f_GrSLType);

    /** sets outColor to results of texture lookup, with swizzle, and/or modulate as necessary. If
    coordName is NULL then it as if defaultTexCoordsName() was passed. coordType must be either
    kVec2f or kVec3f. */
    void emitTextureLookupAndModulate(const char* outColor,
                                      const char* samplerName,
                                      const char* coordName = NULL,
                                      GrSLType coordType = kVec2f_GrSLType);

    /** Gets the name of the default texture coords which are always kVec2f */
    const char* defaultTexCoordsName() const { return fDefaultTexCoordsName.c_str(); }

    /* Returns true if the texture matrix from which the default texture coords are computed has
       perspective. */
    bool defaultTextureMatrixIsPerspective() const {
        return fTexCoordVaryingType == kVec3f_GrSLType;
    }

    /** Emits a texture lookup to the shader code with the form:
          texture2D{Proj}(samplerName, coordName).swizzle
        The routine selects the type of texturing based on samplerMode.
        The generated swizzle state is built based on the format of the texture and the requested
        swizzle access pattern.  coordType must either be Vec2f or Vec3f. The latter is interpreted
        as projective texture coords.*/
    void emitCustomTextureLookup(const GrTextureAccess& textureAccess,
                                 const char* samplerName,
                                 const char* coordName,
                                 GrSLType coordType = kVec2f_GrSLType);

    /** Emits a helper function outside of main(). Currently ShaderType must be
        kFragment_ShaderType. */
    void emitFunction(ShaderType shader,
                      GrSLType returnType,
                      const char* name,
                      int argCnt,
                      const GrGLShaderVar* args,
                      const char* body,
                      SkString* outName);

    /** Generates a StageKey for the shader code based on the texture access parameters and the
        capabilities of the GL context.  This is useful for keying the shader programs that may
        have multiple representations, based on the type/format of textures used. */
    static GrCustomStage::StageKey KeyForTextureAccess(const GrTextureAccess& access,
                                                       const GrGLCaps& caps);

    /** Add a uniform variable to the current program, that has visibilty in one or more shaders.
        visibility is a bitfield of ShaderType values indicating from which shaders the uniform
        should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not NULL then
        it will refer to the final uniform name after return. Use the addUniformArray variant to add
        an array of uniforms.
    */
    GrGLUniformManager::UniformHandle addUniform(uint32_t visibility,
                                                 GrSLType type,
                                                 const char* name,
                                                 const char** outName = NULL) {
        return this->addUniformArray(visibility, type, name, GrGLShaderVar::kNonArray, outName);
    }
    GrGLUniformManager::UniformHandle addUniformArray(uint32_t visibility,
                                                      GrSLType type,
                                                      const char* name,
                                                      int arrayCount,
                                                      const char** outName = NULL);

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

    /** Called after building is complete to get the final shader string. */
    void getShader(ShaderType, SkString*) const;

    /**
     * TODO: Make this do all the compiling, linking, etc. Hide from the custom stages
     */
    void finished(GrGLuint programID);

    /**
     * Sets the current stage (used to make variable names unique).
     * TODO: Hide from the custom stages
     */
    void setCurrentStage(int stage) { fCurrentStage = stage; }
    void setNonStage() { fCurrentStage = kNonStageIdx; }

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
    SkString    fVSCode;
    SkString    fGSCode;
    SkString    fFSCode;
    bool        fUsesGS;

    /// Per-stage settings - only valid while we're inside GrGLProgram::genStageCode().
    //@{

    SkString         fSwizzle;
    SkString         fModulate;

    //@}

private:
    enum {
        kNonStageIdx = -1,
    };

    const GrGLContextInfo&  fContext;
    GrGLUniformManager&     fUniformManager;
    int                     fCurrentStage;
    SkString                fFSFunctions;

    /// Per-stage settings - only valid while we're inside GrGLProgram::genStageCode().
    //@{
    GrSLType         fTexCoordVaryingType;  // the type, either Vec2f or Vec3f, of the coords passed
                                            // as a varying from the VS to the FS.
    SkString         fDefaultTexCoordsName; // the name of the default 2D coords value.
    //@}

};

#endif
