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

#include "SkChecksum.h"

namespace {
inline GrGLEffect::EffectKey get_key_and_update_stats(const GrEffectStage& stage,
                                                      const GrGLCaps& caps,
                                                      bool useExplicitLocalCoords,
                                                      bool* setTrueIfReadsDst,
                                                      bool* setTrueIfReadsPos,
                                                      bool* setTrueIfHasVertexCode) {
    const GrEffectRef& effect = *stage.getEffect();
    const GrBackendEffectFactory& factory = effect->getFactory();
    GrDrawEffect drawEffect(stage, useExplicitLocalCoords);
    if (effect->willReadDstColor()) {
        *setTrueIfReadsDst = true;
    }
    if (effect->willReadFragmentPosition()) {
        *setTrueIfReadsPos = true;
    }
    if (effect->hasVertexCode()) {
        *setTrueIfHasVertexCode = true;
    }
    return factory.glEffectKey(drawEffect, caps);
}
}
void GrGLProgramDesc::Build(const GrDrawState& drawState,
                            bool isPoints,
                            GrDrawState::BlendOptFlags blendOpts,
                            GrBlendCoeff srcCoeff,
                            GrBlendCoeff dstCoeff,
                            const GrGpuGL* gpu,
                            const GrDeviceCoordTexture* dstCopy,
                            SkTArray<const GrEffectStage*, true>* colorStages,
                            SkTArray<const GrEffectStage*, true>* coverageStages,
                            GrGLProgramDesc* desc) {
    colorStages->reset();
    coverageStages->reset();

    // This should already have been caught
    SkASSERT(!(GrDrawState::kSkipDraw_BlendOptFlag & blendOpts));

    bool skipCoverage = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);

    bool skipColor = SkToBool(blendOpts & (GrDrawState::kEmitTransBlack_BlendOptFlag |
                                           GrDrawState::kEmitCoverage_BlendOptFlag));

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    bool requiresColorAttrib = !skipColor && drawState.hasColorVertexAttribute();
    bool requiresCoverageAttrib = !skipCoverage && drawState.hasCoverageVertexAttribute();
    // we only need the local coords if we're actually going to generate effect code
    bool requiresLocalCoordAttrib = !(skipCoverage  && skipColor) &&
                                    drawState.hasLocalCoordAttribute();

    bool colorIsTransBlack = SkToBool(blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag);
    bool colorIsSolidWhite = (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) ||
                             (!requiresColorAttrib && 0xffffffff == drawState.getColor());

    int numEffects = (skipColor ? 0 : drawState.numColorStages()) +
                     (skipCoverage ? 0 : drawState.numCoverageStages());

    size_t newKeyLength = KeyLength(numEffects);
    bool allocChanged;
    desc->fKey.reset(newKeyLength, SkAutoMalloc::kAlloc_OnShrink, &allocChanged);
    if (allocChanged || !desc->fInitialized) {
        // make sure any padding in the header is zero if we we haven't used this allocation before.
        memset(desc->header(), 0, kHeaderSize);
    }
    // write the key length
    *desc->atOffset<uint32_t, kLengthOffset>() = newKeyLength;

    KeyHeader* header = desc->header();
    EffectKey* effectKeys = desc->effectKeys();

    int currEffectKey = 0;
    bool readsDst = false;
    bool readFragPosition = false;
    bool hasVertexCode = false;
    if (!skipColor) {
        for (int s = 0; s < drawState.numColorStages(); ++s) {
            effectKeys[currEffectKey++] =
                get_key_and_update_stats(drawState.getColorStage(s), gpu->glCaps(),
                                         requiresLocalCoordAttrib, &readsDst, &readFragPosition,
                                         &hasVertexCode);
        }
    }
    if (!skipCoverage) {
        for (int s = 0; s < drawState.numCoverageStages(); ++s) {
            effectKeys[currEffectKey++] =
                get_key_and_update_stats(drawState.getCoverageStage(s), gpu->glCaps(),
                                         requiresLocalCoordAttrib, &readsDst, &readFragPosition,
                                         &hasVertexCode);
        }
    }

    header->fHasVertexCode = hasVertexCode || requiresLocalCoordAttrib;
    header->fEmitsPointSize = isPoints;
    header->fColorFilterXfermode = skipColor ? SkXfermode::kDst_Mode : drawState.getColorFilterMode();

    // Currently the experimental GS will only work with triangle prims (and it doesn't do anything
    // other than pass through values from the VS to the FS anyway).
#if GR_GL_EXPERIMENTAL_GS
#if 0
    header->fExperimentalGS = gpu->caps().geometryShaderSupport();
#else
    header->fExperimentalGS = false;
#endif
#endif
    bool defaultToUniformInputs = GR_GL_NO_CONSTANT_ATTRIBUTES || gpu->caps()->pathRenderingSupport();

    if (colorIsTransBlack) {
        header->fColorInput = kTransBlack_ColorInput;
    } else if (colorIsSolidWhite) {
        header->fColorInput = kSolidWhite_ColorInput;
    } else if (defaultToUniformInputs && !requiresColorAttrib) {
        header->fColorInput = kUniform_ColorInput;
    } else {
        header->fColorInput = kAttribute_ColorInput;
        header->fHasVertexCode = true;
    }

    bool covIsSolidWhite = !requiresCoverageAttrib && 0xffffffff == drawState.getCoverage();

    if (skipCoverage) {
        header->fCoverageInput = kTransBlack_ColorInput;
    } else if (covIsSolidWhite) {
        header->fCoverageInput = kSolidWhite_ColorInput;
    } else if (defaultToUniformInputs && !requiresCoverageAttrib) {
        header->fCoverageInput = kUniform_ColorInput;
    } else {
        header->fCoverageInput = kAttribute_ColorInput;
        header->fHasVertexCode = true;
    }

    if (readsDst) {
        SkASSERT(NULL != dstCopy || gpu->caps()->dstReadInShaderSupport());
        const GrTexture* dstCopyTexture = NULL;
        if (NULL != dstCopy) {
            dstCopyTexture = dstCopy->texture();
        }
        header->fDstReadKey = GrGLShaderBuilder::KeyForDstRead(dstCopyTexture, gpu->glCaps());
        SkASSERT(0 != header->fDstReadKey);
    } else {
        header->fDstReadKey = 0;
    }

    if (readFragPosition) {
        header->fFragPosKey = GrGLShaderBuilder::KeyForFragmentPosition(drawState.getRenderTarget(),
                                                                      gpu->glCaps());
    } else {
        header->fFragPosKey = 0;
    }

    // Record attribute indices
    header->fPositionAttributeIndex = drawState.positionAttributeIndex();
    header->fLocalCoordAttributeIndex = drawState.localCoordAttributeIndex();

    // For constant color and coverage we need an attribute with an index beyond those already set
    int availableAttributeIndex = drawState.getVertexAttribCount();
    if (requiresColorAttrib) {
        header->fColorAttributeIndex = drawState.colorVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == header->fColorInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fColorAttributeIndex = availableAttributeIndex;
        availableAttributeIndex++;
    } else {
        header->fColorAttributeIndex = -1;
    }

    if (requiresCoverageAttrib) {
        header->fCoverageAttributeIndex = drawState.coverageVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == header->fCoverageInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fCoverageAttributeIndex = availableAttributeIndex;
    } else {
        header->fCoverageAttributeIndex = -1;
    }

    // Here we deal with whether/how we handle color and coverage separately.

    // Set these defaults and then possibly change our mind if there is coverage.
    header->fDiscardIfZeroCoverage = false;
    header->fCoverageOutput = kModulate_CoverageOutput;

    // If we do have coverage determine whether it matters.
    bool separateCoverageFromColor = false;
    if (!drawState.isCoverageDrawing() && !skipCoverage &&
        (drawState.numCoverageStages() > 0 || requiresCoverageAttrib)) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != header->fColorFilterXfermode) {
            separateCoverageFromColor = true;
        }

        // If we're stenciling then we want to discard samples that have zero coverage
        if (drawState.getStencil().doesWrite()) {
            header->fDiscardIfZeroCoverage = true;
            separateCoverageFromColor = true;
        }

        if (gpu->caps()->dualSourceBlendingSupport() &&
            !(blendOpts & (GrDrawState::kEmitCoverage_BlendOptFlag |
                           GrDrawState::kCoverageAsAlpha_BlendOptFlag))) {
            if (kZero_GrBlendCoeff == dstCoeff) {
                // write the coverage value to second color
                header->fCoverageOutput =  kSecondaryCoverage_CoverageOutput;
                separateCoverageFromColor = true;
            } else if (kSA_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                header->fCoverageOutput = kSecondaryCoverageISA_CoverageOutput;
                separateCoverageFromColor = true;
            } else if (kSC_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                header->fCoverageOutput = kSecondaryCoverageISC_CoverageOutput;
                separateCoverageFromColor = true;
            }
        } else if (readsDst &&
                   kOne_GrBlendCoeff == srcCoeff &&
                   kZero_GrBlendCoeff == dstCoeff) {
            header->fCoverageOutput = kCombineWithDst_CoverageOutput;
            separateCoverageFromColor = true;
        }
    }
    if (!skipColor) {
        for (int s = 0; s < drawState.numColorStages(); ++s) {
            colorStages->push_back(&drawState.getColorStage(s));
        }
        header->fColorEffectCnt = drawState.numColorStages();
    }
    if (!skipCoverage) {
        SkTArray<const GrEffectStage*, true>* array;
        if (separateCoverageFromColor) {
            array = coverageStages;
            header->fCoverageEffectCnt = drawState.numCoverageStages();
        } else {
            array = colorStages;
            header->fColorEffectCnt += drawState.numCoverageStages();
        }
        for (int s = 0; s < drawState.numCoverageStages(); ++s) {
            array->push_back(&drawState.getCoverageStage(s));
        }
    }

    *desc->checksum() = 0;
    *desc->checksum() = SkChecksum::Compute(reinterpret_cast<uint32_t*>(desc->fKey.get()),
                                            newKeyLength);
    desc->fInitialized = true;
}

GrGLProgramDesc& GrGLProgramDesc::operator= (const GrGLProgramDesc& other) {
    fInitialized = other.fInitialized;
    if (fInitialized) {
        size_t keyLength = other.keyLength();
        fKey.reset(keyLength);
        memcpy(fKey.get(), other.fKey.get(), keyLength);
    }
    return *this;
}
