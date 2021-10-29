/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/mtl/GrMtlUniformHandler.h"

#include "include/private/GrMtlTypesPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
static uint32_t grsltype_to_alignment_mask(GrSLType type) {
    switch(type) {
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kFloat_GrSLType:
            return 0x3;
        case kInt2_GrSLType:
        case kUint2_GrSLType:
        case kFloat2_GrSLType:
            return 0x7;
        case kInt3_GrSLType:
        case kUint3_GrSLType:
        case kFloat3_GrSLType:
        case kInt4_GrSLType:
        case kUint4_GrSLType:
        case kFloat4_GrSLType:
            return 0xF;

        case kFloat2x2_GrSLType:
            return 0x7;
        case kFloat3x3_GrSLType:
            return 0xF;
        case kFloat4x4_GrSLType:
            return 0xF;

/*
        // TODO(skia:12339): Enable these once MetalCodeGenerator supports half-precision types.
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kHalf_GrSLType:
            return 0x1;
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kHalf2_GrSLType:
            return 0x3;
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
            return 0x7;

        case kHalf2x2_GrSLType:
            return 0x3;
        case kHalf3x3_GrSLType:
            return 0x7;
        case kHalf4x4_GrSLType:
            return 0x7;
*/
        // TODO(skia:12339): Remove these once MetalCodeGenerator supports half-precision types.
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kHalf_GrSLType:
            return 0x3;
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kHalf2_GrSLType:
            return 0x7;
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
            return 0xF;

        case kHalf2x2_GrSLType:
            return 0x7;
        case kHalf3x3_GrSLType:
            return 0xF;
        case kHalf4x4_GrSLType:
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

/** Returns the size in bytes taken up in Metal buffers for GrSLTypes. */
static inline uint32_t grsltype_to_mtl_size(GrSLType type) {
    switch(type) {
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kFloat_GrSLType:
            return 4;
        case kInt2_GrSLType:
        case kUint2_GrSLType:
        case kFloat2_GrSLType:
            return 8;
        case kInt3_GrSLType:
        case kUint3_GrSLType:
        case kFloat3_GrSLType:
        case kInt4_GrSLType:
        case kUint4_GrSLType:
        case kFloat4_GrSLType:
            return 16;

        case kFloat2x2_GrSLType:
            return 16;
        case kFloat3x3_GrSLType:
            return 48;
        case kFloat4x4_GrSLType:
            return 64;

/*
        // TODO(skia:12339): Enable these once MetalCodeGenerator supports half-precision types.
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kHalf_GrSLType:
            return 2;
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kHalf2_GrSLType:
            return 4;
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
            return 8;

        case kHalf2x2_GrSLType:
            return 8;
        case kHalf3x3_GrSLType:
            return 24;
        case kHalf4x4_GrSLType:
            return 32;
*/
        // TODO(skia:12339): Remove these once MetalCodeGenerator supports half-precision types.
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kHalf_GrSLType:
            return 4;
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kHalf2_GrSLType:
            return 8;
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
            return 16;

        case kHalf2x2_GrSLType:
            return 16;
        case kHalf3x3_GrSLType:
            return 48;
        case kHalf4x4_GrSLType:
            return 64;

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

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static uint32_t get_ubo_aligned_offset(uint32_t* currentOffset,
                                       uint32_t* maxAlignment,
                                       GrSLType type,
                                       int arrayCount) {
    uint32_t alignmentMask = grsltype_to_alignment_mask(type);
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
        *currentOffset = uniformOffset + grsltype_to_mtl_size(type) * arrayCount;
    } else {
        *currentOffset = uniformOffset + grsltype_to_mtl_size(type);
    }
    return uniformOffset;
}

GrGLSLUniformHandler::UniformHandle GrMtlUniformHandler::internalAddUniformArray(
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

    uint32_t offset = get_ubo_aligned_offset(&fCurrentUBOOffset, &fCurrentUBOMaxAlignment,
                                             type, arrayCount);
    SkString layoutQualifier;
    layoutQualifier.appendf("offset=%d", offset);

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
        const GrBackendFormat& backendFormat, GrSamplerState, const GrSwizzle& swizzle,
        const char* name, const GrShaderCaps* caps) {
    int binding = fSamplers.count();

    SkASSERT(name && strlen(name));

    constexpr char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, /*mangle=*/true);

    GrTextureType type = backendFormat.textureType();

    SkString layoutQualifier;
    layoutQualifier.appendf("binding=%d", binding);

    MtlUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(mangleName),
                                     GrSLCombinedSamplerTypeForTextureType(type),
                                     GrShaderVar::TypeModifier::Uniform,
                                     GrShaderVar::kNonArray,
                                     std::move(layoutQualifier),
                                     SkString()};

    tempInfo.fVisibility = kFragment_GrShaderFlag;
    tempInfo.fOwner      = nullptr;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fUBOffset   = 0;

    fSamplers.push_back(tempInfo);

    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.count() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrMtlUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const UniformInfo& sampler : fSamplers.items()) {
        SkASSERT(sampler.fVariable.getType() == kTexture2DSampler_GrSLType);
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
            if (GrSLTypeCanBeUniformValue(localUniform.fVariable.getType())) {
                localUniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), &uniformsString);
                uniformsString.append(";\n");
            }
        }
    }

    if (!uniformsString.isEmpty()) {
        out->appendf("layout (binding=%d) uniform uniformBuffer\n{\n", kUniformBinding);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}

GR_NORETAIN_END
