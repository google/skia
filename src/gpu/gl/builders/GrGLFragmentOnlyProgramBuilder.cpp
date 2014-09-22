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
GrGLFragmentOnlyProgramBuilder::createAndEmitEffects(const GrEffectStage* geometryProcessor,
                                                     const GrEffectStage* colorStages[],
                                                     const GrEffectStage* coverageStages[],
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
        const GrEffectStage* effectStages[], int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider, GrGLSLExpr4* inOutFSColor) {

    fProgramEffects.reset(SkNEW_ARGS(GrGLPathTexGenProgramEffects, (effectCnt)));
    this->INHERITED::createAndEmitEffects(effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return fProgramEffects.detach();
}

void GrGLFragmentOnlyProgramBuilder::emitEffect(const GrEffectStage& stage,
                                                 const GrEffectKey& key,
                                                 const char* outColor,
                                                 const char* inColor,
                                                 int stageIndex) {
    SkASSERT(fProgramEffects.get());
    const GrEffect& effect = *stage.getEffect();
    SkASSERT(0 == effect.getVertexAttribs().count());

    SkSTArray<2, GrGLEffect::TransformedCoords> coords(effect.numTransforms());
    SkSTArray<4, GrGLEffect::TextureSampler> samplers(effect.numTextures());

    this->setupPathTexGen(stage, &coords);
    this->emitSamplers(effect, &samplers);

    GrGLEffect* glEffect = effect.getFactory().createGLInstance(effect);
    SkASSERT(!glEffect->isVertexEffect());
    fProgramEffects->addEffect(glEffect);

    GrGLFragmentShaderBuilder* fsBuilder = this->getFragmentShaderBuilder();
    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("\t{ // Stage %d: %s\n", stageIndex, glEffect->name());
    fsBuilder->codeAppend(openBrace.c_str());

    glEffect->emitCode(this, effect, key, outColor, inColor, coords, samplers);

    fsBuilder->codeAppend("\t}\n");
}

void GrGLFragmentOnlyProgramBuilder::setupPathTexGen(const GrEffectStage& effectStage,
                                           GrGLEffect::TransformedCoordsArray* outCoords) {
    int numTransforms = effectStage.getEffect()->numTransforms();
    int texCoordIndex = this->addTexCoordSets(numTransforms);

    fProgramEffects->addTransforms(texCoordIndex);

    SkString name;
    for (int t = 0; t < numTransforms; ++t) {
        GrSLType type =
                effectStage.isPerspectiveCoordTransform(t, false) ?
                        kVec3f_GrSLType :
                        kVec2f_GrSLType;

        name.printf("%s(gl_TexCoord[%i])", GrGLSLTypeString(type), texCoordIndex++);
        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLEffect::TransformedCoords, (name, type));
    }
}
