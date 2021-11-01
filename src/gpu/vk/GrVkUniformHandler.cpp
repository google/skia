/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkUniformHandler.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkTexture.h"

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
// This alignment mask will give correct alignments for using the std430 block layout. If you want
// the std140 alignment, you can use this, but then make sure if you have an array type it is
// aligned to 16 bytes (i.e. has mask of 0xF).
// These are designated in the Vulkan spec, section 14.5.4 "Offset and Stride Assignment".
// https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#interfaces-resources-layout
static uint32_t grsltype_to_alignment_mask(GrSLType type) {
    switch(type) {
        case kShort_GrSLType: // fall through
        case kUShort_GrSLType:
            return 0x1;
        case kShort2_GrSLType: // fall through
        case kUShort2_GrSLType:
            return 0x3;
        case kShort3_GrSLType: // fall through
        case kShort4_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
            return 0x7;
        case kInt_GrSLType:
        case kUInt_GrSLType:
            return 0x3;
        case kInt2_GrSLType:
        case kUInt2_GrSLType:
            return 0x7;
        case kInt3_GrSLType:
        case kUInt3_GrSLType:
        case kInt4_GrSLType:
        case kUInt4_GrSLType:
            return 0xF;
        case kHalf_GrSLType: // fall through
        case kFloat_GrSLType:
            return 0x3;
        case kHalf2_GrSLType: // fall through
        case kFloat2_GrSLType:
            return 0x7;
        case kHalf3_GrSLType: // fall through
        case kFloat3_GrSLType:
            return 0xF;
        case kHalf4_GrSLType: // fall through
        case kFloat4_GrSLType:
            return 0xF;
        case kHalf2x2_GrSLType: // fall through
        case kFloat2x2_GrSLType:
            return 0x7;
        case kHalf3x3_GrSLType: // fall through
        case kFloat3x3_GrSLType:
            return 0xF;
        case kHalf4x4_GrSLType: // fall through
        case kFloat4x4_GrSLType:
            return 0xF;

        // This query is only valid for certain types.
        case kVoid_GrSLType:
        case kBool_GrSLType:
        case kBool2_GrSLType:
        case kBool3_GrSLType:
        case kBool4_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kInput_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}

/** Returns the size in bytes taken up in vulkanbuffers for GrSLTypes. */
static inline uint32_t grsltype_to_vk_size(GrSLType type, int layout) {
    switch(type) {
        case kShort_GrSLType:
            return sizeof(int16_t);
        case kShort2_GrSLType:
            return 2 * sizeof(int16_t);
        case kShort3_GrSLType:
            return 3 * sizeof(int16_t);
        case kShort4_GrSLType:
            return 4 * sizeof(int16_t);
        case kUShort_GrSLType:
            return sizeof(uint16_t);
        case kUShort2_GrSLType:
            return 2 * sizeof(uint16_t);
        case kUShort3_GrSLType:
            return 3 * sizeof(uint16_t);
        case kUShort4_GrSLType:
            return 4 * sizeof(uint16_t);
        case kHalf_GrSLType: // fall through
        case kFloat_GrSLType:
            return sizeof(float);
        case kHalf2_GrSLType: // fall through
        case kFloat2_GrSLType:
            return 2 * sizeof(float);
        case kHalf3_GrSLType: // fall through
        case kFloat3_GrSLType:
            return 3 * sizeof(float);
        case kHalf4_GrSLType: // fall through
        case kFloat4_GrSLType:
            return 4 * sizeof(float);
        case kInt_GrSLType: // fall through
        case kUInt_GrSLType:
            return sizeof(int32_t);
        case kInt2_GrSLType: // fall through
        case kUInt2_GrSLType:
            return 2 * sizeof(int32_t);
        case kInt3_GrSLType: // fall through
        case kUInt3_GrSLType:
            return 3 * sizeof(int32_t);
        case kInt4_GrSLType: // fall through
        case kUInt4_GrSLType:
            return 4 * sizeof(int32_t);
        case kHalf2x2_GrSLType: // fall through
        case kFloat2x2_GrSLType:
            if (layout == GrVkUniformHandler::kStd430Layout) {
                return 4 * sizeof(float);
            } else {
                return 8 * sizeof(float);
            }
        case kHalf3x3_GrSLType: // fall through
        case kFloat3x3_GrSLType:
            return 12 * sizeof(float);
        case kHalf4x4_GrSLType: // fall through
        case kFloat4x4_GrSLType:
            return 16 * sizeof(float);

        // This query is only valid for certain types.
        case kVoid_GrSLType:
        case kBool_GrSLType:
        case kBool2_GrSLType:
        case kBool3_GrSLType:
        case kBool4_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kInput_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}

// Given the current offset into the ubo data, calculate the offset for the uniform we're trying to
// add taking into consideration all alignment requirements. The uniformOffset is set to the offset
// for the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static uint32_t get_aligned_offset(uint32_t* currentOffset,
                                   GrSLType type,
                                   int arrayCount,
                                   int layout) {
    uint32_t alignmentMask = grsltype_to_alignment_mask(type);
    // For std140 layout we must make arrays align to 16 bytes.
    if (layout == GrVkUniformHandler::kStd140Layout && (arrayCount || type == kFloat2x2_GrSLType)) {
        alignmentMask = 0xF;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    int32_t uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        // TODO: this shouldn't be necessary for std430
        uint32_t elementSize = std::max<uint32_t>(16, grsltype_to_vk_size(type, layout));
        SkASSERT(0 == (elementSize & 0xF));
        *currentOffset = uniformOffset + elementSize * arrayCount;
    } else {
        *currentOffset = uniformOffset + grsltype_to_vk_size(type, layout);
    }
    return uniformOffset;
}

GrVkUniformHandler::~GrVkUniformHandler() {
    for (VkUniformInfo& sampler : fSamplers.items()) {
        if (sampler.fImmutableSampler) {
            sampler.fImmutableSampler->unref();
            sampler.fImmutableSampler = nullptr;
        }
    }
}

GrGLSLUniformHandler::UniformHandle GrVkUniformHandler::internalAddUniformArray(
                                                                   const GrFragmentProcessor* owner,
                                                                   uint32_t visibility,
                                                                   GrSLType type,
                                                                   const char* name,
                                                                   bool mangleName,
                                                                   int arrayCount,
                                                                   const char** outName) {
    SkASSERT(name && strlen(name));
    SkASSERT(GrSLTypeCanBeUniformValue(type));

    // TODO this is a bit hacky, lets think of a better way.  Basically we need to be able to use
    // the uniform view matrix name in the GP, and the GP is immutable so it has to tell the PB
    // exactly what name it wants to use for the uniform view matrix.  If we prefix anythings, then
    // the names will mismatch.  I think the correct solution is to have all GPs which need the
    // uniform view matrix, they should upload the view matrix in their setData along with regular
    // uniforms.
    char prefix = 'u';
    if ('u' == name[0] || !strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX))) {
        prefix = '\0';
    }
    SkString resolvedName = fProgramBuilder->nameVariable(prefix, name, mangleName);

    VkUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(resolvedName),
                                     type,
                                     GrShaderVar::TypeModifier::None,
                                     arrayCount};

    tempInfo.fVisibility = visibility;
    tempInfo.fOwner      = owner;
    tempInfo.fRawName    = SkString(name);

    for (int layout = 0; layout < kLayoutCount; ++layout) {
        tempInfo.fOffsets[layout] = get_aligned_offset(&fCurrentOffsets[layout],
                                                       type,
                                                       arrayCount,
                                                       layout);
    }

    fUniforms.push_back(tempInfo);

    if (outName) {
        *outName = fUniforms.back().fVariable.c_str();
    }

    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrVkUniformHandler::addSampler(
        const GrBackendFormat& backendFormat, GrSamplerState state, const GrSwizzle& swizzle,
        const char* name, const GrShaderCaps* shaderCaps) {
    SkASSERT(name && strlen(name));

    const char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    SkString layoutQualifier;
    layoutQualifier.appendf("set=%d, binding=%d", kSamplerDescSet, fSamplers.count());

    VkUniformInfo tempInfo;
    tempInfo.fVariable =
            GrShaderVar{std::move(mangleName),
                        GrSLCombinedSamplerTypeForTextureType(backendFormat.textureType()),
                        GrShaderVar::TypeModifier::Uniform,
                        GrShaderVar::kNonArray,
                        std::move(layoutQualifier),
                        SkString()};

    tempInfo.fVisibility = kFragment_GrShaderFlag;
    tempInfo.fOwner      = nullptr;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fOffsets[0] = 0;
    tempInfo.fOffsets[1] = 0;

    fSamplers.push_back(tempInfo);

    // Check if we are dealing with an external texture and store the needed information if so.
    auto ycbcrInfo = backendFormat.getVkYcbcrConversionInfo();
    if (ycbcrInfo && ycbcrInfo->isValid()) {
        GrVkGpu* gpu = static_cast<GrVkPipelineStateBuilder*>(fProgramBuilder)->gpu();
        GrVkSampler* sampler = gpu->resourceProvider().findOrCreateCompatibleSampler(state,
                                                                                     *ycbcrInfo);
        fSamplers.back().fImmutableSampler = sampler;
        if (!sampler) {
            return {};
        }
    }

    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.count() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrVkUniformHandler::addInputSampler(const GrSwizzle& swizzle,
                                                                        const char* name) {
    SkASSERT(name && strlen(name));
    SkASSERT(fInputUniform.fVariable.getType() == kVoid_GrSLType);

    const char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    SkString layoutQualifier;
    layoutQualifier.appendf("input_attachment_index=%d, set=%d, binding=%d",
                            kDstInputAttachmentIndex, kInputDescSet, kInputBinding);

    fInputUniform = {
            GrShaderVar{std::move(mangleName), kInput_GrSLType, GrShaderVar::TypeModifier::Uniform,
                        GrShaderVar::kNonArray, std::move(layoutQualifier), SkString()},
            kFragment_GrShaderFlag, nullptr, SkString(name)};
    fInputSwizzle = swizzle;
    return GrGLSLUniformHandler::SamplerHandle(0);
}

void GrVkUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const VkUniformInfo& sampler : fSamplers.items()) {
        SkASSERT(sampler.fVariable.getType() == kTexture2DSampler_GrSLType ||
                 sampler.fVariable.getType() == kTextureExternalSampler_GrSLType);
        if (visibility == sampler.fVisibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }
    if (fInputUniform.fVariable.getType() == kInput_GrSLType) {
        if (visibility == fInputUniform.fVisibility) {
            SkASSERT(visibility == kFragment_GrShaderFlag);
            fInputUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }

#ifdef SK_DEBUG
    bool firstOffsetCheck = false;
    for (const VkUniformInfo& localUniform : fUniforms.items()) {
        if (!firstOffsetCheck) {
            // Check to make sure we are starting our offset at 0 so the offset qualifier we
            // set on each variable in the uniform block is valid.
            SkASSERT(0 == localUniform.fOffsets[kStd140Layout] &&
                     0 == localUniform.fOffsets[kStd430Layout]);
            firstOffsetCheck = true;
        }
    }
#endif

    // At this point we determine whether we'll be using push constants based on the
    // uniforms set so far. Later checks will use the internal bool we set here to
    // keep things consistent.
    this->determineIfUsePushConstants();
    SkString uniformsString;
    for (const VkUniformInfo& localUniform : fUniforms.items()) {
        if (visibility & localUniform.fVisibility) {
            if (GrSLTypeCanBeUniformValue(localUniform.fVariable.getType())) {
                Layout layout = fUsePushConstants ? kStd430Layout : kStd140Layout;
                uniformsString.appendf("layout(offset=%d) ", localUniform.fOffsets[layout]);
                localUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
                uniformsString.append(";\n");
            }
        }
    }

    if (!uniformsString.isEmpty()) {
        if (fUsePushConstants) {
            out->append("layout (push_constant) ");
        } else {
            out->appendf("layout (set=%d, binding=%d) ",
                         kUniformBufferDescSet, kUniformBinding);
        }
        out->append("uniform uniformBuffer\n{\n");
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

uint32_t GrVkUniformHandler::getRTFlipOffset() const {
    Layout layout = fUsePushConstants ? kStd430Layout : kStd140Layout;
    uint32_t currentOffset = fCurrentOffsets[layout];
    return get_aligned_offset(&currentOffset, kFloat2_GrSLType, 0, layout);
}

void GrVkUniformHandler::determineIfUsePushConstants() const {
    // We may insert a uniform for flipping origin-sensitive language features (e.g. sk_FragCoord).
    // We won't know that for sure until then but we need to make this determination now,
    // so assume we will need it.
    static constexpr uint32_t kPad = 2*sizeof(float);
    fUsePushConstants =
            fCurrentOffsets[kStd430Layout] > 0 &&
            fCurrentOffsets[kStd430Layout] + kPad <= fProgramBuilder->caps()->maxPushConstantsSize();
}
