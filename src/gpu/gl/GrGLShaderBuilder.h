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
    /**
     * Used by GrGLProgramStages to add texture reads to their shader code.
     */
    class TextureSampler {
    public:
        TextureSampler()
            : fTextureAccess(NULL)
            , fSamplerUniform(GrGLUniformManager::kInvalidUniformHandle) {}

        TextureSampler(const TextureSampler& other) { *this = other; }

        TextureSampler& operator= (const TextureSampler& other) {
            GrAssert(NULL == fTextureAccess);
            GrAssert(GrGLUniformManager::kInvalidUniformHandle == fSamplerUniform);

            fTextureAccess = other.fTextureAccess;
            fSamplerUniform = other.fSamplerUniform;
            return *this;
        }

        const GrTextureAccess* textureAccess() const { return fTextureAccess; }

    private:
        void init(GrGLShaderBuilder* builder, const GrTextureAccess* access) {
            GrAssert(NULL == fTextureAccess);
            GrAssert(GrGLUniformManager::kInvalidUniformHandle == fSamplerUniform);

            GrAssert(NULL != builder);
            GrAssert(NULL != access);
            fSamplerUniform = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                  kSampler2D_GrSLType,
                                                  "Sampler");
            GrAssert(GrGLUniformManager::kInvalidUniformHandle != fSamplerUniform);

            fTextureAccess = access;
        }

        const GrTextureAccess*            fTextureAccess;
        GrGLUniformManager::UniformHandle fSamplerUniform;

        friend class GrGLShaderBuilder; // to access fSamplerUniform
        friend class GrGLProgram;       // to construct these and access fSamplerUniform.
    };

    typedef SkTArray<TextureSampler> TextureSamplerArray;

    enum ShaderType {
        kVertex_ShaderType   = 0x1,
        kGeometry_ShaderType = 0x2,
        kFragment_ShaderType = 0x4,
    };

    GrGLShaderBuilder(const GrGLContextInfo&, GrGLUniformManager&);

    /** Determines whether we should use texture2D() or texture2Dproj(), and if an explicit divide
        is required for the sample coordinates, creates the new variable and emits the code to
        initialize it. This should only be called by GrGLProgram.*/
    void setupTextureAccess(const char* varyingFSName, GrSLType varyingType);

    /** Appends a texture sample with projection if necessary; if coordName is not
        specified, uses fSampleCoords. coordType must either be Vec2f or Vec3f. The latter is
        interpreted as projective texture coords. The vec length and swizzle order of the result
        depends on the GrTextureAccess associated with the TextureSampler. */
    void appendTextureLookup(SkString* out,
                             const TextureSampler&,
                             const char* coordName = NULL,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by TextureSampler must both be vec4 or
        float. If modulation is "" or NULL it this function acts as though appendTextureLookup were
        called. */
    void appendTextureLookupAndModulate(SkString* out,
                                        const char* modulation,
                                        const TextureSampler&,
                                        const char* coordName = NULL,
                                        GrSLType coordType = kVec2f_GrSLType) const;

    /** Gets the name of the default texture coords which are always kVec2f */
    const char* defaultTexCoordsName() const { return fDefaultTexCoordsName.c_str(); }

    /* Returns true if the texture matrix from which the default texture coords are computed has
       perspective. */
    bool defaultTextureMatrixIsPerspective() const {
        return fTexCoordVaryingType == kVec3f_GrSLType;
    }

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

    /** If texture swizzling is available using tex parameters then it is preferred over mangling
        the generated shader code. This potentially allows greater reuse of cached shaders. */
    static const GrGLenum* GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps);

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
