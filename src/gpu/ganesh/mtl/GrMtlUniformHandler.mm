/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/mtl/GrMtlUniformHandler.h"

#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/mtl/GrMtlTypesPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
static uint32_t sksltype_to_alignment_mask(SkSLType type) {
    switch(type) {
        case SkSLType::kInt:
        case SkSLType::kUInt:
        case SkSLType::kFloat:
            return 0x3;
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
        case SkSLType::kFloat2:
            return 0x7;
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kFloat3:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
        case SkSLType::kFloat4:
            return 0xF;

        case SkSLType::kFloat2x2:
            return 0x7;
        case SkSLType::kFloat3x3:
            return 0xF;
        case SkSLType::kFloat4x4:
            return 0xF;

        case SkSLType::kShort:
        case SkSLType::kUShort:
        case SkSLType::kHalf:
            return 0x1;
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
        case SkSLType::kHalf2:
            return 0x3;
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kHalf3:
        case SkSLType::kHalf4:
            return 0x7;

        case SkSLType::kHalf2x2:
            return 0x3;
        case SkSLType::kHalf3x3:
            return 0x7;
        case SkSLType::kHalf4x4:
            return 0x7;

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

/** Returns the size in bytes taken up in Metal buffers for SkSLTypes. */
static inline uint32_t sksltype_to_mtl_size(SkSLType type) {
    switch(type) {
        case SkSLType::kInt:
        case SkSLType::kUInt:
        case SkSLType::kFloat:
            return 4;
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
        case SkSLType::kFloat2:
            return 8;
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kFloat3:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
        case SkSLType::kFloat4:
            return 16;

        case SkSLType::kFloat2x2:
            return 16;
        case SkSLType::kFloat3x3:
            return 48;
        case SkSLType::kFloat4x4:
            return 64;

        case SkSLType::kShort:
        case SkSLType::kUShort:
        case SkSLType::kHalf:
            return 2;
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
        case SkSLType::kHalf2:
            return 4;
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kHalf3:
        case SkSLType::kHalf4:
            return 8;

        case SkSLType::kHalf2x2:
            return 8;
        case SkSLType::kHalf3x3:
            return 24;
        case SkSLType::kHalf4x4:
            return 32;

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

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static uint32_t get_ubo_aligned_offset(uint32_t* currentOffset,
                                       uint32_t* maxAlignment,
                                       SkSLType type,
                                       int arrayCount) {
    uint32_t alignmentMask = sksltype_to_alignment_mask(type);
    if (alignmentMask > *maxAlignment) {
        *maxAlignment = alignmentMask;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    uint32_t uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        *currentOffset = uniformOffset + sksltype_to_mtl_size(type) * arrayCount;
    } else {
        *currentOffset = uniformOffset + sksltype_to_mtl_size(type);
    }
    return uniformOffset;
}

GrGLSLUniformHandler::UniformHandle GrMtlUniformHandler::internalAddUniformArray(
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

    uint32_t offset = get_ubo_aligned_offset(&fCurrentUBOOffset, &fCurrentUBOMaxAlignment,
                                             type, arrayCount);
    SkString layoutQualifier;
    layoutQualifier.appendf("offset=%u", offset);

    // When outputing the GLSL, only the outer uniform block will get the Uniform modifier. Thus
    // we set the modifier to none for all uniforms declared inside the block.
    MtlUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(resolvedName),
                                     type,
                                     GrShaderVar::TypeModifier::None,
                                     arrayCount,
                                     std::move(layoutQualifier),
                                     SkString()};

    tempInfo.fVisibility = kFragment_GrShaderFlag | kVertex_GrShaderFlag;
    tempInfo.fOwner      = owner;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fUBOffset   = offset;

    fUniforms.push_back(tempInfo);

    if (outName) {
        *outName = fUniforms.back().fVariable.c_str();
    }

    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrMtlUniformHandler::addSampler(
        const GrBackendFormat& backendFormat, GrSamplerState, const skgpu::Swizzle& swizzle,
        const char* name, const GrShaderCaps* caps) {
    int binding = fSamplers.count();

    SkASSERT(name && strlen(name));

    constexpr char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    GrTextureType type = backendFormat.textureType();

    SkString layoutQualifier;
    layoutQualifier.appendf("metal, binding=%d", binding);

    MtlUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(mangleName),
                                     SkSLCombinedSamplerTypeForTextureType(type),
                                     GrShaderVar::TypeModifier::None,
                                     GrShaderVar::kNonArray,
                                     std::move(layoutQualifier),
                                     SkString()};

    tempInfo.fVisibility = kFragment_GrShaderFlag;
    tempInfo.fOwner      = nullptr;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fUBOffset   = 0;

    fSamplers.push_back(tempInfo);

    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.size() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrMtlUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const UniformInfo& sampler : fSamplers.items()) {
        SkASSERT(sampler.fVariable.getType() == SkSLType::kTexture2DSampler);
        if (visibility == sampler.fVisibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }

#ifdef SK_DEBUG
    bool firstOffsetCheck = false;
    for (const MtlUniformInfo& localUniform : fUniforms.items()) {
        if (!firstOffsetCheck) {
            // Check to make sure we are starting our offset at 0 so the offset qualifier we
            // set on each variable in the uniform block is valid.
            SkASSERT(0 == localUniform.fUBOffset);
            firstOffsetCheck = true;
        }
    }
#endif

    SkString uniformsString;
    for (const UniformInfo& localUniform : fUniforms.items()) {
        if (visibility & localUniform.fVisibility) {
            if (SkSLTypeCanBeUniformValue(localUniform.fVariable.getType())) {
                localUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
                uniformsString.append(";\n");
            }
        }
    }

    if (!uniformsString.isEmpty()) {
        out->appendf("layout (metal, binding=%zu) uniform uniformBuffer\n{\n", kUniformBinding);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

GR_NORETAIN_END
