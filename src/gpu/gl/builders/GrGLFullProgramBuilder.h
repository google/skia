/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFullProgramBuilder_DEFINED
#define GrGLFullProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

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

private:
    virtual void createAndEmitEffects(const GrEffectStage* geometryProcessor,
                                      const GrEffectStage* colorStages[],
                                      const GrEffectStage* coverageStages[],
                                      GrGLSLExpr4* inputColor,
                                      GrGLSLExpr4* inputCoverage) SK_OVERRIDE;

    GrGLProgramEffects* onCreateAndEmitEffects(const GrEffectStage* effectStages[],
                                               int effectCnt,
                                               const GrGLProgramDesc::EffectKeyProvider&,
                                               GrGLSLExpr4* inOutFSColor);

    virtual void emitEffect(const GrEffectStage& stage,
                            const GrEffectKey& key,
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
    void emitTransforms(const GrEffectStage& effectStage,
                        GrGLEffect::TransformedCoordsArray* outCoords);

    virtual bool compileAndAttachShaders(GrGLuint programId,
                                         SkTDArray<GrGLuint>* shaderIds) const SK_OVERRIDE;

    virtual void bindProgramLocations(GrGLuint programId) SK_OVERRIDE;

    virtual GrGLProgramEffects* getProgramEffects() SK_OVERRIDE { return fProgramEffects.get(); }

    typedef GrGLProgramDesc::EffectKeyProvider EffectKeyProvider;

    GrGLGeometryShaderBuilder fGS;
    GrGLVertexShaderBuilder   fVS;
    SkAutoTDelete<GrGLVertexProgramEffects> fProgramEffects;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
