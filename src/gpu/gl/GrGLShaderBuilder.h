/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderBuilder_DEFINED
#define GrGLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "GrBackendEffectFactory.h"
#include "GrEffect.h"
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
     * Passed to GrGLEffects to add texture reads to their shader code.
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
        // The idx param is used to ensure multiple samplers within a single effect have unique
        // uniform names.
        void init(GrGLShaderBuilder* builder, const GrTextureAccess* access, int idx) {
            GrAssert(NULL == fTextureAccess);
            GrAssert(GrGLUniformManager::kInvalidUniformHandle == fSamplerUniform);

            GrAssert(NULL != builder);
            GrAssert(NULL != access);
            SkString name;
            name.printf("Sampler%d_", idx);
            fSamplerUniform = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                                  kSampler2D_GrSLType,
                                                  name.c_str());
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

    /** Appends a 2D texture sample with projection if necessary. coordType must either be Vec2f or
        Vec3f. The latter is interpreted as projective texture coords. The vec length and swizzle
        order of the result depends on the GrTextureAccess associated with the TextureSampler. */
    void appendTextureLookup(SkString* out,
                             const TextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by TextureSampler must both be vec4 or
        float. If modulation is "" or NULL it this function acts as though appendTextureLookup were
        called. */
    void appendTextureLookupAndModulate(SkString* out,
                                        const char* modulation,
                                        const TextureSampler&,
                                        const char* coordName,
                                        GrSLType coordType = kVec2f_GrSLType) const;

    /** Emits a helper function outside of main(). Currently ShaderType must be
        kFragment_ShaderType. */
    void emitFunction(ShaderType shader,
                      GrSLType returnType,
                      const char* name,
                      int argCnt,
                      const GrGLShaderVar* args,
                      const char* body,
                      SkString* outName);

    /** Generates a EffectKey for the shader code based on the texture access parameters and the
        capabilities of the GL context.  This is useful for keying the shader programs that may
        have multiple representations, based on the type/format of textures used. */
    static GrBackendEffectFactory::EffectKey KeyForTextureAccess(const GrTextureAccess&,
                                                                 const GrGLCaps&);

    /** If texture swizzling is available using tex parameters then it is preferred over mangling
        the generated shader code. This potentially allows greater reuse of cached shaders. */
    static const GrGLenum* GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps);

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
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
     * Shortcut for getUniformVariable(u).c_str()
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

    /** Returns a variable name that represents the position of the fragment in the FS. The position
        is in device space (e.g. 0,0 is the top left and pixel centers are at half-integers). */
    const char* fragmentPosition();

    /** Returns a vertex attribute that represents the vertex position in the VS. This is the
        pre-matrix position and is commonly used by effects to compute texture coords via a matrix.
      */
    const GrGLShaderVar& positionAttribute() const { return *fPositionVar; }

    /**
     * Interfaces used by GrGLProgram.
     * TODO: Hide these from the GrEffects using friend or splitting this into two related classes.
     * Also, GrGLProgram's shader string construction should be moved to this class.
     */

    /** Called after building is complete to get the final shader string. */
    void getShader(ShaderType, SkString*) const;

    void setCurrentStage(int stageIdx) { fCurrentStageIdx = stageIdx; }
    void setNonStage() { fCurrentStageIdx = kNonStageIdx; }
    // TODO: move remainder of shader code generation to this class and call this privately
    // Handles of sampler uniforms generated for the effect are appended to samplerHandles.
    GrGLEffect* createAndEmitGLEffect(
                                const GrEffectStage& stage,
                                GrBackendEffectFactory::EffectKey key,
                                const char* fsInColor, // NULL means no incoming color
                                const char* fsOutColor,
                                const char* vsInCoord,
                                SkTArray<GrGLUniformManager::UniformHandle, true>* samplerHandles);
    GrGLUniformManager::UniformHandle getRTHeightUniform() const { return fRTHeightUniform; }
    // TODO: Make this do all the compiling, linking, etc.
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
    SkString    fVSCode;
    SkString    fGSCode;
    SkString    fFSCode;
    bool        fUsesGS;

private:
    enum {
        kNonStageIdx = -1,
    };

    const GrGLContextInfo&              fContext;
    GrGLUniformManager&                 fUniformManager;
    int                                 fCurrentStageIdx;
    SkString                            fFSFunctions;
    SkString                            fFSHeader;

    bool                                fSetupFragPosition;
    GrGLUniformManager::UniformHandle   fRTHeightUniform;

    GrGLShaderVar*                      fPositionVar;
};

#endif
