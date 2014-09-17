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
    SkASSERT(!desc.getHeader().fRequiresVertexShader);
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

GrGLProgramEffects* GrGLFragmentOnlyProgramBuilder::createAndEmitEffects(
        const GrEffectStage* effectStages[], int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider, GrGLSLExpr4* inOutFSColor) {

    GrGLPathTexGenProgramEffectsBuilder pathTexGenEffectsBuilder(this,
                                                                 effectCnt);
    this->INHERITED::createAndEmitEffects(&pathTexGenEffectsBuilder,
                                          effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return pathTexGenEffectsBuilder.finish();
}
