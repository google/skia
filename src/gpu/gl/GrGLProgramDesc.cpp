/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramDesc.h"
#include "GrBackendEffectFactory.h"
#include "GrDrawEffect.h"
#include "GrEffect.h"
#include "GrGLShaderBuilder.h"
#include "GrGpuGL.h"

void GrGLProgramDesc::Build(const GrDrawState& drawState,
                            bool isPoints,
                            GrDrawState::BlendOptFlags blendOpts,
                            GrBlendCoeff srcCoeff,
                            GrBlendCoeff dstCoeff,
                            const GrGpuGL* gpu,
                            const GrDeviceCoordTexture* dstCopy,
                            GrGLProgramDesc* desc) {

    // This should already have been caught
    GrAssert(!(GrDrawState::kSkipDraw_BlendOptFlag & blendOpts));

    bool skipCoverage = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);

    bool skipColor = SkToBool(blendOpts & (GrDrawState::kEmitTransBlack_BlendOptFlag |
                                           GrDrawState::kEmitCoverage_BlendOptFlag));

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.


    desc->fEmitsPointSize = isPoints;

    bool requiresColorAttrib = !skipColor && drawState.hasColorVertexAttribute();
    bool requiresCoverageAttrib = !skipCoverage && drawState.hasCoverageVertexAttribute();
    // we only need the local coords if we're actually going to generate effect code
    bool requiresLocalCoordAttrib = !(skipCoverage  && skipColor) &&
                                    drawState.hasLocalCoordAttribute();

    // fColorInput/fCoverageInput records how colors are specified for the program so we strip the
    // bits from the bindings to avoid false negatives when searching for an existing program in the
    // cache.

    desc->fColorFilterXfermode = skipColor ? SkXfermode::kDst_Mode : drawState.getColorFilterMode();


    bool colorIsTransBlack = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);
    bool colorIsSolidWhite = (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) ||
                             (!requiresColorAttrib && 0xffffffff == drawState.getColor());
    if (colorIsTransBlack) {
        desc->fColorInput = kTransBlack_ColorInput;
    } else if (colorIsSolidWhite) {
        desc->fColorInput = kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresColorAttrib) {
        desc->fColorInput = kUniform_ColorInput;
    } else {
        desc->fColorInput = kAttribute_ColorInput;
    }

    bool covIsSolidWhite = !requiresCoverageAttrib && 0xffffffff == drawState.getCoverage();

    if (skipCoverage) {
        desc->fCoverageInput = kTransBlack_ColorInput;
    } else if (covIsSolidWhite) {
        desc->fCoverageInput = kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresCoverageAttrib) {
        desc->fCoverageInput = kUniform_ColorInput;
    } else {
        desc->fCoverageInput = kAttribute_ColorInput;
    }

    bool readsDst = false;
    int lastEnabledStage = -1;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {

        bool skip = s < drawState.getFirstCoverageStage() ? skipColor : skipCoverage;
        if (!skip && drawState.isStageEnabled(s)) {
            lastEnabledStage = s;
            const GrEffectRef& effect = *drawState.getStage(s).getEffect();
            const GrBackendEffectFactory& factory = effect->getFactory();
            GrDrawEffect drawEffect(drawState.getStage(s), requiresLocalCoordAttrib);
            desc->fEffectKeys[s] = factory.glEffectKey(drawEffect, gpu->glCaps());
            if (effect->willReadDst()) {
                readsDst = true;
            }
        } else {
            desc->fEffectKeys[s] = 0;
        }
    }

    if (readsDst) {
        GrAssert(NULL != dstCopy);
        desc->fDstRead = GrGLShaderBuilder::KeyForDstRead(dstCopy->texture(), gpu->glCaps());
        GrAssert(0 != desc->fDstRead);
    } else {
        desc->fDstRead = 0;
    }

    desc->fDualSrcOutput = kNone_DualSrcOutput;

    // Currently the experimental GS will only work with triangle prims (and it doesn't do anything
    // other than pass through values from the VS to the FS anyway).
#if GR_GL_EXPERIMENTAL_GS
#if 0
    desc->fExperimentalGS = gpu->caps().geometryShaderSupport();
#else
    desc->fExperimentalGS = false;
#endif
#endif

    // We leave this set to kNumStages until we discover that the coverage/color distinction is
    // material to the generated program. We do this to avoid distinct keys that generate equivalent
    // programs.
    desc->fFirstCoverageStage = GrDrawState::kNumStages;
    // This tracks the actual first coverage stage.
    int firstCoverageStage = GrDrawState::kNumStages;
    desc->fDiscardIfZeroCoverage = false; // Enabled below if stenciling and there is coverage.
    bool hasCoverage = false;
    // If we're rendering coverage-as-color then it's as though there are no coverage stages.
    if (!drawState.isCoverageDrawing()) {
        // We can have coverage either through a stage or coverage vertex attributes.
        if (drawState.getFirstCoverageStage() <= lastEnabledStage) {
            firstCoverageStage = drawState.getFirstCoverageStage();
            hasCoverage = true;
        } else {
            hasCoverage = requiresCoverageAttrib;
        }
    }

    if (hasCoverage) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != desc->fColorFilterXfermode) {
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        // If we're stenciling then we want to discard samples that have zero coverage
        if (drawState.getStencil().doesWrite()) {
            desc->fDiscardIfZeroCoverage = true;
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        if (gpu->caps()->dualSourceBlendingSupport() &&
            !(blendOpts & (GrDrawState::kEmitCoverage_BlendOptFlag |
                           GrDrawState::kCoverageAsAlpha_BlendOptFlag))) {
            if (kZero_GrBlendCoeff == dstCoeff) {
                // write the coverage value to second color
                desc->fDualSrcOutput =  kCoverage_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSA_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = kCoverageISA_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSC_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                desc->fDualSrcOutput = kCoverageISC_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            }
        }
    }

    desc->fPositionAttributeIndex = drawState.positionAttributeIndex();
    desc->fLocalCoordAttributeIndex = drawState.localCoordAttributeIndex();

    // For constant color and coverage we need an attribute with an index beyond those already set
    int availableAttributeIndex = drawState.getVertexAttribCount();
    if (requiresColorAttrib) {
        desc->fColorAttributeIndex = drawState.colorVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == desc->fColorInput) {
        GrAssert(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        desc->fColorAttributeIndex = availableAttributeIndex;
        availableAttributeIndex++;
    } else {
        desc->fColorAttributeIndex = -1;
    }

    if (requiresCoverageAttrib) {
        desc->fCoverageAttributeIndex = drawState.coverageVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == desc->fCoverageInput) {
        GrAssert(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        desc->fCoverageAttributeIndex = availableAttributeIndex;
    } else {
        desc->fCoverageAttributeIndex = -1;
    }
}
