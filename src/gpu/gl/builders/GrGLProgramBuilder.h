/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrAllocator.h"
#include "GrBackendProcessorFactory.h"
#include "GrColor.h"
#include "GrProcessor.h"
#include "GrGLFragmentShaderBuilder.h"
#include "GrGLGeometryShaderBuilder.h"
#include "GrGLVertexShaderBuilder.h"
#include "SkTypes.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLProgramDesc.h"
#include "gl/GrGLProgramEffects.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLProgramDataManager.h"

#include <stdarg.h>

class GrGLContextInfo;
class GrProcessorStage;
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

    bool genProgram(const GrGeometryStage* inGeometryProcessor,
                    const GrFragmentStage* inColorStages[],
                    const GrFragmentStage* inCoverageStages[]);

    GrGLProgramEffects* getGeometryProcessor() const {
        SkASSERT(fProgramID); return fGeometryProcessor.get();
    }
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
    void createAndEmitEffects(const GrFragmentStage* effectStages[],
                              int effectCnt,
                              const GrGLProgramDesc::EffectKeyProvider&,
                              GrGLSLExpr4* inOutFSColor);

    /*
     * A helper function called to emit the geometry processor as well as individual coverage
     * and color stages.  this will call into subclasses emit effect
     */
    void emitEffect(const GrProcessorStage& effectStage,
                    int effectIndex,
                    const GrGLProgramDesc::EffectKeyProvider& keyProvider,
                    GrGLSLExpr4* inColor,
                    GrGLSLExpr4* outColor);

    /**
     * Helper for emitEffect() in subclasses. Emits uniforms for an effect's texture accesses and
     * appends the necessary data to the TextureSamplerArray* object so effects can add texture
     * lookups to their code. This method is only meant to be called during the construction phase.
     */
    void emitSamplers(const GrProcessor& effect,
                      GrGLProcessor::TextureSamplerArray* outSamplers);

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);

    virtual bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;

    virtual void bindProgramLocations(GrGLuint programId);
    void resolveProgramLocations(GrGLuint programId);

    void appendDecls(const VarArray&, SkString*) const;
    void appendUniformDecls(ShaderVisibility, SkString*) const;

    class CodeStage : SkNoncopyable {
    public:
        CodeStage() : fNextIndex(0), fCurrentIndex(-1), fEffectStage(NULL) {}

        bool inStageCode() const {
            this->validate();
            return SkToBool(fEffectStage);
        }

        const GrProcessorStage* effectStage() const {
            this->validate();
            return fEffectStage;
        }

        int stageIndex() const {
            this->validate();
            return fCurrentIndex;
        }

        class AutoStageRestore : SkNoncopyable {
        public:
            AutoStageRestore(CodeStage* codeStage, const GrProcessorStage* newStage) {
                SkASSERT(codeStage);
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
            const GrProcessorStage*    fSavedEffectStage;
        };
    private:
        void validate() const { SkASSERT((NULL == fEffectStage) == (-1 == fCurrentIndex)); }
        int                     fNextIndex;
        int                     fCurrentIndex;
        const GrProcessorStage*    fEffectStage;
    };

    class GrGLProcessorEmitterInterface {
     public:
        virtual ~GrGLProcessorEmitterInterface() {}
        virtual GrGLProcessor* createGLInstance() = 0;
        virtual void emit(const GrProcessorKey& key,
                          const char* outColor,
                          const char* inColor,
                          const GrGLProcessor::TransformedCoordsArray& coords,
                          const GrGLProcessor::TextureSamplerArray& samplers) = 0;
    };

    class GrGLFragmentProcessorEmitter  : public GrGLProcessorEmitterInterface {
    public:
        GrGLFragmentProcessorEmitter(GrGLProgramBuilder* builder)
            : fBuilder(builder)
            , fFragmentProcessor(NULL)
            , fGLFragmentProcessor(NULL) {}
        virtual ~GrGLFragmentProcessorEmitter() {}
        void set(const GrFragmentProcessor* fp) {
            SkASSERT(NULL == fFragmentProcessor);
            fFragmentProcessor = fp;
        }
        virtual GrGLProcessor* createGLInstance() {
            SkASSERT(fFragmentProcessor);
            SkASSERT(NULL == fGLFragmentProcessor);
            fGLFragmentProcessor =
                    fFragmentProcessor->getFactory().createGLInstance(*fFragmentProcessor);
            return fGLFragmentProcessor;
        }
        virtual void emit(const GrProcessorKey& key,
                          const char* outColor,
                          const char* inColor,
                          const GrGLProcessor::TransformedCoordsArray& coords,
                          const GrGLProcessor::TextureSamplerArray& samplers) {
            SkASSERT(fFragmentProcessor);
            SkASSERT(fGLFragmentProcessor);
            fGLFragmentProcessor->emitCode(fBuilder, *fFragmentProcessor, key, outColor, inColor,
                                           coords, samplers);
            // this will not leak because it hasa already been used by createGLInstance
            fGLFragmentProcessor = NULL;
            fFragmentProcessor = NULL;
        }
    private:
        GrGLProgramBuilder*         fBuilder;
        const GrFragmentProcessor*  fFragmentProcessor;
        GrGLFragmentProcessor*      fGLFragmentProcessor;
    };

    GrGLProcessorEmitterInterface*   fEffectEmitter;
    CodeStage                        fCodeStage;
    SkAutoTUnref<GrGLProgramEffects> fGeometryProcessor;
    SkAutoTUnref<GrGLProgramEffects> fColorEffects;
    SkAutoTUnref<GrGLProgramEffects> fCoverageEffects;
    BuiltinUniformHandles            fUniformHandles;
    bool                             fFragOnly;
    int                              fTexCoordSetCnt;
    GrGLuint                         fProgramID;
    GrGLFragmentShaderBuilder        fFS;
    SeparableVaryingInfoArray        fSeparableVaryingInfos;

private:
    virtual void createAndEmitEffects(const GrGeometryStage* geometryProcessor,
                                      const GrFragmentStage* colorStages[],
                                      const GrFragmentStage* coverageStages[],
                                      GrGLSLExpr4* inputColor,
                                      GrGLSLExpr4* inputCoverage) = 0;
    /*
     * Subclasses override emitEffect below to emit data and code for a specific single effect
     */
    virtual void emitEffect(const GrProcessorStage&,
                            const GrProcessorKey&,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) = 0;

    /*
     * Because we have fragment only builders, and those builders need to implement a subclass
     * of program effects, we have to have base classes overload the program effects here
     */
    virtual GrGLProgramEffects* getProgramEffects() = 0;

    /**
     * Compiles all the shaders, links them into a program, and writes the program id to the output
     * struct.
     **/
    bool finish();

    GrGLFragmentProcessorEmitter            fGrProcessorEmitter;

    const GrGLProgramDesc&                  fDesc;
    GrGpuGL*                                fGpu;
    UniformInfoArray                        fUniforms;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexShaderBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryShaderBuilder;
};

#endif
