/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"

#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
// This alignment mask will give correct alignments for using the std430 block layout. If you want
// the std140 alignment, you can use this, but then make sure if you have an array type it is
// aligned to 16 bytes (i.e. has mask of 0xF).
// These are designated in the Vulkan spec, section 14.5.4 "Offset and Stride Assignment".
// https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#interfaces-resources-layout
static uint32_t sksltype_to_alignment_mask(SkSLType type) {
    switch(type) {
        case SkSLType::kShort: // fall through
        case SkSLType::kUShort:
            return 0x1;
        case SkSLType::kShort2: // fall through
        case SkSLType::kUShort2:
            return 0x3;
        case SkSLType::kShort3: // fall through
        case SkSLType::kShort4:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
            return 0x7;
        case SkSLType::kInt:
        case SkSLType::kUInt:
            return 0x3;
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
            return 0x7;
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
            return 0xF;
        case SkSLType::kHalf: // fall through
        case SkSLType::kFloat:
            return 0x3;
        case SkSLType::kHalf2: // fall through
        case SkSLType::kFloat2:
            return 0x7;
        case SkSLType::kHalf3: // fall through
        case SkSLType::kFloat3:
            return 0xF;
        case SkSLType::kHalf4: // fall through
        case SkSLType::kFloat4:
            return 0xF;
        case SkSLType::kHalf2x2: // fall through
        case SkSLType::kFloat2x2:
            return 0x7;
        case SkSLType::kHalf3x3: // fall through
        case SkSLType::kFloat3x3:
            return 0xF;
        case SkSLType::kHalf4x4: // fall through
        case SkSLType::kFloat4x4:
            return 0xF;

        // This query is only valid for certain types.
        case SkSLType::kVoid:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

/** Returns the size in bytes taken up in vulkanbuffers for SkSLTypes. */
static inline uint32_t sksltype_to_vk_size(SkSLType type, int layout) {
    switch(type) {
        case SkSLType::kShort:
            return sizeof(int16_t);
        case SkSLType::kShort2:
            return 2 * sizeof(int16_t);
        case SkSLType::kShort3:
            return 3 * sizeof(int16_t);
        case SkSLType::kShort4:
            return 4 * sizeof(int16_t);
        case SkSLType::kUShort:
            return sizeof(uint16_t);
        case SkSLType::kUShort2:
            return 2 * sizeof(uint16_t);
        case SkSLType::kUShort3:
            return 3 * sizeof(uint16_t);
        case SkSLType::kUShort4:
            return 4 * sizeof(uint16_t);
        case SkSLType::kHalf: // fall through
        case SkSLType::kFloat:
            return sizeof(float);
        case SkSLType::kHalf2: // fall through
        case SkSLType::kFloat2:
            return 2 * sizeof(float);
        case SkSLType::kHalf3: // fall through
        case SkSLType::kFloat3:
            return 3 * sizeof(float);
        case SkSLType::kHalf4: // fall through
        case SkSLType::kFloat4:
            return 4 * sizeof(float);
        case SkSLType::kInt: // fall through
        case SkSLType::kUInt:
            return sizeof(int32_t);
        case SkSLType::kInt2: // fall through
        case SkSLType::kUInt2:
            return 2 * sizeof(int32_t);
        case SkSLType::kInt3: // fall through
        case SkSLType::kUInt3:
            return 3 * sizeof(int32_t);
        case SkSLType::kInt4: // fall through
        case SkSLType::kUInt4:
            return 4 * sizeof(int32_t);
        case SkSLType::kHalf2x2: // fall through
        case SkSLType::kFloat2x2:
            if (layout == GrVkUniformHandler::kStd430Layout) {
                return 4 * sizeof(float);
            } else {
                return 8 * sizeof(float);
            }
        case SkSLType::kHalf3x3: // fall through
        case SkSLType::kFloat3x3:
            return 12 * sizeof(float);
        case SkSLType::kHalf4x4: // fall through
        case SkSLType::kFloat4x4:
            return 16 * sizeof(float);

        // This query is only valid for certain types.
        case SkSLType::kVoid:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

// Given the current offset into the ubo data, calculate the offset for the uniform we're trying to
// add taking into consideration all alignment requirements. The uniformOffset is set to the offset
// for the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static uint32_t get_aligned_offset(uint32_t* currentOffset,
                                   SkSLType type,
                                   int arrayCount,
                                   int layout) {
    uint32_t alignmentMask = sksltype_to_alignment_mask(type);
    // For std140 layout we must make arrays align to 16 bytes.
    // TODO(skia:13380): make sure 2x3 and 3x2 matrices are handled properly once SkSLType adds
    // support for non-square matrices
    if (layout == GrVkUniformHandler::kStd140Layout &&
        (arrayCount || type == SkSLType::kFloat2x2 || type == SkSLType::kHalf2x2)) {
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
        uint32_t elementSize = std::max<uint32_t>(16, sksltype_to_vk_size(type, layout));
        SkASSERT(0 == (elementSize & 0xF));
        *currentOffset = uniformOffset + elementSize * arrayCount;
    } else {
        *currentOffset = uniformOffset + sksltype_to_vk_size(type, layout);
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
                                                                   const GrProcessor* owner,
                                                                   uint32_t visibility,
                                                                   SkSLType type,
                                                                   const char* name,
                                                                   bool mangleName,
                                                                   int arrayCount,
                                                                   const char** outName) {
    SkASSERT(name && strlen(name));
    SkASSERT(SkSLTypeCanBeUniformValue(type));

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
        const GrBackendFormat& backendFormat,
        GrSamplerState state,
        const skgpu::Swizzle& swizzle,
        const char* name,
        const GrShaderCaps* shaderCaps) {
    SkASSERT(name && strlen(name));

    const char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    SkString layoutQualifier;
    layoutQualifier.appendf("vulkan, set=%d, binding=%d", kSamplerDescSet, fSamplers.count());

    VkUniformInfo tempInfo;
    tempInfo.fVariable =
            GrShaderVar{std::move(mangleName),
                        SkSLCombinedSamplerTypeForTextureType(backendFormat.textureType()),
                        GrShaderVar::TypeModifier::None,
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
    auto ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(backendFormat);
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
    SkASSERT(fSamplerSwizzles.size() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrVkUniformHandler::addInputSampler(
            const skgpu::Swizzle& swizzle, const char* name) {
    SkASSERT(name && strlen(name));
    SkASSERT(fInputUniform.fVariable.getType() == SkSLType::kVoid);

    const char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    auto layoutQualifier = SkStringPrintf("vulkan, input_attachment_index=%d, set=%d, binding=%d",
                                          kDstInputAttachmentIndex,
                                          kInputDescSet,
                                          kInputBinding);

    fInputUniform = {GrShaderVar{std::move(mangleName),
                                 SkSLType::kInput,
                                 GrShaderVar::TypeModifier::None,
                                 GrShaderVar::kNonArray,
                                 std::move(layoutQualifier),
                                 SkString()},
                     kFragment_GrShaderFlag,
                     nullptr,
                     SkString(name)};
    fInputSwizzle = swizzle;
    return GrGLSLUniformHandler::SamplerHandle(0);
}

void GrVkUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const VkUniformInfo& sampler : fSamplers.items()) {
        SkASSERT(sampler.fVariable.getType() == SkSLType::kTexture2DSampler ||
                 sampler.fVariable.getType() == SkSLType::kTextureExternalSampler);
        if (visibility == sampler.fVisibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }
    if (fInputUniform.fVariable.getType() == SkSLType::kInput) {
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
            if (SkSLTypeCanBeUniformValue(localUniform.fVariable.getType())) {
                Layout layout = fUsePushConstants ? kStd430Layout : kStd140Layout;
                uniformsString.appendf("layout(offset=%d) ", localUniform.fOffsets[layout]);
                localUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
                uniformsString.append(";\n");
            }
        }
    }

    if (!uniformsString.isEmpty()) {
        if (fUsePushConstants) {
            out->append("layout (vulkan, push_constant) ");
        } else {
            out->appendf("layout (vulkan, set=%d, binding=%d) ",
                         kUniformBufferDescSet, kUniformBinding);
        }
        out->append("uniform uniformBuffer\n{\n");
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

uint32_t GrVkUniformHandler::getRTFlipOffset() const {
    Layout layout = fUsePushConstants ? kStd430Layout : kStd140Layout;
    uint32_t currentOffset = fCurrentOffsets[layout];
    return get_aligned_offset(&currentOffset, SkSLType::kFloat2, 0, layout);
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
