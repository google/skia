/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrGLProgramDesc.h"

#include "GrGLProcessor.h"
#include "GrBackendProcessorFactory.h"
#include "GrProcessor.h"
#include "GrGpuGL.h"
#include "GrOptDrawState.h"
#include "SkChecksum.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"

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

static uint32_t gen_attrib_key(const GrGeometryProcessor& proc) {
    uint32_t key = 0;

    const GrGeometryProcessor::VertexAttribArray& vars = proc.getVertexAttribs();
    int numAttributes = vars.count();
    SkASSERT(numAttributes <= 2);
    for (int a = 0; a < numAttributes; ++a) {
        uint32_t value = 1 << a;
        key |= value;
    }
    return key;
}

static uint32_t gen_transform_key(const GrFragmentStage& effectStage,
                                  bool useExplicitLocalCoords) {
    uint32_t totalKey = 0;
    int numTransforms = effectStage.getProcessor()->numTransforms();
    for (int t = 0; t < numTransforms; ++t) {
        uint32_t key = 0;
        if (effectStage.isPerspectiveCoordTransform(t, useExplicitLocalCoords)) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }

        const GrCoordTransform& coordTransform = effectStage.getProcessor()->coordTransform(t);
        if (kLocal_GrCoordSet != coordTransform.sourceCoords() && useExplicitLocalCoords) {
            key |= kPositionCoords_Flag;
        }
        key <<= kTransformKeyBits * t;
        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}

static uint32_t gen_texture_key(const GrProcessor& proc, const GrGLCaps& caps) {
    uint32_t key = 0;
    int numTextures = proc.numTextures();
    for (int t = 0; t < numTextures; ++t) {
        const GrTextureAccess& access = proc.textureAccess(t);
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
 * which must be different for every GrProcessor subclass. It can fail if an effect uses too many
 * textures, transforms, etc, for the space allotted in the meta-key.  NOTE, both FPs and GPs share
 * this function because it is hairy, though FPs do not have attribs, and GPs do not have transforms
 */
static bool get_meta_key(const GrProcessor& proc,
                         const GrGLCaps& caps,
                         uint32_t transformKey,
                         uint32_t attribKey,
                         GrProcessorKeyBuilder* b,
                         uint16_t* processorKeySize) {
    const GrBackendProcessorFactory& factory = proc.getFactory();
    factory.getGLProcessorKey(proc, caps, b);
    size_t size = b->size();
    if (size > SK_MaxU16) {
        *processorKeySize = 0; // suppresses a warning.
        return false;
    }
    *processorKeySize = SkToU16(size);
    uint32_t textureKey = gen_texture_key(proc, caps);
    uint32_t classID = proc.getFactory().classID();

    // Currently we allow 16 bits for each of the above portions of the meta-key. Fail if they
    // don't fit.
    static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
    if ((textureKey | transformKey | classID) & kMetaKeyInvalidMask) {
        return false;
    }

    uint32_t* key = b->add32n(2);
    key[0] = (textureKey << 16 | transformKey);
    key[1] = (classID << 16);
    return true;
}

struct GeometryProcessorKeyBuilder {
    typedef GrGeometryProcessor StagedProcessor;
    static bool GetProcessorKey(const GrGeometryProcessor& gp,
                                const GrGLCaps& caps,
                                bool,
                                GrProcessorKeyBuilder* b,
                                uint16_t* keySize) {
        /* 0 because no transforms on a GP */
        return get_meta_key(gp, caps, 0, gen_attrib_key(gp), b, keySize);
    }
};

struct FragmentProcessorKeyBuilder {
    typedef GrFragmentStage StagedProcessor;
    static bool GetProcessorKey(const GrFragmentStage& fps,
                                const GrGLCaps& caps,
                                bool useLocalCoords,
                                GrProcessorKeyBuilder* b,
                                uint16_t* keySize) {
        /* 0 because no attribs on a fP */
        return get_meta_key(*fps.getProcessor(), caps, gen_transform_key(fps, useLocalCoords), 0,
                            b, keySize);
    }
};


template <class ProcessorKeyBuilder>
bool
GrGLProgramDescBuilder::BuildStagedProcessorKey(const typename ProcessorKeyBuilder::StagedProcessor& stage,
                                         const GrGLCaps& caps,
                                         bool requiresLocalCoordAttrib,
                                         GrProgramDesc* desc,
                                         int* offsetAndSizeIndex) {
    GrProcessorKeyBuilder b(&desc->fKey);
    uint16_t processorKeySize;
    uint32_t processorOffset = desc->fKey.count();
    if (processorOffset > SK_MaxU16 ||
            !ProcessorKeyBuilder::GetProcessorKey(stage, caps, requiresLocalCoordAttrib, &b,
                                                  &processorKeySize)){
        desc->fKey.reset();
        return false;
    }

    uint16_t* offsetAndSize =
            reinterpret_cast<uint16_t*>(desc->fKey.begin() + kProcessorKeyOffsetsAndLengthOffset +
                                        *offsetAndSizeIndex * 2 * sizeof(uint16_t));
    offsetAndSize[0] = SkToU16(processorOffset);
    offsetAndSize[1] = processorKeySize;
    ++(*offsetAndSizeIndex);
    return true;
}

bool GrGLProgramDescBuilder::Build(const GrOptDrawState& optState,
                            const GrProgramDesc::DescInfo& descInfo,
                            GrGpu::DrawType drawType,
                            GrGpuGL* gpu,
                            const GrDeviceCoordTexture* dstCopy,
                            GrProgramDesc* desc) {
    bool inputColorIsUsed = descInfo.fInputColorIsUsed;
    bool inputCoverageIsUsed = descInfo.fInputCoverageIsUsed;

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the attribute
    // bindings in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    bool requiresLocalCoordAttrib = descInfo.fRequiresLocalCoordAttrib;

    int numStages = optState.numTotalStages();

    GR_STATIC_ASSERT(0 == kProcessorKeyOffsetsAndLengthOffset % sizeof(uint32_t));
    // Make room for everything up to and including the array of offsets to effect keys.
    desc->fKey.reset();
    desc->fKey.push_back_n(kProcessorKeyOffsetsAndLengthOffset + 2 * sizeof(uint16_t) * numStages);

    int offsetAndSizeIndex = 0;

    // We can only have one effect which touches the vertex shader
    if (optState.hasGeometryProcessor()) {
        if (!BuildStagedProcessorKey<GeometryProcessorKeyBuilder>(*optState.getGeometryProcessor(),
                                                                  gpu->glCaps(),
                                                                  false,
                                                                  desc,
                                                                  &offsetAndSizeIndex)) {
            return false;
        }
    }

    for (int s = 0; s < optState.numFragmentStages(); ++s) {
        if (!BuildStagedProcessorKey<FragmentProcessorKeyBuilder>(optState.getFragmentStage(s),
                                                                  gpu->glCaps(),
                                                                  requiresLocalCoordAttrib,
                                                                  desc,
                                                                  &offsetAndSizeIndex)) {
            return false;
        }
    }

    // --------DO NOT MOVE HEADER ABOVE THIS LINE--------------------------------------------------
    // Because header is a pointer into the dynamic array, we can't push any new data into the key
    // below here.
    GLKeyHeader* header = desc->atOffset<GLKeyHeader, kHeaderOffset>();

    // make sure any padding in the header is zeroed.
    memset(header, 0, kHeaderSize);

    header->fHasGeometryProcessor = optState.hasGeometryProcessor();

    header->fEmitsPointSize = GrGpu::kDrawPoints_DrawType == drawType;

    bool isPathRendering = GrGpu::IsPathRenderingDrawType(drawType);
    if (gpu->caps()->pathRenderingSupport() && isPathRendering) {
        header->fUseNvpr = true;
        SkASSERT(!optState.hasGeometryProcessor());
    } else {
        header->fUseNvpr = false;
    }

    bool hasUniformColor = inputColorIsUsed &&
                           (isPathRendering || !descInfo.hasColorVertexAttribute());

    bool hasUniformCoverage = inputCoverageIsUsed &&
                              (isPathRendering || !descInfo.hasCoverageVertexAttribute());

    if (!inputColorIsUsed) {
        header->fColorInput = GrProgramDesc::kAllOnes_ColorInput;
    } else if (hasUniformColor) {
        header->fColorInput = GrProgramDesc::kUniform_ColorInput;
    } else {
        header->fColorInput = GrProgramDesc::kAttribute_ColorInput;
        SkASSERT(!header->fUseNvpr);
    }

    bool covIsSolidWhite = !descInfo.hasCoverageVertexAttribute() &&
                           0xffffffff == optState.getCoverageColor();

    if (covIsSolidWhite || !inputCoverageIsUsed) {
        header->fCoverageInput = GrProgramDesc::kAllOnes_ColorInput;
    } else if (hasUniformCoverage) {
        header->fCoverageInput = GrProgramDesc::kUniform_ColorInput;
    } else {
        header->fCoverageInput = GrProgramDesc::kAttribute_ColorInput;
        SkASSERT(!header->fUseNvpr);
    }

    if (descInfo.fReadsDst) {
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

    if (descInfo.fReadsFragPosition) {
        header->fFragPosKey =
                GrGLFragmentShaderBuilder::KeyForFragmentPosition(optState.getRenderTarget(),
                                                                  gpu->glCaps());
    } else {
        header->fFragPosKey = 0;
    }

    // Record attribute indices
    header->fPositionAttributeIndex = descInfo.positionAttributeIndex();
    header->fLocalCoordAttributeIndex = descInfo.localCoordAttributeIndex();

    // For constant color and coverage we need an attribute with an index beyond those already set
    int availableAttributeIndex = optState.getVertexAttribCount();
    if (descInfo.hasColorVertexAttribute()) {
        header->fColorAttributeIndex = descInfo.colorVertexAttributeIndex();
    } else if (GrProgramDesc::kAttribute_ColorInput == header->fColorInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fColorAttributeIndex = availableAttributeIndex;
        availableAttributeIndex++;
    } else {
        header->fColorAttributeIndex = -1;
    }

    if (descInfo.hasCoverageVertexAttribute()) {
        header->fCoverageAttributeIndex = descInfo.coverageVertexAttributeIndex();
    } else if (GrProgramDesc::kAttribute_ColorInput == header->fCoverageInput) {
        SkASSERT(availableAttributeIndex < GrDrawState::kMaxVertexAttribCnt);
        header->fCoverageAttributeIndex = availableAttributeIndex;
    } else {
        header->fCoverageAttributeIndex = -1;
    }

    header->fPrimaryOutputType = descInfo.fPrimaryOutputType;
    header->fSecondaryOutputType = descInfo.fSecondaryOutputType;

    header->fColorEffectCnt = optState.numColorStages();
    header->fCoverageEffectCnt = optState.numCoverageStages();
    desc->finalize();
    return true;
}
