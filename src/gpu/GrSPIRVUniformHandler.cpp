/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSPIRVUniformHandler.h"

#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

GrSPIRVUniformHandler::GrSPIRVUniformHandler(GrGLSLProgramBuilder* program)
    : INHERITED(program)
    , fUniforms(kUniformsPerBlock)
    , fSamplers(kUniformsPerBlock)
    , fTextures(kUniformsPerBlock)
{
}

const GrShaderVar& GrSPIRVUniformHandler::getUniformVariable(UniformHandle u) const {
    return fUniforms.item(u.toIndex()).fVariable;
}

const char* GrSPIRVUniformHandler::getUniformCStr(UniformHandle u) const {
    return fUniforms.item(u.toIndex()).fVariable.getName().c_str();
}

// FIXME: this code was ripped from GrVkUniformHandler; should be refactored.
namespace {

uint32_t grsltype_to_alignment_mask(GrSLType type) {
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
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kInput_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}

static inline uint32_t grsltype_to_size(GrSLType type) {
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
        case kBool2_GrSLType:
        case kBool3_GrSLType:
        case kBool4_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kInput_GrSLType:
            break;
    }
    SK_ABORT("Unexpected type");
}

uint32_t get_ubo_offset(uint32_t* currentOffset, GrSLType type, int arrayCount) {
    uint32_t alignmentMask = grsltype_to_alignment_mask(type);
    // We want to use the std140 layout here, so we must make arrays align to 16 bytes.
    if (arrayCount || type == kFloat2x2_GrSLType) {
        alignmentMask = 0xF;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    uint32_t uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        uint32_t elementSize = std::max<uint32_t>(16, grsltype_to_size(type));
        SkASSERT(0 == (elementSize & 0xF));
        *currentOffset = uniformOffset + elementSize * arrayCount;
    } else {
        *currentOffset = uniformOffset + grsltype_to_size(type);
    }
    return uniformOffset;
}

}  // namespace

GrGLSLUniformHandler::UniformHandle GrSPIRVUniformHandler::internalAddUniformArray(
        const GrFragmentProcessor* owner,
        uint32_t visibility,
        GrSLType type,
        const char* name,
        bool mangleName,
        int arrayCount,
        const char** outName) {
    char prefix = 'u';
    if ('u' == name[0] || !strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX))) {
        prefix = '\0';
    }
    SkString resolvedName = fProgramBuilder->nameVariable(prefix, name, mangleName);

    int offset = get_ubo_offset(&fCurrentUBOOffset, type, arrayCount);
    SkString layoutQualifier;
    layoutQualifier.appendf("offset = %d", offset);

    SPIRVUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(resolvedName),
                                     type,
                                     GrShaderVar::TypeModifier::None,
                                     arrayCount,
                                     std::move(layoutQualifier),
                                     SkString()};

    tempInfo.fVisibility = visibility;
    tempInfo.fOwner      = owner;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fUBOOffset  = offset;

    fUniforms.push_back(tempInfo);

    if (outName) {
        *outName = fUniforms.back().fVariable.c_str();
    }
    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrSPIRVUniformHandler::addSampler(const GrBackendFormat&,
                                                                     GrSamplerState,
                                                                     const GrSwizzle& swizzle,
                                                                     const char* name,
                                                                     const GrShaderCaps* caps) {
    int binding = fSamplers.count() * 2;

    SkString mangleName = fProgramBuilder->nameVariable('s', name, /*mangle=*/true);
    SkString layoutQualifier;
    layoutQualifier.appendf("set = %d, binding = %d", kSamplerTextureDescriptorSet, binding);

    SPIRVUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(mangleName),
                                     kSampler_GrSLType,
                                     GrShaderVar::TypeModifier::Uniform,
                                     GrShaderVar::kNonArray,
                                     std::move(layoutQualifier),
                                     SkString()};

    tempInfo.fVisibility = kFragment_GrShaderFlag;
    tempInfo.fOwner      = nullptr;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fUBOOffset  = 0;

    fSamplers.push_back(tempInfo);
    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.count() == fSamplers.count());

    SkString mangleTexName = fProgramBuilder->nameVariable('t', name, /*mangle=*/true);
    SkString texLayoutQualifier;
    texLayoutQualifier.appendf("set = %d, binding = %d", kSamplerTextureDescriptorSet, binding + 1);
    tempInfo.fVariable = GrShaderVar{std::move(mangleTexName),
                                     kTexture2D_GrSLType,
                                     GrShaderVar::TypeModifier::Uniform,
                                     GrShaderVar::kNonArray,
                                     std::move(texLayoutQualifier),
                                     SkString()};
    fTextures.push_back(tempInfo);

    SkString reference;
    reference.printf("makeSampler2D(%s, %s)",
                     fTextures.back().fVariable.getName().c_str(),
                     fSamplers.back().fVariable.getName().c_str());
    fSamplerReferences.emplace_back(std::move(reference));
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

const char* GrSPIRVUniformHandler::samplerVariable(
        GrGLSLUniformHandler::SamplerHandle handle) const {
    return fSamplerReferences[handle.toIndex()].c_str();
}

GrSwizzle GrSPIRVUniformHandler::samplerSwizzle(GrGLSLUniformHandler::SamplerHandle handle) const {
    return fSamplerSwizzles[handle.toIndex()];
}

void GrSPIRVUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    auto textures = fTextures.items().begin();
    for (const SPIRVUniformInfo& sampler : fSamplers.items()) {
        if (sampler.fVisibility & visibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
            (*textures).fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
        ++textures;
    }
    SkString uniformsString;
    for (const UniformInfo& uniform : fUniforms.items()) {
        if (uniform.fVisibility & visibility) {
            uniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
            uniformsString.append(";\n");
        }
    }
    if (!uniformsString.isEmpty()) {
        out->appendf("layout (set = %d, binding = %d) uniform UniformBuffer\n{\n",
                     kUniformDescriptorSet, kUniformBinding);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

uint32_t GrSPIRVUniformHandler::getRTFlipOffset() const {
    uint32_t currentOffset = fCurrentUBOOffset;
    return get_ubo_offset(&currentOffset, kFloat2_GrSLType, 0);
}
