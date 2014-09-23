/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFragmentOnlyProgramBuilder.h"
#include "../GrGpuGL.h"

GrGLFragmentOnlyProgramBuilder::GrGLFragmentOnlyProgramBuilder(GrGpuGL* gpu,
                                                               const GrGLProgramDesc& desc)
    : INHERITED(gpu, desc) {
    SkASSERT(desc.getHeader().fUseFragShaderOnly);
    SkASSERT(gpu->glCaps().pathRenderingSupport());
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fColorInput);
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fCoverageInput);
}

int GrGLFragmentOnlyProgramBuilder::addTexCoordSets(int count) {
    int firstFreeCoordSet = fTexCoordSetCnt;
    fTexCoordSetCnt += count;
    SkASSERT(gpu()->glCaps().maxFixedFunctionTextureCoords() >= fTexCoordSetCnt);
    return firstFreeCoordSet;
}

void
GrGLFragmentOnlyProgramBuilder::createAndEmitEffects(const GrGeometryStage* geometryProcessor,
                                                     const GrFragmentStage* colorStages[],
                                                     const GrFragmentStage* coverageStages[],
                                                     GrGLSLExpr4* inputColor,
                                                     GrGLSLExpr4* inputCoverage) {
    ///////////////////////////////////////////////////////////////////////////
    // emit the per-effect code for both color and coverage effects

    EffectKeyProvider colorKeyProvider(&this->desc(), EffectKeyProvider::kColor_EffectType);
    fColorEffects.reset(this->onCreateAndEmitEffects(colorStages,
                                                     this->desc().numColorEffects(),
                                                     colorKeyProvider,
                                                     inputColor));

    EffectKeyProvider coverageKeyProvider(&this->desc(), EffectKeyProvider::kCoverage_EffectType);
    fCoverageEffects.reset(this->onCreateAndEmitEffects(coverageStages,
                                                        this->desc().numCoverageEffects(),
                                                        coverageKeyProvider,
                                                        inputCoverage));
}

GrGLProgramEffects* GrGLFragmentOnlyProgramBuilder::onCreateAndEmitEffects(
        const GrFragmentStage* effectStages[], int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider, GrGLSLExpr4* inOutFSColor) {
    fProgramEffects.reset(SkNEW_ARGS(GrGLPathTexGenProgramEffects, (effectCnt)));
    this->INHERITED::createAndEmitEffects(effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return fProgramEffects.detach();
}

void GrGLFragmentOnlyProgramBuilder::emitEffect(const GrProcessorStage& stage,
                                                const GrProcessorKey& key,
                                                const char* outColor,
                                                const char* inColor,
                                                int stageIndex) {
    SkASSERT(fProgramEffects.get());
    const GrProcessor& effect = *stage.getProcessor();

    SkSTArray<2, GrGLProcessor::TransformedCoords> coords(effect.numTransforms());
    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(effect.numTextures());

    this->setupPathTexGen(stage, &coords);
    this->emitSamplers(effect, &samplers);

    SkASSERT(fEffectEmitter);
    GrGLProcessor* glEffect = fEffectEmitter->createGLInstance();
    fProgramEffects->addEffect(glEffect);

    GrGLFragmentShaderBuilder* fsBuilder = this->getFragmentShaderBuilder();
    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("\t{ // Stage %d: %s\n", stageIndex, glEffect->name());
    fsBuilder->codeAppend(openBrace.c_str());

    fEffectEmitter->emit(key, outColor, inColor, coords, samplers);

    fsBuilder->codeAppend("\t}\n");
}

void GrGLFragmentOnlyProgramBuilder::setupPathTexGen(
        const GrProcessorStage& effectStage, GrGLProcessor::TransformedCoordsArray* outCoords) {
    int numTransforms = effectStage.getProcessor()->numTransforms();
    int texCoordIndex = this->addTexCoordSets(numTransforms);

    fProgramEffects->addTransforms(texCoordIndex);

    SkString name;
    for (int t = 0; t < numTransforms; ++t) {
        GrSLType type =
                effectStage.isPerspectiveCoordTransform(t, false) ?
                        kVec3f_GrSLType :
                        kVec2f_GrSLType;

        name.printf("%s(gl_TexCoord[%i])", GrGLSLTypeString(type), texCoordIndex++);
        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLProcessor::TransformedCoords, (name, type));
    }
}
