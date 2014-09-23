/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFullProgramBuilder_DEFINED
#define GrGLFullProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"
#include "../GrGLGeometryProcessor.h"

class GrGLVertexProgramEffects;

class GrGLFullProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLFullProgramBuilder(GrGpuGL*, const GrGLProgramDesc&);

   /** Add a varying variable to the current program to pass values between vertex and fragment
        shaders. If the last two parameters are non-NULL, they are filled in with the name
        generated. */
    void addVarying(GrSLType type,
                    const char* name,
                    const char** vsOutName = NULL,
                    const char** fsInName = NULL,
                    GrGLShaderVar::Precision fsPrecision=GrGLShaderVar::kDefault_Precision);

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

    /*
     * This non-virtual call will hide the parent call to prevent GPs from accessing fragment shader
     * functionality they shouldn't be using
     */
    GrGLProcessorFragmentShaderBuilder* getFragmentShaderBuilder() { return &fFS; }

private:
    virtual void createAndEmitEffects(const GrGeometryStage* geometryProcessor,
                                      const GrFragmentStage* colorStages[],
                                      const GrFragmentStage* coverageStages[],
                                      GrGLSLExpr4* inputColor,
                                      GrGLSLExpr4* inputCoverage) SK_OVERRIDE;

    GrGLProgramEffects* onCreateAndEmitEffects(const GrFragmentStage* effectStages[],
                                               int effectCnt,
                                               const GrGLProgramDesc::EffectKeyProvider&,
                                               GrGLSLExpr4* inOutFSColor);

    class GrGLGeometryProcessorEmitter : public GrGLProgramBuilder::GrGLProcessorEmitterInterface {
        public:
            GrGLGeometryProcessorEmitter(GrGLFullProgramBuilder* builder)
                : fBuilder(builder)
                , fGeometryProcessor(NULL)
                , fGLGeometryProcessor(NULL) {}
            virtual ~GrGLGeometryProcessorEmitter() {}
            void set(const GrGeometryProcessor* gp) {
                SkASSERT(NULL == fGeometryProcessor);
                fGeometryProcessor = gp;
            }
            virtual GrGLProcessor* createGLInstance() {
                SkASSERT(fGeometryProcessor);
                SkASSERT(NULL == fGLGeometryProcessor);
                fGLGeometryProcessor =
                        fGeometryProcessor->getFactory().createGLInstance(*fGeometryProcessor);
                return fGLGeometryProcessor;
            }
            virtual void emit(const GrProcessorKey& key,
                              const char* outColor,
                              const char* inColor,
                              const GrGLProcessor::TransformedCoordsArray& coords,
                              const GrGLProcessor::TextureSamplerArray& samplers) {
                SkASSERT(fGeometryProcessor);
                SkASSERT(fGLGeometryProcessor);
                fGLGeometryProcessor->emitCode(fBuilder, *fGeometryProcessor, key, outColor,
                                               inColor, coords, samplers);
                // this will not leak because it has already been used by createGLInstance
                fGLGeometryProcessor = NULL;
                fGeometryProcessor = NULL;
            }
        private:
            GrGLFullProgramBuilder*     fBuilder;
            const GrGeometryProcessor*  fGeometryProcessor;
            GrGLGeometryProcessor*      fGLGeometryProcessor;
        };

    virtual void emitEffect(const GrProcessorStage& stage,
                            const GrProcessorKey& key,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) SK_OVERRIDE;

    /**
     * Helper for emitEffect(). Emits code to implement an effect's coord transforms in the VS.
     * Varyings are added as an outputs of the VS and inputs to the FS. The varyings may be either a
     * vec2f or vec3f depending upon whether perspective interpolation is required or not. The names
     * of the varyings in the VS and FS as well their types are appended to the
     * TransformedCoordsArray* object, which is in turn passed to the effect's emitCode() function.
     */
    void emitTransforms(const GrProcessorStage& effectStage,
                        GrGLProcessor::TransformedCoordsArray* outCoords);

    virtual bool compileAndAttachShaders(GrGLuint programId,
                                         SkTDArray<GrGLuint>* shaderIds) const SK_OVERRIDE;

    virtual void bindProgramLocations(GrGLuint programId) SK_OVERRIDE;

    virtual GrGLProgramEffects* getProgramEffects() SK_OVERRIDE { return fProgramEffects.get(); }

    typedef GrGLProgramDesc::EffectKeyProvider EffectKeyProvider;

    GrGLGeometryProcessorEmitter fGLGeometryProcessorEmitter;
    GrGLGeometryShaderBuilder fGS;
    GrGLVertexShaderBuilder   fVS;
    SkAutoTDelete<GrGLVertexProgramEffects> fProgramEffects;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
