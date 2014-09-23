/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrGLProgramDesc.h"
#include "GrBackendEffectFactory.h"
#include "GrEffect.h"
#include "GrGpuGL.h"
#include "GrOptDrawState.h"

#include "SkChecksum.h"

/**
 * The key for an individual coord transform is made up of a matrix type and a bit that
 * indicates the source of the input coords.
 */
enum {
    kMatrixTypeKeyBits   = 1,
    kMatrixTypeKeyMask   = (1 << kMatrixTypeKeyBits) - 1,
    kPositionCoords_Flag = (1 << kMatrixTypeKeyBits),
    kTransformKeyBits    = kMatrixTypeKeyBits + 1,
};

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

/**
 * Do we need to either map r,g,b->a or a->r. configComponentMask indicates which channels are
 * present in the texture's config. swizzleComponentMask indicates the channels present in the
 * shader swizzle.
 */
static bool swizzle_requires_alpha_remapping(const GrGLCaps& caps,
                                             uint32_t configComponentMask,
                                             uint32_t swizzleComponentMask) {
    if (caps.textureSwizzleSupport()) {
        // Any remapping is handled using texture swizzling not shader modifications.
        return false;
    }
    // check if the texture is alpha-only
    if (kA_GrColorComponentFlag == configComponentMask) {
        if (caps.textureRedSupport() && (kA_GrColorComponentFlag & swizzleComponentMask)) {
            // we must map the swizzle 'a's to 'r'.
            return true;
        }
        if (kRGB_GrColorComponentFlags & swizzleComponentMask) {
            // The 'r', 'g', and/or 'b's must be mapped to 'a' according to our semantics that
            // alpha-only textures smear alpha across all four channels when read.
            return true;
        }
    }
    return false;
}

static uint32_t gen_attrib_key(const GrEffect* effect) {
    uint32_t key = 0;

    const GrEffect::VertexAttribArray& vars = effect->getVertexAttribs();
    int numAttributes = vars.count();
    SkASSERT(numAttributes <= 2);
    for (int a = 0; a < numAttributes; ++a) {
        uint32_t value = 1 << a;
        key |= value;
    }
    return key;
}

static uint32_t gen_transform_key(const GrEffectStage& effectStage,
                                  bool useExplicitLocalCoords) {
    uint32_t totalKey = 0;
    int numTransforms = effectStage.getEffect()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        uint32_t key = 0;
        if (effectStage.isPerspectiveCoordTransform(t, useExplicitLocalCoords)) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }

        const GrCoordTransform& coordTransform = effectStage.getEffect()->coordTransform(t);
        if (kLocal_GrCoordSet != coordTransform.sourceCoords() && useExplicitLocalCoords) {
            key |= kPositionCoords_Flag;
        }
        key <<= kTransformKeyBits * t;
        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}

static uint32_t gen_texture_key(const GrEffect* effect, const GrGLCaps& caps) {
    uint32_t key = 0;
    int numTextures = effect->numTextures();
    for (int t = 0; t < numTextures; ++t) {
        const GrTextureAccess& access = effect->textureAccess(t);
        uint32_t configComponentMask = GrPixelConfigComponentMask(access.getTexture()->config());
        if (swizzle_requires_alpha_remapping(caps, configComponentMask, access.swizzleMask())) {
            key |= 1 << t;
        }
    }
    return key;
}

/**
 * A function which emits a meta key into the key builder.  This is required because shader code may
 * be dependent on properties of the effect that the effect itself doesn't use
 * in its key (e.g. the pixel format of textures used). So we create a meta-key for
 * every effect using this function. It is also responsible for inserting the effect's class ID
 * which must be different for every GrEffect subclass. It can fail if an effect uses too many
 * textures, attributes, etc for the space allotted in the meta-key.
 */

static bool gen_effect_meta_key(const GrEffectStage& effectStage,
                                bool useExplicitLocalCoords,
                                const GrGLCaps& caps,
                                GrEffectKeyBuilder* b) {

    uint32_t textureKey = gen_texture_key(effectStage.getEffect(), caps);
    uint32_t transformKey = gen_transform_key(effectStage,useExplicitLocalCoords);
    uint32_t attribKey = gen_attrib_key(effectStage.getEffect());
    uint32_t classID = effectStage.getEffect()->getFactory().effectClassID();

    // Currently we allow 16 bits for each of the above portions of the meta-key. Fail if they
    // don't fit.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
    if ((textureKey | transformKey | attribKey | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    uint32_t* key = b->add32n(2);
    key[0] = (textureKey << 16 | transformKey);
    key[1] = (classID << 16 | attribKey);
    return true;
}

bool GrGLProgramDesc::GetEffectKey(const GrEffectStage& stage, const GrGLCaps& caps,
                                   bool useExplicitLocalCoords, GrEffectKeyBuilder* b,
                                   uint16_t* effectKeySize) {
    const GrBackendEffectFactory& factory = stage.getEffect()->getFactory();
    const GrEffect& effect = *stage.getEffect();
    factory.getGLEffectKey(effect, caps, b);
    size_t size = b->size();
    if (size > SK_MaxU16) {
        *effectKeySize = 0; // suppresses a warning.
        return false;
    }
    *effectKeySize = SkToU16(size);
    if (!gen_effect_meta_key(stage, useExplicitLocalCoords, caps, b)) {
        return false;
    }
    return true;
}

bool GrGLProgramDesc::Build(const GrOptDrawState& optState,
                            GrGpu::DrawType drawType,
                            GrBlendCoeff srcCoeff,
                            GrBlendCoeff dstCoeff,
                            GrGpuGL* gpu,
                            const GrDeviceCoordTexture* dstCopy,
                            const GrEffectStage** geometryProcessor,
                            SkTArray<const GrEffectStage*, true>* colorStages,
                            SkTArray<const GrEffectStage*, true>* coverageStages,
                            GrGLProgramDesc* desc) {
    colorStages->reset();
    coverageStages->reset();

    bool inputColorIsUsed = optState.inputColorIsUsed();
    bool inputCoverageIsUsed = optState.inputCoverageIsUsed();

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    bool requiresColorAttrib = optState.hasColorVertexAttribute();
    bool requiresCoverageAttrib = optState.hasCoverageVertexAttribute();
    bool requiresLocalCoordAttrib = optState.requiresLocalCoordAttrib();

    int numStages = optState.numTotalStages();

    GR_STATIC_ASSERT(0 == kEffectKeyOffsetsAndLengthOffset % sizeof(uint32_t));
    // Make room for everything up to and including the array of offsets to effect keys.
    desc->fKey.reset();
    desc->fKey.push_back_n(kEffectKeyOffsetsAndLengthOffset + 2 * sizeof(uint16_t) * numStages);

    int offsetAndSizeIndex = 0;
    bool effectKeySuccess = true;

    KeyHeader* header = desc->header();
    // make sure any padding in the header is zeroed.
    memset(desc->header(), 0, kHeaderSize);

    // We can only have one effect which touches the vertex shader
    if (optState.hasGeometryProcessor()) {
        uint16_t* offsetAndSize =
                reinterpret_cast<uint16_t*>(desc->fKey.begin() + kEffectKeyOffsetsAndLengthOffset +
                                            offsetAndSizeIndex * 2 * sizeof(uint16_t));

            GrEffectKeyBuilder b(&desc->fKey);
            uint16_t effectKeySize;
            uint32_t effectOffset = desc->fKey.count();
            effectKeySuccess |= GetEffectKey(*optState.getGeometryProcessor(), gpu->glCaps(),
                                             requiresLocalCoordAttrib, &b, &effectKeySize);
            effectKeySuccess |= (effectOffset <= SK_MaxU16);

            offsetAndSize[0] = SkToU16(effectOffset);
            offsetAndSize[1] = effectKeySize;
            ++offsetAndSizeIndex;
            *geometryProcessor = optState.getGeometryProcessor();
            header->fHasGeometryProcessor = true;
    }

    for (int s = 0; s < optState.numColorStages(); ++s) {
        uint16_t* offsetAndSize =
            reinterpret_cast<uint16_t*>(desc->fKey.begin() + kEffectKeyOffsetsAndLengthOffset +
                                        offsetAndSizeIndex * 2 * sizeof(uint16_t));

        GrEffectKeyBuilder b(&desc->fKey);
        uint16_t effectKeySize;
        uint32_t effectOffset = desc->fKey.count();
        effectKeySuccess |= GetEffectKey(optState.getColorStage(s), gpu->glCaps(),
                                         requiresLocalCoordAttrib, &b, &effectKeySize);
        effectKeySuccess |= (effectOffset <= SK_MaxU16);

        offsetAndSize[0] = SkToU16(effectOffset);
        offsetAndSize[1] = effectKeySize;
        ++offsetAndSizeIndex;
    }

    for (int s = 0; s < optState.numCoverageStages(); ++s) {
        uint16_t* offsetAndSize =
            reinterpret_cast<uint16_t*>(desc->fKey.begin() + kEffectKeyOffsetsAndLengthOffset +
                                        offsetAndSizeIndex * 2 * sizeof(uint16_t));

        GrEffectKeyBuilder b(&desc->fKey);
        uint16_t effectKeySize;
        uint32_t effectOffset = desc->fKey.count();
        effectKeySuccess |= GetEffectKey(optState.getCoverageStage(s), gpu->glCaps(),
                                         requiresLocalCoordAttrib, &b, &effectKeySize);
        effectKeySuccess |= (effectOffset <= SK_MaxU16);

        offsetAndSize[0] = SkToU16(effectOffset);
        offsetAndSize[1] = effectKeySize;
        ++offsetAndSizeIndex;
    }

    if (!effectKeySuccess) {
        desc->fKey.reset();
        return false;
    }

    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.

    header->fUseFragShaderOnly = gpu->caps()->pathRenderingSupport() &&
        GrGpu::IsPathRenderingDrawType(drawType) &&
        gpu->glPathRendering()->texturingMode() == GrGLPathRendering::FixedFunction_TexturingMode;
    SkASSERT(!header->fUseFragShaderOnly || !optState.hasGeometryProcessor());

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
    bool defaultToUniformInputs = GR_GL_NO_CONSTANT_ATTRIBUTES || header->fUseFragShaderOnly;

    if (!inputColorIsUsed) {
        header->fColorInput = kAllOnes_ColorInput;
    } else if (defaultToUniformInputs && !requiresColorAttrib) {
        header->fColorInput = kUniform_ColorInput;
    } else {
        header->fColorInput = kAttribute_ColorInput;
        header->fUseFragShaderOnly = false;
    }

    bool covIsSolidWhite = !requiresCoverageAttrib && 0xffffffff == optState.getCoverageColor();

    if (covIsSolidWhite || !inputCoverageIsUsed) {
        header->fCoverageInput = kAllOnes_ColorInput;
    } else if (defaultToUniformInputs && !requiresCoverageAttrib) {
        header->fCoverageInput = kUniform_ColorInput;
    } else {
        header->fCoverageInput = kAttribute_ColorInput;
        header->fUseFragShaderOnly = false;
    }

    if (optState.readsDst()) {
        SkASSERT(dstCopy || gpu->caps()->dstReadInShaderSupport());
        const GrTexture* dstCopyTexture = NULL;
        if (dstCopy) {
            dstCopyTexture = dstCopy->texture();
        }
        header->fDstReadKey = GrGLFragmentShaderBuilder::KeyForDstRead(dstCopyTexture,
                gpu->glCaps());
        SkASSERT(0 != header->fDstReadKey);
    } else {
        header->fDstReadKey = 0;
    }

    if (optState.readsFragPosition()) {
        header->fFragPosKey = GrGLFragmentShaderBuilder::KeyForFragmentPosition(
                optState.getRenderTarget(), gpu->glCaps());
    } else {
        header->fFragPosKey = 0;
    }

    // Record attribute indices
    header->fPositionAttributeIndex = optState.positionAttributeIndex();
    header->fLocalCoordAttributeIndex = optState.localCoordAttributeIndex();

    // For constant color and coverage we need an attribute with an index beyond those already set
    int availableAttributeIndex = optState.getVertexAttribCount();
    if (requiresColorAttrib) {
        header->fColorAttributeIndex = optState.colorVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == header->fColorInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fColorAttributeIndex = availableAttributeIndex;
        availableAttributeIndex++;
    } else {
        header->fColorAttributeIndex = -1;
    }

    if (requiresCoverageAttrib) {
        header->fCoverageAttributeIndex = optState.coverageVertexAttributeIndex();
    } else if (GrGLProgramDesc::kAttribute_ColorInput == header->fCoverageInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fCoverageAttributeIndex = availableAttributeIndex;
    } else {
        header->fCoverageAttributeIndex = -1;
    }

    header->fPrimaryOutputType = optState.getPrimaryOutputType();
    header->fSecondaryOutputType = optState.getSecondaryOutputType();

    for (int s = 0; s < optState.numColorStages(); ++s) {
        colorStages->push_back(&optState.getColorStage(s));
    }
    for (int s = 0; s < optState.numCoverageStages(); ++s) {
        coverageStages->push_back(&optState.getCoverageStage(s));
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
