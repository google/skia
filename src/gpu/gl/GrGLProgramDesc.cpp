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

bool GrGLProgramDesc::GetEffectKeyAndUpdateStats(const GrEffectStage& stage,
                                                 const GrGLCaps& caps,
                                                 bool useExplicitLocalCoords,
                                                 GrEffectKeyBuilder* b,
                                                 uint16_t* effectKeySize,
                                                 bool* setTrueIfReadsDst,
                                                 bool* setTrueIfReadsPos,
                                                 bool* setTrueIfHasVertexCode) {
    const GrBackendEffectFactory& factory = stage.getEffect()->getFactory();
    GrDrawEffect drawEffect(stage, useExplicitLocalCoords);
    if (stage.getEffect()->willReadDstColor()) {
        *setTrueIfReadsDst = true;
    }
    if (stage.getEffect()->willReadFragmentPosition()) {
        *setTrueIfReadsPos = true;
    }
    if (stage.getEffect()->hasVertexCode()) {
        *setTrueIfHasVertexCode = true;
    }
    factory.getGLEffectKey(drawEffect, caps, b);
    size_t size = b->size();
    if (size > SK_MaxU16) {
        *effectKeySize = 0; // suppresses a warning.
        return false;
    }
    *effectKeySize = SkToU16(size);
    if (!GrGLProgramEffects::GenEffectMetaKey(drawEffect, caps, b)) {
        return false;
    }
    return true;
}

bool GrGLProgramDesc::Build(const GrDrawState& drawState,
                            GrGpu::DrawType drawType,
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

    int firstEffectiveColorStage = 0;
    bool inputColorIsUsed = true;
    if (!skipColor) {
        firstEffectiveColorStage = drawState.numColorStages();
        while (firstEffectiveColorStage > 0 && inputColorIsUsed) {
            --firstEffectiveColorStage;
            const GrEffect* effect = drawState.getColorStage(firstEffectiveColorStage).getEffect();
            inputColorIsUsed = effect->willUseInputColor();
        }
    }

    int firstEffectiveCoverageStage = 0;
    bool inputCoverageIsUsed = true;
    if (!skipCoverage) {
        firstEffectiveCoverageStage = drawState.numCoverageStages();
        while (firstEffectiveCoverageStage > 0 && inputCoverageIsUsed) {
            --firstEffectiveCoverageStage;
            const GrEffect* effect = drawState.getCoverageStage(firstEffectiveCoverageStage).getEffect();
            inputCoverageIsUsed = effect->willUseInputColor();
        }
    }

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    bool requiresColorAttrib = !skipColor && drawState.hasColorVertexAttribute();
    bool requiresCoverageAttrib = !skipCoverage && drawState.hasCoverageVertexAttribute();
    // we only need the local coords if we're actually going to generate effect code
    bool requiresLocalCoordAttrib = !(skipCoverage  && skipColor) &&
                                    drawState.hasLocalCoordAttribute();

    bool readsDst = false;
    bool readFragPosition = false;
    // We use vertexshader-less shader programs only when drawing paths.
    bool hasVertexCode = !(GrGpu::kDrawPath_DrawType == drawType ||
                           GrGpu::kDrawPaths_DrawType == drawType);
    int numStages = 0;
    if (!skipColor) {
        numStages += drawState.numColorStages() - firstEffectiveColorStage;
    }
    if (!skipCoverage) {
        numStages += drawState.numCoverageStages() - firstEffectiveCoverageStage;
    }
    GR_STATIC_ASSERT(0 == kEffectKeyOffsetsAndLengthOffset % sizeof(uint32_t));
    // Make room for everything up to and including the array of offsets to effect keys.
    desc->fKey.reset();
    desc->fKey.push_back_n(kEffectKeyOffsetsAndLengthOffset + 2 * sizeof(uint16_t) * numStages);

    int offsetAndSizeIndex = 0;

    bool effectKeySuccess = true;
    if (!skipColor) {
        for (int s = firstEffectiveColorStage; s < drawState.numColorStages(); ++s) {
            uint16_t* offsetAndSize =
                reinterpret_cast<uint16_t*>(desc->fKey.begin() + kEffectKeyOffsetsAndLengthOffset +
                                            offsetAndSizeIndex * 2 * sizeof(uint16_t));

            GrEffectKeyBuilder b(&desc->fKey);
            uint16_t effectKeySize;
            uint32_t effectOffset = desc->fKey.count();
            effectKeySuccess |= GetEffectKeyAndUpdateStats(
                                    drawState.getColorStage(s), gpu->glCaps(),
                                    requiresLocalCoordAttrib, &b,
                                    &effectKeySize, &readsDst,
                                    &readFragPosition, &hasVertexCode);
            effectKeySuccess |= (effectOffset <= SK_MaxU16);

            offsetAndSize[0] = SkToU16(effectOffset);
            offsetAndSize[1] = effectKeySize;
            ++offsetAndSizeIndex;
        }
    }
    if (!skipCoverage) {
        for (int s = firstEffectiveCoverageStage; s < drawState.numCoverageStages(); ++s) {
            uint16_t* offsetAndSize =
                reinterpret_cast<uint16_t*>(desc->fKey.begin() + kEffectKeyOffsetsAndLengthOffset +
                                            offsetAndSizeIndex * 2 * sizeof(uint16_t));

            GrEffectKeyBuilder b(&desc->fKey);
            uint16_t effectKeySize;
            uint32_t effectOffset = desc->fKey.count();
            effectKeySuccess |= GetEffectKeyAndUpdateStats(
                                    drawState.getCoverageStage(s), gpu->glCaps(),
                                    requiresLocalCoordAttrib, &b,
                                    &effectKeySize, &readsDst,
                                    &readFragPosition, &hasVertexCode);
            effectKeySuccess |= (effectOffset <= SK_MaxU16);

            offsetAndSize[0] = SkToU16(effectOffset);
            offsetAndSize[1] = effectKeySize;
            ++offsetAndSizeIndex;
        }
    }
    if (!effectKeySuccess) {
        desc->fKey.reset();
        return false;
    }

    KeyHeader* header = desc->header();
    // make sure any padding in the header is zeroed.
    memset(desc->header(), 0, kHeaderSize);

    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.

    header->fHasVertexCode = hasVertexCode || requiresLocalCoordAttrib;
    header->fEmitsPointSize = GrGpu::kDrawPoints_DrawType == drawType;

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

    if (defaultToUniformInputs && !requiresColorAttrib && inputColorIsUsed) {
        header->fColorInput = kUniform_ColorInput;
    } else {
        header->fColorInput = kAttribute_ColorInput;
        header->fHasVertexCode = true;
    }

    bool covIsSolidWhite = !requiresCoverageAttrib && 0xffffffff == drawState.getCoverageColor();

    if ((covIsSolidWhite || !inputCoverageIsUsed) && !skipCoverage) {
        header->fCoverageInput = kSolidWhite_ColorInput;
    } else if (defaultToUniformInputs && !requiresCoverageAttrib && inputCoverageIsUsed) {
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

    // Set this default and then possibly change our mind if there is coverage.
    header->fCoverageOutput = kModulate_CoverageOutput;

    // If we do have coverage determine whether it matters.
    bool separateCoverageFromColor = false;
    if (!drawState.isCoverageDrawing() && !skipCoverage &&
        (drawState.numCoverageStages() > 0 || requiresCoverageAttrib)) {

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
        for (int s = firstEffectiveColorStage; s < drawState.numColorStages(); ++s) {
            colorStages->push_back(&drawState.getColorStage(s));
        }
    }
    if (!skipCoverage) {
        SkTArray<const GrEffectStage*, true>* array;
        if (separateCoverageFromColor) {
            array = coverageStages;
        } else {
            array = colorStages;
        }
        for (int s = firstEffectiveCoverageStage; s < drawState.numCoverageStages(); ++s) {
            array->push_back(&drawState.getCoverageStage(s));
        }
    }
    header->fColorEffectCnt = colorStages->count();
    header->fCoverageEffectCnt = coverageStages->count();

    desc->finalize();
    return true;
}

void GrGLProgramDesc::finalize() {
    int keyLength = fKey.count();
    SkASSERT(0 == (keyLength % 4));
    *this->atOffset<uint32_t, kLengthOffset>() = SkToU32(keyLength);

    uint32_t* checksum = this->atOffset<uint32_t, kChecksumOffset>();
    *checksum = 0;
    *checksum = SkChecksum::Compute(reinterpret_cast<uint32_t*>(fKey.begin()), keyLength);
}

GrGLProgramDesc& GrGLProgramDesc::operator= (const GrGLProgramDesc& other) {
    size_t keyLength = other.keyLength();
    fKey.reset(keyLength);
    memcpy(fKey.begin(), other.fKey.begin(), keyLength);
    return *this;
}
