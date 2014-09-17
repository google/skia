/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFullProgramBuilder_DEFINED
#define GrGLFullProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

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
    virtual void emitCodeBeforeEffects(GrGLSLExpr4* color,
                                       GrGLSLExpr4* coverage) SK_OVERRIDE;

    virtual void emitGeometryProcessor(const GrEffectStage* geometryProcessor,
                                       GrGLSLExpr4* coverage) SK_OVERRIDE;

    virtual GrGLProgramEffects* createAndEmitEffects(const GrEffectStage* effectStages[],
                                                     int effectCnt,
                                                     const GrGLProgramDesc::EffectKeyProvider&,
                                                     GrGLSLExpr4* inOutFSColor) SK_OVERRIDE;

    /*
     * These functions are temporary and will eventually operate not on effects but on
     * geometry processors
     */
    void createAndEmitEffect(GrGLProgramEffectsBuilder*,
                             const GrEffectStage* effectStage,
                             const GrGLProgramDesc::EffectKeyProvider&,
                             GrGLSLExpr4* inOutFSColor);

    GrGLProgramEffects* createAndEmitEffect(const GrEffectStage* geometryProcessor,
                                            const GrGLProgramDesc::EffectKeyProvider&,
                                            GrGLSLExpr4* inOutFSColor);

    virtual void emitCodeAfterEffects() SK_OVERRIDE;

    virtual bool compileAndAttachShaders(GrGLuint programId,
                                         SkTDArray<GrGLuint>* shaderIds) const SK_OVERRIDE;

    virtual void bindProgramLocations(GrGLuint programId) SK_OVERRIDE;

    GrGLGeometryShaderBuilder fGS;
    GrGLVertexShaderBuilder   fVS;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
