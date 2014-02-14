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
#include "GrColor.h"
#include "GrEffect.h"
#include "SkTypes.h"
#include "gl/GrGLProgramEffects.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLUniformManager.h"

#include <stdarg.h>

class GrGLContextInfo;
class GrEffectStage;
class GrGLProgramDesc;

/**
  Contains all the incremental state of a shader as it is being built,as well as helpers to
  manipulate that state.
*/
class GrGLShaderBuilder {
public:
    typedef GrTAllocator<GrGLShaderVar> VarArray;
    typedef GrBackendEffectFactory::EffectKey EffectKey;
    typedef GrGLProgramEffects::TextureSampler TextureSampler;
    typedef GrGLProgramEffects::TransformedCoordsArray TransformedCoordsArray;
    typedef GrGLUniformManager::BuilderUniform BuilderUniform;

    enum ShaderVisibility {
        kVertex_Visibility   = 0x1,
        kGeometry_Visibility = 0x2,
        kFragment_Visibility = 0x4,
    };

    GrGLShaderBuilder(GrGpuGL*, GrGLUniformManager&, const GrGLProgramDesc&);
    virtual ~GrGLShaderBuilder() {}

    /**
     * Use of these features may require a GLSL extension to be enabled. Shaders may not compile
     * if code is added that uses one of these features without calling enableFeature()
     */
    enum GLSLFeature {
        kStandardDerivatives_GLSLFeature = 0,

        kLastGLSLFeature = kStandardDerivatives_GLSLFeature
    };

    /**
     * If the feature is supported then true is returned and any necessary #extension declarations
     * are added to the shaders. If the feature is not supported then false will be returned.
     */
    bool enableFeature(GLSLFeature);

    /**
     * Called by GrGLEffects to add code the fragment shader.
     */
    void fsCodeAppendf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
        va_list args;
        va_start(args, format);
        fFSCode.appendVAList(format, args);
        va_end(args);
    }

    void fsCodeAppend(const char* str) { fFSCode.append(str); }

    /** Appends a 2D texture sample with projection if necessary. coordType must either be Vec2f or
        Vec3f. The latter is interpreted as projective texture coords. The vec length and swizzle
        order of the result depends on the GrTextureAccess associated with the TextureSampler. */
    void appendTextureLookup(SkString* out,
                             const TextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Version of above that appends the result to the fragment shader code instead.*/
    void fsAppendTextureLookup(const TextureSampler&,
                               const char* coordName,
                               GrSLType coordType = kVec2f_GrSLType);


    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by TextureSampler must both be vec4 or
        float. If modulation is "" or NULL it this function acts as though appendTextureLookup were
        called. */
    void fsAppendTextureLookupAndModulate(const char* modulation,
                                          const TextureSampler&,
                                          const char* coordName,
                                          GrSLType coordType = kVec2f_GrSLType);

    /** Emits a helper function outside of main() in the fragment shader. */
    void fsEmitFunction(GrSLType returnType,
                        const char* name,
                        int argCnt,
                        const GrGLShaderVar* args,
                        const char* body,
                        SkString* outName);

    typedef uint8_t DstReadKey;
    typedef uint8_t FragPosKey;

    /**  Returns a key for adding code to read the copy-of-dst color in service of effects that
         require reading the dst. It must not return 0 because 0 indicates that there is no dst
         copy read at all (in which case this function should not be called). */
    static DstReadKey KeyForDstRead(const GrTexture* dstCopy, const GrGLCaps&);

    /** Returns a key for reading the fragment location. This should only be called if there is an
        effect that will requires the fragment position. If the fragment position is not required,
        the key is 0. */
    static FragPosKey KeyForFragmentPosition(const GrRenderTarget* dst, const GrGLCaps&);

    /** If texture swizzling is available using tex parameters then it is preferred over mangling
        the generated shader code. This potentially allows greater reuse of cached shaders. */
    static const GrGLenum* GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps);

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
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

    const GrGLShaderVar& getUniformVariable(GrGLUniformManager::UniformHandle u) const {
        return fUniformManager.getBuilderUniform(fUniforms, u).fVariable;
    }

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    const char* getUniformCStr(GrGLUniformManager::UniformHandle u) const {
        return this->getUniformVariable(u).c_str();
    }

    /**
     * This returns a variable name to access the 2D, perspective correct version of the coords in
     * the fragment shader. If the coordinates at index are 3-dimensional, it immediately emits a
     * perspective divide into the fragment shader (xy / z) to convert them to 2D.
     */
    SkString ensureFSCoords2D(const TransformedCoordsArray&, int index);

    /** Returns a variable name that represents the position of the fragment in the FS. The position
        is in device space (e.g. 0,0 is the top left and pixel centers are at half-integers). */
    const char* fragmentPosition();

    /** Returns the color of the destination pixel. This may be NULL if no effect advertised
        that it will read the destination. */
    const char* dstColor();

    /**
     * Interfaces used by GrGLProgram.
     */
    const GrGLSLExpr4& getInputColor() const {
        return fInputColor;
    }
    const GrGLSLExpr4& getInputCoverage() const {
        return fInputCoverage;
    }

    /**
     * Adds code for effects and returns a GrGLProgramEffects* object. The caller is responsible for
     * deleting it when finished. effectStages contains the effects to add. effectKeys[i] is the key
     * generated from effectStages[i]. inOutFSColor specifies the input color to the first stage and
     * is updated to be the output color of the last stage.
     * The handles to texture samplers for effectStage[i] are added to
     * effectSamplerHandles[i].
     */
    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     const EffectKey effectKeys[],
                                                     int effectCnt,
                                                     GrGLSLExpr4* inOutFSColor) = 0;

    const char* getColorOutputName() const;
    const char* enableSecondaryOutput();

    GrGLUniformManager::UniformHandle getRTHeightUniform() const { return fRTHeightUniform; }
    GrGLUniformManager::UniformHandle getDstCopyTopLeftUniform() const {
        return fDstCopyTopLeftUniform;
    }
    GrGLUniformManager::UniformHandle getDstCopyScaleUniform() const {
        return fDstCopyScaleUniform;
    }
    GrGLUniformManager::UniformHandle getColorUniform() const { return fColorUniform; }
    GrGLUniformManager::UniformHandle getCoverageUniform() const { return fCoverageUniform; }
    GrGLUniformManager::UniformHandle getDstCopySamplerUniform() const {
        return fDstCopySamplerUniform;
    }

    bool finish(GrGLuint* outProgramId);

    const GrGLContextInfo& ctxInfo() const;

    /**
     * Helper for begining and ending a block in the fragment code. TODO: Make GrGLShaderBuilder
     * aware of all blocks and turn single \t's into the correct number of tabs (or spaces) so that
     * our shaders print pretty without effect writers tracking indentation.
     */
    class FSBlock {
    public:
        FSBlock(GrGLShaderBuilder* builder) : fBuilder(builder) {
            SkASSERT(NULL != builder);
            fBuilder->fsCodeAppend("\t{\n");
        }

        ~FSBlock() {
            fBuilder->fsCodeAppend("\t}\n");
        }
    private:
        GrGLShaderBuilder* fBuilder;
    };

protected:
    GrGpuGL* gpu() const { return fGpu; }

    void setInputColor(const GrGLSLExpr4& inputColor) { fInputColor = inputColor; }
    void setInputCoverage(const GrGLSLExpr4& inputCoverage) { fInputCoverage = inputCoverage; }

    /** Add input/output variable declarations (i.e. 'varying') to the fragment shader. */
    GrGLShaderVar& fsInputAppend() { return fFSInputs.push_back(); }

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);

    // Helper for emitEffects().
    void createAndEmitEffects(GrGLProgramEffectsBuilder*,
                              const GrEffectStage* effectStages[],
                              const EffectKey effectKeys[],
                              int effectCnt,
                              GrGLSLExpr4* inOutFSColor);

    virtual bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;
    virtual void bindProgramLocations(GrGLuint programId) const;

    void appendDecls(const VarArray&, SkString*) const;
    void appendUniformDecls(ShaderVisibility, SkString*) const;

private:
    class CodeStage : public SkNoncopyable {
    public:
        CodeStage() : fNextIndex(0), fCurrentIndex(-1), fEffectStage(NULL) {}

        bool inStageCode() const {
            this->validate();
            return NULL != fEffectStage;
        }

        const GrEffectStage* effectStage() const {
            this->validate();
            return fEffectStage;
        }

        int stageIndex() const {
            this->validate();
            return fCurrentIndex;
        }

        class AutoStageRestore : public SkNoncopyable {
        public:
            AutoStageRestore(CodeStage* codeStage, const GrEffectStage* newStage) {
                SkASSERT(NULL != codeStage);
                fSavedIndex = codeStage->fCurrentIndex;
                fSavedEffectStage = codeStage->fEffectStage;

                if (NULL == newStage) {
                    codeStage->fCurrentIndex = -1;
                } else {
                    codeStage->fCurrentIndex = codeStage->fNextIndex++;
                }
                codeStage->fEffectStage = newStage;

                fCodeStage = codeStage;
            }
            ~AutoStageRestore() {
                fCodeStage->fCurrentIndex = fSavedIndex;
                fCodeStage->fEffectStage = fSavedEffectStage;
            }
        private:
            CodeStage*              fCodeStage;
            int                     fSavedIndex;
            const GrEffectStage*    fSavedEffectStage;
        };
    private:
        void validate() const { SkASSERT((NULL == fEffectStage) == (-1 == fCurrentIndex)); }
        int                     fNextIndex;
        int                     fCurrentIndex;
        const GrEffectStage*    fEffectStage;
    } fCodeStage;

    /**
     * Features that should only be enabled by GrGLShaderBuilder itself.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature = kLastGLSLFeature + 1,
        kEXTShaderFramebufferFetch_GLSLPrivateFeature,
        kNVShaderFramebufferFetch_GLSLPrivateFeature,
    };
    bool enablePrivateFeature(GLSLPrivateFeature);

    // If we ever have VS/GS features we can expand this to take a bitmask of ShaderVisibility and
    // track the enables separately for each shader.
    void addFSFeature(uint32_t featureBit, const char* extensionName);

    // Interpretation of DstReadKey when generating code
    enum {
        kNoDstRead_DstReadKey         = 0,
        kYesDstRead_DstReadKeyBit     = 0x1, // Set if we do a dst-copy-read.
        kUseAlphaConfig_DstReadKeyBit = 0x2, // Set if dst-copy config is alpha only.
        kTopLeftOrigin_DstReadKeyBit  = 0x4, // Set if dst-copy origin is top-left.
    };

    enum {
        kNoFragPosRead_FragPosKey           = 0,  // The fragment positition will not be needed.
        kTopLeftFragPosRead_FragPosKey      = 0x1,// Read frag pos relative to top-left.
        kBottomLeftFragPosRead_FragPosKey   = 0x2,// Read frag pos relative to bottom-left.
    };

    GrGpuGL*                                fGpu;
    GrGLUniformManager&                     fUniformManager;
    uint32_t                                fFSFeaturesAddedMask;
    SkString                                fFSFunctions;
    SkString                                fFSExtensions;
    VarArray                                fFSInputs;
    VarArray                                fFSOutputs;
    GrGLUniformManager::BuilderUniformArray fUniforms;

    SkString                                fFSCode;

    bool                                    fSetupFragPosition;
    GrGLUniformManager::UniformHandle       fDstCopySamplerUniform;

    GrGLSLExpr4                             fInputColor;
    GrGLSLExpr4                             fInputCoverage;

    bool                                    fHasCustomColorOutput;
    bool                                    fHasSecondaryOutput;

    GrGLUniformManager::UniformHandle       fRTHeightUniform;
    GrGLUniformManager::UniformHandle       fDstCopyTopLeftUniform;
    GrGLUniformManager::UniformHandle       fDstCopyScaleUniform;
    GrGLUniformManager::UniformHandle       fColorUniform;
    GrGLUniformManager::UniformHandle       fCoverageUniform;

    bool                                    fTopLeftFragPosRead;
};

////////////////////////////////////////////////////////////////////////////////

class GrGLFullShaderBuilder : public GrGLShaderBuilder {
public:
    GrGLFullShaderBuilder(GrGpuGL*, GrGLUniformManager&, const GrGLProgramDesc&);

    /**
     * Called by GrGLEffects to add code to one of the shaders.
     */
    void vsCodeAppendf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
        va_list args;
        va_start(args, format);
        fVSCode.appendVAList(format, args);
        va_end(args);
    }

    void vsCodeAppend(const char* str) { fVSCode.append(str); }

   /** Add a vertex attribute to the current program that is passed in from the vertex data.
       Returns false if the attribute was already there, true otherwise. */
    bool addAttribute(GrSLType type, const char* name);

   /** Add a varying variable to the current program to pass values between vertex and fragment
        shaders. If the last two parameters are non-NULL, they are filled in with the name
        generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);

    /** Returns a vertex attribute that represents the vertex position in the VS. This is the
        pre-matrix position and is commonly used by effects to compute texture coords via a matrix.
      */
    const GrGLShaderVar& positionAttribute() const { return *fPositionVar; }

    /** Returns a vertex attribute that represents the local coords in the VS. This may be the same
        as positionAttribute() or it may not be. It depends upon whether the rendering code
        specified explicit local coords or not in the GrDrawState. */
    const GrGLShaderVar& localCoordsAttribute() const { return *fLocalCoordsVar; }

    /**
     * Are explicit local coordinates provided as input to the vertex shader.
     */
    bool hasExplicitLocalCoords() const { return (fLocalCoordsVar != fPositionVar); }

    bool addEffectAttribute(int attributeIndex, GrSLType type, const SkString& name);
    const SkString* getEffectAttributeName(int attributeIndex) const;

    virtual GrGLProgramEffects* createAndEmitEffects(
                const GrEffectStage* effectStages[],
                const EffectKey effectKeys[],
                int effectCnt,
                GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

    GrGLUniformManager::UniformHandle getViewMatrixUniform() const {
        return fViewMatrixUniform;
    }

protected:
    virtual bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const SK_OVERRIDE;
    virtual void bindProgramLocations(GrGLuint programId) const SK_OVERRIDE;

private:
    const GrGLProgramDesc&              fDesc;
    VarArray                            fVSAttrs;
    VarArray                            fVSOutputs;
    VarArray                            fGSInputs;
    VarArray                            fGSOutputs;

    SkString                            fVSCode;

    struct AttributePair {
        void set(int index, const SkString& name) {
            fIndex = index; fName = name;
        }
        int      fIndex;
        SkString fName;
    };
    SkSTArray<10, AttributePair, true>  fEffectAttributes;

    GrGLUniformManager::UniformHandle   fViewMatrixUniform;

    GrGLShaderVar*                      fPositionVar;
    GrGLShaderVar*                      fLocalCoordsVar;

    typedef GrGLShaderBuilder INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

class GrGLFragmentOnlyShaderBuilder : public GrGLShaderBuilder {
public:
    GrGLFragmentOnlyShaderBuilder(GrGpuGL*, GrGLUniformManager&, const GrGLProgramDesc&);

    int getNumTexCoordSets() const { return fNumTexCoordSets; }
    int addTexCoordSets(int count);

    virtual GrGLProgramEffects* createAndEmitEffects(
                const GrEffectStage* effectStages[],
                const EffectKey effectKeys[],
                int effectCnt,
                GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

private:
    int fNumTexCoordSets;

    typedef GrGLShaderBuilder INHERITED;
};

#endif
