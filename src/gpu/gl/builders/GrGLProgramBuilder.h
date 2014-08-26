/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrAllocator.h"
#include "GrBackendEffectFactory.h"
#include "GrColor.h"
#include "GrEffect.h"
#include "GrGLFragmentShaderBuilder.h"
#include "GrGLGeometryShaderBuilder.h"
#include "GrGLVertexShaderBuilder.h"
#include "SkTypes.h"
#include "gl/GrGLProgramDesc.h"
#include "gl/GrGLProgramEffects.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLProgramDataManager.h"

#include <stdarg.h>

class GrGLContextInfo;
class GrEffectStage;
class GrGLProgramDesc;

/**
  Contains all the incremental state of a shader as it is being built,as well as helpers to
  manipulate that state.
*/
class GrGLProgramBuilder {
public:
    enum ShaderVisibility {
        kVertex_Visibility   = 0x1,
        kGeometry_Visibility = 0x2,
        kFragment_Visibility = 0x4,
    };

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProgramDataManager::VaryingHandle VaryingHandle;

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fViewMatrixUni;
        UniformHandle       fRTAdjustmentUni;
        UniformHandle       fColorUni;
        UniformHandle       fCoverageUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;

        // Uniforms for computing texture coords to do the dst-copy lookup
        UniformHandle       fDstCopyTopLeftUni;
        UniformHandle       fDstCopyScaleUni;
        UniformHandle       fDstCopySamplerUni;
    };

    struct UniformInfo {
        GrGLShaderVar fVariable;
        uint32_t      fVisibility;
        GrGLint       fLocation;
    };

    // This uses an allocator rather than array so that the GrGLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<UniformInfo> UniformInfoArray;

    struct SeparableVaryingInfo {
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    typedef GrTAllocator<SeparableVaryingInfo> SeparableVaryingInfoArray;

    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    bool genProgram(const GrEffectStage* inColorStages[],
                    const GrEffectStage* inCoverageStages[]);

    // Below are the results of the shader generation.

    GrGLProgramEffects* getColorEffects() const { SkASSERT(fProgramID); return fColorEffects.get(); }
    GrGLProgramEffects* getCoverageEffects() const { SkASSERT(fProgramID); return fCoverageEffects.get(); }
    const BuiltinUniformHandles& getBuiltinUniformHandles() const {
        SkASSERT(fProgramID);
        return fUniformHandles;
    }
    GrGLuint getProgramID() const { SkASSERT(fProgramID); return fProgramID; }
    bool hasVertexShader() const { SkASSERT(fProgramID); return !fFragOnly; }
    int getTexCoordSetCount() const { SkASSERT(fProgramID); return fTexCoordSetCnt; }
    const UniformInfoArray& getUniformInfos() const { return fUniforms; }
    const SeparableVaryingInfoArray& getSeparableVaryingInfos() const {
        return fSeparableVaryingInfos;
    }

    virtual ~GrGLProgramBuilder() {}

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not NULL then
        it will refer to the final uniform name after return. Use the addUniformArray variant to add
        an array of uniforms. */
    GrGLProgramDataManager::UniformHandle addUniform(uint32_t visibility,
                                                     GrSLType type,
                                                     const char* name,
                                                     const char** outName = NULL) {
        return this->addUniformArray(visibility, type, name, GrGLShaderVar::kNonArray, outName);
    }
    GrGLProgramDataManager::UniformHandle addUniformArray(uint32_t visibility,
                                                          GrSLType type,
                                                          const char* name,
                                                          int arrayCount,
                                                          const char** outName = NULL);

    const GrGLShaderVar& getUniformVariable(GrGLProgramDataManager::UniformHandle u) const {
        return fUniforms[u.toShaderBuilderIndex()].fVariable;
    }

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    const char* getUniformCStr(GrGLProgramDataManager::UniformHandle u) const {
        return this->getUniformVariable(u).c_str();
    }

    const GrGLContextInfo& ctxInfo() const;

    GrGLFragmentShaderBuilder* getFragmentShaderBuilder() { return &fFS; }
    GrGpuGL* gpu() const { return fGpu; }

protected:
    typedef GrTAllocator<GrGLShaderVar> VarArray;
    GrGLProgramBuilder(GrGpuGL*, const GrGLProgramDesc&);

    const GrGLProgramDesc& desc() const { return fDesc; }

    // Helper for emitEffects().
    void createAndEmitEffects(GrGLProgramEffectsBuilder*,
                              const GrEffectStage* effectStages[],
                              int effectCnt,
                              const GrGLProgramDesc::EffectKeyProvider&,
                              GrGLSLExpr4* inOutFSColor);

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);

    virtual bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;

    virtual void bindProgramLocations(GrGLuint programId);
    void resolveProgramLocations(GrGLuint programId);

    void appendDecls(const VarArray&, SkString*) const;
    void appendUniformDecls(ShaderVisibility, SkString*) const;

    SkAutoTUnref<GrGLProgramEffects> fColorEffects;
    SkAutoTUnref<GrGLProgramEffects> fCoverageEffects;
    BuiltinUniformHandles            fUniformHandles;
    bool                             fFragOnly;
    int                              fTexCoordSetCnt;
    GrGLuint                         fProgramID;
    GrGLFragmentShaderBuilder        fFS;
    SeparableVaryingInfoArray        fSeparableVaryingInfos;
private:
    class CodeStage : SkNoncopyable {
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

        class AutoStageRestore : SkNoncopyable {
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
     * The base class will emit the fragment code that precedes the per-effect code and then call
     * this function. The subclass can use it to insert additional fragment code that should
     * execute before the effects' code and/or emit other shaders (e.g. geometry, vertex).
     *
     * The subclass can modify the initial color or coverage 
     */
    virtual void emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) = 0;

    /**
    * Adds code for effects and returns a GrGLProgramEffects* object. The caller is responsible for
    * deleting it when finished. effectStages contains the effects to add. The effect key provider 
    * is used to communicate the key each effect created in its GenKey function. inOutFSColor
    * specifies the input color to the first stage and is updated to be the output color of the
    * last stage. The handles to texture samplers for effectStage[i] are added to
    * effectSamplerHandles[i].
    */
    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     int effectCnt,
                                                     const GrGLProgramDesc::EffectKeyProvider&,
                                                     GrGLSLExpr4* inOutFSColor) = 0;

    /**
     * Similar to emitCodeBeforeEffects() but called after per-effect code is emitted.
     */
    virtual void emitCodeAfterEffects() = 0;

    /**
     * Compiles all the shaders, links them into a program, and writes the program id to the output
     * struct.
     **/
    bool finish();

    const GrGLProgramDesc&                  fDesc;
    GrGpuGL*                                fGpu;
    UniformInfoArray                        fUniforms;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexShaderBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryShaderBuilder;
};

////////////////////////////////////////////////////////////////////////////////

class GrGLFullProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLFullProgramBuilder(GrGpuGL*, const GrGLProgramDesc&);

   /** Add a varying variable to the current program to pass values between vertex and fragment
        shaders. If the last two parameters are non-NULL, they are filled in with the name
        generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL);

    /** Add a separable varying input variable to the current program.
     * A separable varying (fragment shader input) is a varying that can be used also when vertex
     * shaders are not used. With a vertex shader, the operation is same as with other
     * varyings. Without a vertex shader, such as with NV_path_rendering, GL APIs are used to
     * populate the variable. The APIs can refer to the variable through the returned handle.
     */
    VaryingHandle addSeparableVarying(GrSLType type,
                                      const char* name,
                                      const char** vsOutName,
                                      const char** fsInName);

    GrGLVertexShaderBuilder* getVertexShaderBuilder() { return &fVS; }

private:
    virtual void emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) SK_OVERRIDE;

    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     int effectCnt,
                                                     const GrGLProgramDesc::EffectKeyProvider&,
                                                     GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

    virtual void emitCodeAfterEffects() SK_OVERRIDE;

    virtual bool compileAndAttachShaders(GrGLuint programId,
                                         SkTDArray<GrGLuint>* shaderIds) const SK_OVERRIDE;

    virtual void bindProgramLocations(GrGLuint programId) SK_OVERRIDE;

    GrGLGeometryShaderBuilder fGS;
    GrGLVertexShaderBuilder   fVS;

    typedef GrGLProgramBuilder INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

class GrGLFragmentOnlyProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLFragmentOnlyProgramBuilder(GrGpuGL*, const GrGLProgramDesc&);

    int addTexCoordSets(int count);

private:
    virtual void emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) SK_OVERRIDE {}

    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     int effectCnt,
                                                     const GrGLProgramDesc::EffectKeyProvider&,
                                                     GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

    virtual void emitCodeAfterEffects() SK_OVERRIDE {}

    typedef GrGLProgramBuilder INHERITED;
};

#endif
