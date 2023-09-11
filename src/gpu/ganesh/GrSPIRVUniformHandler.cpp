/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrSPIRVUniformHandler.h"

#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"

GrSPIRVUniformHandler::GrSPIRVUniformHandler(GrGLSLProgramBuilder* program)
    : INHERITED(program)
    , fUniforms(kUniformsPerBlock)
    , fSamplers(kUniformsPerBlock) {}

const GrShaderVar& GrSPIRVUniformHandler::getUniformVariable(UniformHandle u) const {
    return fUniforms.item(u.toIndex()).fVariable;
}

const char* GrSPIRVUniformHandler::getUniformCStr(UniformHandle u) const {
    return fUniforms.item(u.toIndex()).fVariable.getName().c_str();
}

// FIXME: this code was ripped from GrVkUniformHandler; should be refactored.
namespace {

uint32_t sksltype_to_alignment_mask(SkSLType type) {
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
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

static inline uint32_t sksltype_to_size(SkSLType type) {
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
            //TODO: this will be 4 * szof(float) on std430.
            return 8 * sizeof(float);
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
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

uint32_t get_ubo_offset(uint32_t* currentOffset, SkSLType type, int arrayCount) {
    uint32_t alignmentMask = sksltype_to_alignment_mask(type);
    // We want to use the std140 layout here, so we must make arrays align to 16 bytes.
    // TODO(skia:13380): make sure 2x3 and 3x2 matrices are handled properly once SkSLType adds
    // support for non-square matrices
    if (arrayCount || type == SkSLType::kFloat2x2 || type == SkSLType::kHalf2x2) {
        alignmentMask = 0xF;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    uint32_t uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        uint32_t elementSize = std::max<uint32_t>(16, sksltype_to_size(type));
        SkASSERT(0 == (elementSize & 0xF));
        *currentOffset = uniformOffset + elementSize * arrayCount;
    } else {
        *currentOffset = uniformOffset + sksltype_to_size(type);
    }
    return uniformOffset;
}

}  // namespace

GrGLSLUniformHandler::UniformHandle GrSPIRVUniformHandler::internalAddUniformArray(
        const GrProcessor* owner,
        uint32_t visibility,
        SkSLType type,
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

GrGLSLUniformHandler::SamplerHandle GrSPIRVUniformHandler::addSampler(
        const GrBackendFormat& backendFormat,
        GrSamplerState,
        const skgpu::Swizzle& swizzle,
        const char* name,
        const GrShaderCaps* caps) {
    SkASSERT(name && strlen(name));

    int binding = fSamplers.count() * 2;

    SkString mangleName = fProgramBuilder->nameVariable('u', name, /*mangle=*/true);
    SkString layoutQualifier = SkStringPrintf("direct3d, set = %d, sampler = %d, texture = %d",
                                              kSamplerTextureDescriptorSet,
                                              binding,
                                              binding + 1);

    SPIRVUniformInfo& uniformInfo = fSamplers.emplace_back();
    uniformInfo.fVariable =
            GrShaderVar{std::move(mangleName),
                        SkSLCombinedSamplerTypeForTextureType(backendFormat.textureType()),
                        GrShaderVar::TypeModifier::None,
                        GrShaderVar::kNonArray,
                        std::move(layoutQualifier),
                        SkString()};

    uniformInfo.fVisibility = kFragment_GrShaderFlag;
    uniformInfo.fOwner      = nullptr;
    uniformInfo.fRawName    = SkString(name);
    uniformInfo.fUBOOffset  = 0;

    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.size() == fSamplers.count());

    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

const char* GrSPIRVUniformHandler::samplerVariable(
        GrGLSLUniformHandler::SamplerHandle handle) const {
    return fSamplers.item(handle.toIndex()).fVariable.getName().c_str();
}

skgpu::Swizzle GrSPIRVUniformHandler::samplerSwizzle(
        GrGLSLUniformHandler::SamplerHandle handle) const {
    return fSamplerSwizzles[handle.toIndex()];
}

void GrSPIRVUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const SPIRVUniformInfo& sampler : fSamplers.items()) {
        if (sampler.fVisibility & visibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
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
    return get_ubo_offset(&currentOffset, SkSLType::kFloat2, 0);
}
