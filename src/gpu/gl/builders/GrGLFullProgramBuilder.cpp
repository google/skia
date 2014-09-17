/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFullProgramBuilder.h"
#include "../GrGpuGL.h"

GrGLFullProgramBuilder::GrGLFullProgramBuilder(GrGpuGL* gpu,
                                              const GrGLProgramDesc& desc)
    : INHERITED(gpu, desc)
    , fGS(this)
    , fVS(this) {
}

void GrGLFullProgramBuilder::emitCodeBeforeEffects(GrGLSLExpr4* color,
                                                   GrGLSLExpr4* coverage) {
    fVS.emitCodeBeforeEffects(color, coverage);
}

void GrGLFullProgramBuilder::emitGeometryProcessor(const GrEffectStage* geometryProcessor,
                                                   GrGLSLExpr4* coverage) {
    if (geometryProcessor) {
        GrGLProgramDesc::EffectKeyProvider geometryProcessorKeyProvider(
                &this->desc(), GrGLProgramDesc::EffectKeyProvider::kGeometryProcessor_EffectType);
        fGeometryProcessor.reset(this->createAndEmitEffect(
                                 geometryProcessor,
                                 geometryProcessorKeyProvider,
                                 coverage));
    }
}

void GrGLFullProgramBuilder::emitCodeAfterEffects() {
    fVS.emitCodeAfterEffects();
}

void GrGLFullProgramBuilder::addVarying(GrSLType type,
                                        const char* name,
                                        const char** vsOutName,
                                        const char** fsInName,
                                        GrGLShaderVar::Precision fsPrecision) {
    fVS.addVarying(type, name, vsOutName);

    SkString* fsInputName = fVS.fOutputs.back().accessName();

#if GR_GL_EXPERIMENTAL_GS
    if (desc().getHeader().fExperimentalGS) {
       // TODO let the caller use these names
       fGS.addVarying(type, fsInputName->c_str(), NULL);
       fsInputName = fGS.fOutputs.back().accessName();
    }
#endif
    fFS.addVarying(type, fsInputName->c_str(), fsInName, fsPrecision);
}

GrGLFullProgramBuilder::VaryingHandle
GrGLFullProgramBuilder::addSeparableVarying(GrSLType type,
                                            const char* name,
                                            const char** vsOutName,
                                            const char** fsInName) {
    addVarying(type, name, vsOutName, fsInName);
    SeparableVaryingInfo& varying = fSeparableVaryingInfos.push_back();
    varying.fVariable = fFS.fInputs.back();
    return VaryingHandle::CreateFromSeparableVaryingIndex(fSeparableVaryingInfos.count() - 1);
}


GrGLProgramEffects* GrGLFullProgramBuilder::createAndEmitEffects(
        const GrEffectStage* effectStages[],
        int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider,
        GrGLSLExpr4* inOutFSColor) {

    GrGLVertexProgramEffectsBuilder programEffectsBuilder(this, effectCnt);
    this->INHERITED::createAndEmitEffects(&programEffectsBuilder,
                                          effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return programEffectsBuilder.finish();
}

void GrGLFullProgramBuilder::createAndEmitEffect(GrGLProgramEffectsBuilder* programEffectsBuilder,
                                              const GrEffectStage* effectStages,
                                              const GrGLProgramDesc::EffectKeyProvider& keyProvider,
                                              GrGLSLExpr4* fsInOutColor) {
    GrGLSLExpr4 inColor = *fsInOutColor;
    GrGLSLExpr4 outColor;

    SkASSERT(effectStages && effectStages->getEffect());
    const GrEffectStage& stage = *effectStages;

    // Using scope to force ASR destructor to be triggered
    {
        CodeStage::AutoStageRestore csar(&fCodeStage, &stage);

        if (inColor.isZeros()) {
            SkString inColorName;

            // Effects have no way to communicate zeros, they treat an empty string as ones.
            this->nameVariable(&inColorName, '\0', "input");
            fFS.codeAppendf("vec4 %s = %s;", inColorName.c_str(), inColor.c_str());
            inColor = inColorName;
        }

        // create var to hold stage result
        SkString outColorName;
        this->nameVariable(&outColorName, '\0', "output");
        fFS.codeAppendf("vec4 %s;", outColorName.c_str());
        outColor = outColorName;


        programEffectsBuilder->emitEffect(stage,
                                          keyProvider.get(0),
                                          outColor.c_str(),
                                          inColor.isOnes() ? NULL : inColor.c_str(),
                                          fCodeStage.stageIndex());
    }

    *fsInOutColor = outColor;
}

GrGLProgramEffects* GrGLFullProgramBuilder::createAndEmitEffect(
        const GrEffectStage* geometryProcessor,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider,
        GrGLSLExpr4* inOutFSColor) {

    GrGLVertexProgramEffectsBuilder programEffectsBuilder(this, 1);
    this->createAndEmitEffect(&programEffectsBuilder, geometryProcessor, keyProvider, inOutFSColor);
    return programEffectsBuilder.finish();
}

bool GrGLFullProgramBuilder::compileAndAttachShaders(GrGLuint programId,
                                                     SkTDArray<GrGLuint>* shaderIds) const {
    return INHERITED::compileAndAttachShaders(programId, shaderIds)
         && fVS.compileAndAttachShaders(programId, shaderIds)
#if GR_GL_EXPERIMENTAL_GS
         && (!desc().getHeader().fExperimentalGS
                 || fGS.compileAndAttachShaders(programId, shaderIds))
#endif
         ;
}

void GrGLFullProgramBuilder::bindProgramLocations(GrGLuint programId) {
    fVS.bindProgramLocations(programId);
    INHERITED::bindProgramLocations(programId);
}
