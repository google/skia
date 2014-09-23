/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFullProgramBuilder.h"
#include "../GrGLGeometryProcessor.h"
#include "../GrGpuGL.h"

GrGLFullProgramBuilder::GrGLFullProgramBuilder(GrGpuGL* gpu,
                                               const GrGLProgramDesc& desc)
    : INHERITED(gpu, desc)
    , fGLGeometryProcessorEmitter(this)
    , fGS(this)
    , fVS(this) {
}

void
GrGLFullProgramBuilder::createAndEmitEffects(const GrGeometryStage* geometryProcessor,
                                             const GrFragmentStage* colorStages[],
                                             const GrFragmentStage* coverageStages[],
                                             GrGLSLExpr4* inputColor,
                                             GrGLSLExpr4* inputCoverage) {
    fVS.emitCodeBeforeEffects(inputColor, inputCoverage);

    ///////////////////////////////////////////////////////////////////////////
    // emit the per-effect code for both color and coverage effects

    EffectKeyProvider colorKeyProvider(&this->desc(), EffectKeyProvider::kColor_EffectType);
    fColorEffects.reset(this->onCreateAndEmitEffects(colorStages,
                                                     this->desc().numColorEffects(),
                                                     colorKeyProvider,
                                                     inputColor));

    if (geometryProcessor) {
        const GrGeometryProcessor& gp = *geometryProcessor->getGeometryProcessor();
        fGLGeometryProcessorEmitter.set(&gp);
        fEffectEmitter = &fGLGeometryProcessorEmitter;
        fVS.emitAttributes(gp);
        GrGLSLExpr4 gpInputCoverage = *inputCoverage;
        GrGLSLExpr4 gpOutputCoverage;
        EffectKeyProvider gpKeyProvider(&this->desc(),
                EffectKeyProvider::kGeometryProcessor_EffectType);
        bool useLocalCoords = this->getVertexShaderBuilder()->hasExplicitLocalCoords();
        fProgramEffects.reset(SkNEW_ARGS(GrGLVertexProgramEffects, (1, useLocalCoords)));
        this->INHERITED::emitEffect(*geometryProcessor, 0, gpKeyProvider, &gpInputCoverage,
                                    &gpOutputCoverage);
        fGeometryProcessor.reset(fProgramEffects.detach());
        *inputCoverage = gpOutputCoverage;
    }

    EffectKeyProvider coverageKeyProvider(&this->desc(), EffectKeyProvider::kCoverage_EffectType);
    fCoverageEffects.reset(this->onCreateAndEmitEffects(coverageStages,
                                                        this->desc().numCoverageEffects(),
                                                        coverageKeyProvider,
                                                        inputCoverage));

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

GrGLProgramEffects* GrGLFullProgramBuilder::onCreateAndEmitEffects(
        const GrFragmentStage* effectStages[],
        int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider,
        GrGLSLExpr4* inOutFSColor) {
    fProgramEffects.reset(SkNEW_ARGS(GrGLVertexProgramEffects,
                                 (effectCnt,
                                  this->getVertexShaderBuilder()->hasExplicitLocalCoords())));
    this->INHERITED::createAndEmitEffects(effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return fProgramEffects.detach();
}

void GrGLFullProgramBuilder::emitEffect(const GrProcessorStage& stage,
                                        const GrProcessorKey& key,
                                        const char* outColor,
                                        const char* inColor,
                                        int stageIndex) {
    SkASSERT(fProgramEffects.get());
    const GrProcessor& effect = *stage.getProcessor();
    SkSTArray<2, GrGLProcessor::TransformedCoords> coords(effect.numTransforms());
    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(effect.numTextures());

    this->emitTransforms(stage, &coords);
    this->emitSamplers(effect, &samplers);

    SkASSERT(fEffectEmitter);
    GrGLProcessor* glEffect = fEffectEmitter->createGLInstance();
    fProgramEffects->addEffect(glEffect);

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("{ // Stage %d: %s\n", stageIndex, glEffect->name());
    fFS.codeAppend(openBrace.c_str());
    fVS.codeAppend(openBrace.c_str());

    fEffectEmitter->emit(key, outColor, inColor, coords, samplers);

    fVS.codeAppend("\t}\n");
    fFS.codeAppend("\t}\n");
}

void GrGLFullProgramBuilder::emitTransforms(const GrProcessorStage& effectStage,
                                            GrGLProcessor::TransformedCoordsArray* outCoords) {
    SkTArray<GrGLVertexProgramEffects::Transform, true>& transforms =
            fProgramEffects->addTransforms();
    const GrProcessor* effect = effectStage.getProcessor();
    int numTransforms = effect->numTransforms();
    transforms.push_back_n(numTransforms);

    SkTArray<GrGLVertexProgramEffects::PathTransform, true>* pathTransforms = NULL;
    const GrGLCaps* glCaps = this->ctxInfo().caps();
    if (glCaps->pathRenderingSupport() &&
        this->gpu()->glPathRendering()->texturingMode() ==
           GrGLPathRendering::SeparableShaders_TexturingMode) {
        pathTransforms = &fProgramEffects->addPathTransforms();
        pathTransforms->push_back_n(numTransforms);
    }

    for (int t = 0; t < numTransforms; t++) {
        const char* uniName = "StageMatrix";
        GrSLType varyingType =
                effectStage.isPerspectiveCoordTransform(t, fVS.hasExplicitLocalCoords()) ?
                        kVec3f_GrSLType :
                        kVec2f_GrSLType;

        SkString suffixedUniName;
        if (0 != t) {
            suffixedUniName.append(uniName);
            suffixedUniName.appendf("_%i", t);
            uniName = suffixedUniName.c_str();
        }
        transforms[t].fHandle = this->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                                 kMat33f_GrSLType,
                                                 uniName,
                                                 &uniName);

        const char* varyingName = "MatrixCoord";
        SkString suffixedVaryingName;
        if (0 != t) {
            suffixedVaryingName.append(varyingName);
            suffixedVaryingName.appendf("_%i", t);
            varyingName = suffixedVaryingName.c_str();
        }
        const char* vsVaryingName;
        const char* fsVaryingName;
        if (pathTransforms) {
            (*pathTransforms)[t].fHandle =
                this->addSeparableVarying(varyingType, varyingName, &vsVaryingName, &fsVaryingName);
            (*pathTransforms)[t].fType = varyingType;
        } else {
            this->addVarying(varyingType, varyingName, &vsVaryingName, &fsVaryingName);
        }

        const GrGLShaderVar& coords =
                kPosition_GrCoordSet == effect->coordTransform(t).sourceCoords() ?
                                          fVS.positionAttribute() :
                                          fVS.localCoordsAttribute();
        // varying = matrix * coords (logically)
        SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
        if (kVec2f_GrSLType == varyingType) {
            fVS.codeAppendf("%s = (%s * vec3(%s, 1)).xy;",
                            vsVaryingName, uniName, coords.c_str());
        } else {
            fVS.codeAppendf("%s = %s * vec3(%s, 1);",
                            vsVaryingName, uniName, coords.c_str());
        }
        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLProcessor::TransformedCoords,
                               (SkString(fsVaryingName), varyingType));
    }
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
