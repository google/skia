/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

// TODO: this class is basically copy and pasted from GrVklUniformHandler so that we can have
// some shaders working. The SkSL Metal code generator was written to work with GLSL generated for
// the Ganesh Vulkan backend, so it should all work. There might be better ways to do things in
// Metal and/or some Vulkan GLSLisms left in.

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
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
            break;
    }
    SK_ABORT("Unexpected type");
    return 0;
}

/** Returns the size in bytes taken up in Metal buffers for GrSLTypes. */
static inline uint32_t grsltype_to_mtl_size(GrSLType type) {
    switch(type) {
        case kByte_GrSLType:
            return sizeof(int8_t);
        case kByte2_GrSLType:
            return 2 * sizeof(int8_t);
        case kByte3_GrSLType:
            return 4 * sizeof(int8_t);
        case kByte4_GrSLType:
            return 4 * sizeof(int8_t);
        case kUByte_GrSLType:
            return sizeof(uint8_t);
        case kUByte2_GrSLType:
            return 2 * sizeof(uint8_t);
        case kUByte3_GrSLType:
            return 4 * sizeof(uint8_t);
        case kUByte4_GrSLType:
            return 4 * sizeof(uint8_t);
        case kShort_GrSLType:
            return sizeof(int16_t);
        case kShort2_GrSLType:
            return 2 * sizeof(int16_t);
        case kShort3_GrSLType:
            return 4 * sizeof(int16_t);
        case kShort4_GrSLType:
            return 4 * sizeof(int16_t);
        case kUShort_GrSLType:
            return sizeof(uint16_t);
        case kUShort2_GrSLType:
            return 2 * sizeof(uint16_t);
        case kUShort3_GrSLType:
            return 4 * sizeof(uint16_t);
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
            return 4 * sizeof(float);
        case kHalf4_GrSLType: // fall through
        case kFloat4_GrSLType:
            return 4 * sizeof(float);
        case kUint2_GrSLType:
            return 2 * sizeof(uint32_t);
        case kInt2_GrSLType:
            return 2 * sizeof(int32_t);
        case kInt3_GrSLType:
            return 4 * sizeof(int32_t);
        case kInt4_GrSLType:
            return 4 * sizeof(int32_t);
        case kHalf2x2_GrSLType: // fall through
        case kFloat2x2_GrSLType:
            return 4 * sizeof(float);
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
            break;
    }
    SK_ABORT("Unexpected type");
    return 0;
}

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static void get_ubo_aligned_offset(uint32_t* uniformOffset,
                                   uint32_t* currentOffset,
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
    *uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        *currentOffset = *uniformOffset + grsltype_to_mtl_size(type) * arrayCount;
    } else {
        *currentOffset = *uniformOffset + grsltype_to_mtl_size(type);
    }
}

GrGLSLUniformHandler::UniformHandle GrMtlUniformHandler::internalAddUniformArray(
                                                                            uint32_t visibility,
                                                                            GrSLType type,
                                                                            const char* name,
                                                                            bool mangleName,
                                                                            int arrayCount,
                                                                            const char** outName) {
    SkASSERT(name && strlen(name));
    // For now asserting the the visibility is either geometry types (vertex, tesselation, geometry,
    // etc.) or only fragment.
    SkASSERT(kVertex_GrShaderFlag == visibility ||
             kGeometry_GrShaderFlag == visibility ||
             (kVertex_GrShaderFlag | kGeometry_GrShaderFlag) == visibility ||
             kFragment_GrShaderFlag == visibility);
    GrSLTypeIsFloatType(type);

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

    uint32_t* currentOffset;
    uint32_t* maxAlignment;
    uint32_t geomStages = kVertex_GrShaderFlag | kGeometry_GrShaderFlag;
    if (geomStages & visibility) {
        currentOffset = &fCurrentGeometryUBOOffset;
        maxAlignment = &fCurrentGeometryUBOMaxAlignment;
    } else {
        SkASSERT(kFragment_GrShaderFlag == visibility);
        currentOffset = &fCurrentFragmentUBOOffset;
        maxAlignment = &fCurrentFragmentUBOMaxAlignment;
    }
    get_ubo_aligned_offset(&uni.fUBOffset, currentOffset, maxAlignment, type, arrayCount);

    SkString layoutQualifier;
    layoutQualifier.appendf("offset=%d", uni.fUBOffset);
    uni.fVariable.addLayoutQualifier(layoutQualifier.c_str());

    if (outName) {
        *outName = uni.fVariable.c_str();
    }

    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrMtlUniformHandler::addSampler(const GrTexture* texture,
                                                                    const GrSamplerState&,
                                                                    const GrSwizzle& swizzle,
                                                                    const char* name,
                                                                    const GrShaderCaps* caps) {
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
    layoutQualifier.appendf("binding=%d", fSamplers.count() - 1);
    info.fVariable.addLayoutQualifier(layoutQualifier.c_str());
    info.fVisibility = kFragment_GrShaderFlag;
    info.fUBOffset = 0;
    SkASSERT(caps->textureSwizzleAppliedInShader());
    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplerSwizzles.count() == fSamplers.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrMtlUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    SkASSERT(kVertex_GrShaderFlag == visibility ||
             kGeometry_GrShaderFlag == visibility ||
             kFragment_GrShaderFlag == visibility);

    for (int i = 0; i < fSamplers.count(); ++i) {
        const UniformInfo& sampler = fSamplers[i];
        SkASSERT(sampler.fVariable.getType() == kTexture2DSampler_GrSLType);
        if (visibility == sampler.fVisibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }

#ifdef SK_DEBUG
    bool firstGeomOffsetCheck = false;
    bool firstFragOffsetCheck = false;
    for (int i = 0; i < fUniforms.count(); ++i) {
        const UniformInfo& localUniform = fUniforms[i];
        if (kVertex_GrShaderFlag == localUniform.fVisibility ||
            kGeometry_GrShaderFlag == localUniform.fVisibility ||
            (kVertex_GrShaderFlag | kGeometry_GrShaderFlag) == localUniform.fVisibility) {
            if (!firstGeomOffsetCheck) {
                // Check to make sure we are starting our offset at 0 so the offset qualifier we
                // set on each variable in the uniform block is valid.
                SkASSERT(0 == localUniform.fUBOffset);
                firstGeomOffsetCheck = true;
            }
        } else {
            SkASSERT(kFragment_GrShaderFlag == localUniform.fVisibility);
            if (!firstFragOffsetCheck) {
                // Check to make sure we are starting our offset at 0 so the offset qualifier we
                // set on each variable in the uniform block is valid.
                SkASSERT(0 == localUniform.fUBOffset);
                firstFragOffsetCheck = true;
            }
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
        uint32_t uniformBinding;
        const char* stage;
        if (kVertex_GrShaderFlag == visibility) {
            uniformBinding = kGeometryBinding;
            stage = "vertex";
        } else if (kGeometry_GrShaderFlag == visibility) {
            uniformBinding = kGeometryBinding;
            stage = "geometry";
        } else {
            SkASSERT(kFragment_GrShaderFlag == visibility);
            uniformBinding = kFragBinding;
            stage = "fragment";
        }
        out->appendf("layout (binding=%d) uniform %sUniformBuffer\n{\n", uniformBinding, stage);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}
