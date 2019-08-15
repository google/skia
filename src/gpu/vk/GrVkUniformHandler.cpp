/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkUniformHandler.h"

#include "src/gpu/GrTexturePriv.h"
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
        case kByte_GrSLType: // fall through
        case kUByte_GrSLType:
            return 0x0;
        case kByte2_GrSLType: // fall through
        case kUByte2_GrSLType:
            return 0x1;
        case kByte3_GrSLType: // fall through
        case kByte4_GrSLType:
        case kUByte3_GrSLType:
        case kUByte4_GrSLType:
            return 0x3;
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
        case kUint_GrSLType:
            return 0x3;
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
        case kUint2_GrSLType:
            return 0x7;
        case kInt2_GrSLType:
            return 0x7;
        case kInt3_GrSLType:
            return 0xF;
        case kInt4_GrSLType:
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
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kSampler_GrSLType:
        case kTexture2D_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}

/** Returns the size in bytes taken up in vulkanbuffers for GrSLTypes. */
static inline uint32_t grsltype_to_vk_size(GrSLType type) {
    switch(type) {
        case kByte_GrSLType:
            return sizeof(int8_t);
        case kByte2_GrSLType:
            return 2 * sizeof(int8_t);
        case kByte3_GrSLType:
            return 3 * sizeof(int8_t);
        case kByte4_GrSLType:
            return 4 * sizeof(int8_t);
        case kUByte_GrSLType:
            return sizeof(uint8_t);
        case kUByte2_GrSLType:
            return 2 * sizeof(uint8_t);
        case kUByte3_GrSLType:
            return 3 * sizeof(uint8_t);
        case kUByte4_GrSLType:
            return 4 * sizeof(uint8_t);
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
        case kInt_GrSLType:
            return sizeof(int32_t);
        case kUint_GrSLType:
            return sizeof(int32_t);
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
        case kUint2_GrSLType:
            return 2 * sizeof(uint32_t);
        case kInt2_GrSLType:
            return 2 * sizeof(int32_t);
        case kInt3_GrSLType:
            return 3 * sizeof(int32_t);
        case kInt4_GrSLType:
            return 4 * sizeof(int32_t);
        case kHalf2x2_GrSLType: // fall through
        case kFloat2x2_GrSLType:
            //TODO: this will be 4 * szof(float) on std430.
            return 8 * sizeof(float);
        case kHalf3x3_GrSLType: // fall through
        case kFloat3x3_GrSLType:
            return 12 * sizeof(float);
        case kHalf4x4_GrSLType: // fall through
        case kFloat4x4_GrSLType:
            return 16 * sizeof(float);

        // This query is only valid for certain types.
        case kVoid_GrSLType:
        case kBool_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kSampler_GrSLType:
        case kTexture2D_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}


// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static void get_ubo_aligned_offset(uint32_t* uniformOffset,
                                   uint32_t* currentOffset,
                                   GrSLType type,
                                   int arrayCount) {
    uint32_t alignmentMask = grsltype_to_alignment_mask(type);
    // We want to use the std140 layout here, so we must make arrays align to 16 bytes.
    if (arrayCount || type == kFloat2x2_GrSLType) {
        alignmentMask = 0xF;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    *uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        uint32_t elementSize = SkTMax<uint32_t>(16, grsltype_to_vk_size(type));
        SkASSERT(0 == (elementSize & 0xF));
        *currentOffset = *uniformOffset + elementSize * arrayCount;
    } else {
        *currentOffset = *uniformOffset + grsltype_to_vk_size(type);
    }
}

GrVkUniformHandler::~GrVkUniformHandler() {
    GrVkGpu* gpu = static_cast<GrVkPipelineStateBuilder*>(fProgramBuilder)->gpu();
    for (decltype(fSamplers)::Iter iter(&fSamplers); iter.next();) {
        if (iter->fImmutableSampler) {
            iter->fImmutableSampler->unref(gpu);
            iter->fImmutableSampler = nullptr;
        }
    }
}

GrGLSLUniformHandler::UniformHandle GrVkUniformHandler::internalAddUniformArray(
                                                                            uint32_t visibility,
                                                                            GrSLType type,
                                                                            const char* name,
                                                                            bool mangleName,
                                                                            int arrayCount,
                                                                            const char** outName) {
    SkASSERT(name && strlen(name));
    SkASSERT(GrSLTypeIsFloatType(type));

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
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
    fProgramBuilder->nameVariable(uni.fVariable.accessName(), prefix, name, mangleName);
    uni.fVariable.setArrayCount(arrayCount);
    uni.fVisibility = visibility;
    // When outputing the GLSL, only the outer uniform block will get the Uniform modifier. Thus
    // we set the modifier to none for all uniforms declared inside the block.
    uni.fVariable.setTypeModifier(GrShaderVar::kNone_TypeModifier);

    get_ubo_aligned_offset(&uni.fUBOffset, &fCurrentUBOOffset, type, arrayCount);

    SkString layoutQualifier;
    layoutQualifier.appendf("offset=%d", uni.fUBOffset);
    uni.fVariable.addLayoutQualifier(layoutQualifier.c_str());

    if (outName) {
        *outName = uni.fVariable.c_str();
    }

    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrVkUniformHandler::addSampler(const GrTexture* texture,
                                                                   const GrSamplerState& state,
                                                                   const GrSwizzle& swizzle,
                                                                   const char* name,
                                                                   const GrShaderCaps* shaderCaps) {
    SkASSERT(name && strlen(name));
    SkString mangleName;
    char prefix = 'u';
    fProgramBuilder->nameVariable(&mangleName, prefix, name, true);

    GrTextureType type = texture->texturePriv().textureType();

    UniformInfo& info = fSamplers.push_back();
    info.fVariable.setType(GrSLCombinedSamplerTypeForTextureType(type));
    info.fVariable.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
    info.fVariable.setName(mangleName);
    SkString layoutQualifier;
    layoutQualifier.appendf("set=%d, binding=%d", kSamplerDescSet, fSamplers.count() - 1);
    info.fVariable.addLayoutQualifier(layoutQualifier.c_str());
    info.fVisibility = kFragment_GrShaderFlag;
    info.fUBOffset = 0;

    // Check if we are dealing with an external texture and store the needed information if so.
    const GrVkTexture* vkTexture = static_cast<const GrVkTexture*>(texture);
    if (vkTexture->ycbcrConversionInfo().isValid()) {
        GrVkGpu* gpu = static_cast<GrVkPipelineStateBuilder*>(fProgramBuilder)->gpu();
        info.fImmutableSampler = gpu->resourceProvider().findOrCreateCompatibleSampler(
                state, vkTexture->ycbcrConversionInfo());
        SkASSERT(info.fImmutableSampler);
    }

    SkASSERT(shaderCaps->textureSwizzleAppliedInShader());
    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.count() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrVkUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (int i = 0; i < fSamplers.count(); ++i) {
        const UniformInfo& sampler = fSamplers[i];
        SkASSERT(sampler.fVariable.getType() == kTexture2DSampler_GrSLType);
        if (visibility == sampler.fVisibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }

#ifdef SK_DEBUG
    bool firstOffsetCheck = false;
    for (int i = 0; i < fUniforms.count(); ++i) {
        const UniformInfo& localUniform = fUniforms[i];
        if (!firstOffsetCheck) {
            // Check to make sure we are starting our offset at 0 so the offset qualifier we
            // set on each variable in the uniform block is valid.
            SkASSERT(0 == localUniform.fUBOffset);
            firstOffsetCheck = true;
        }
    }
#endif

    SkString uniformsString;
    for (int i = 0; i < fUniforms.count(); ++i) {
        const UniformInfo& localUniform = fUniforms[i];
        if (visibility & localUniform.fVisibility) {
            if (GrSLTypeIsFloatType(localUniform.fVariable.getType())) {
                localUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
                uniformsString.append(";\n");
            }
        }
    }

    if (!uniformsString.isEmpty()) {
        out->appendf("layout (set=%d, binding=%d) uniform uniformBuffer\n{\n",
                     kUniformBufferDescSet, kUniformBinding);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

uint32_t GrVkUniformHandler::getRTHeightOffset() const {
    uint32_t result;
    uint32_t currentOffset = fCurrentUBOOffset;
    get_ubo_aligned_offset(&result, &currentOffset, kFloat_GrSLType, 0);
    return result;
}
