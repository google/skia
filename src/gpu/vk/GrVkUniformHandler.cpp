/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkUniformHandler.h"
#include "glsl/GrGLSLProgramBuilder.h"

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
uint32_t grsltype_to_alignment_mask(GrSLType type) {
    SkASSERT(GrSLTypeIsFloatType(type));
    static const uint32_t kAlignments[kGrSLTypeCount] = {
        0x0, // kVoid_GrSLType, should never return this
        0x3, // kFloat_GrSLType
        0x7, // kVec2f_GrSLType
        0xF, // kVec3f_GrSLType
        0xF, // kVec4f_GrSLType
        0xF, // kMat33f_GrSLType
        0xF, // kMat44f_GrSLType
        0x0, // Sampler2D_GrSLType, should never return this
        0x0, // SamplerExternal_GrSLType, should never return this
    };
    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(7 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(8 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kAlignments) == kGrSLTypeCount);
    return kAlignments[type];
}

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
void get_ubo_aligned_offset(uint32_t* uniformOffset,
                            uint32_t* currentOffset,
                            GrSLType type,
                            int arrayCount) {
    uint32_t alignmentMask = grsltype_to_alignment_mask(type);
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    *uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    // We use a 0 arrayCount to indicate it is not an array type but we still need to count the one
    // object.
    int count = arrayCount ? arrayCount : 1;
    *currentOffset = *uniformOffset + count * (uint32_t)GrSLTypeSize(type);
}

GrGLSLUniformHandler::UniformHandle GrVkUniformHandler::internalAddUniformArray(
                                                                            uint32_t visibility,
                                                                            GrSLType type,
                                                                            GrSLPrecision precision,
                                                                            const char* name,
                                                                            bool mangleName,
                                                                            int arrayCount,
                                                                            const char** outName) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_GrShaderFlag|kFragment_GrShaderFlag);
    SkASSERT(0 == (~kVisibilityMask & visibility));
    SkASSERT(0 != visibility);
    SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
    // TODO this is a bit hacky, lets think of a better way.  Basically we need to be able to use
    // the uniform view matrix name in the GP, and the GP is immutable so it has to tell the PB
    // exactly what name it wants to use for the uniform view matrix.  If we prefix anythings, then
    // the names will mismatch.  I think the correct solution is to have all GPs which need the
    // uniform view matrix, they should upload the view matrix in their setData along with regular
    // uniforms.
    char prefix = 'u';
    if ('u' == name[0]) {
        prefix = '\0';
    }
    fProgramBuilder->nameVariable(uni.fVariable.accessName(), prefix, name, mangleName);
    uni.fVariable.setArrayCount(arrayCount);
    // For now asserting the the visibility is either only vertex or only fragment
    SkASSERT(kVertex_GrShaderFlag == visibility || kFragment_GrShaderFlag == visibility);
    uni.fVisibility = visibility;
    uni.fVariable.setPrecision(precision);
    if (GrSLTypeIsFloatType(type)) {
        // When outputing the GLSL, only the outer uniform block will get the Uniform modifier. Thus
        // we set the modifier to none for all uniforms declared inside the block.
        uni.fVariable.setTypeModifier(GrGLSLShaderVar::kNone_TypeModifier);

        uint32_t* currentOffset = kVertex_GrShaderFlag == visibility ? &fCurrentVertexUBOOffset
                                                                     : &fCurrentFragmentUBOOffset;
        get_ubo_aligned_offset(&uni.fUBOffset, currentOffset, type, arrayCount);
        uni.fSetNumber = kUniformBufferDescSet;
        uni.fBinding = kVertex_GrShaderFlag == visibility ? kVertexBinding : kFragBinding;

        if (outName) {
            *outName = uni.fVariable.c_str();
        }
    } else {
        SkASSERT(type == kSampler2D_GrSLType);
        uni.fVariable.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);

        uni.fSetNumber = kSamplerDescSet;
        uni.fBinding = fCurrentSamplerBinding++;
        uni.fUBOffset = 0; // This value will be ignored, but initializing to avoid any errors.
        SkString layoutQualifier;
        layoutQualifier.appendf("set=%d, binding=%d", uni.fSetNumber, uni.fBinding);
        uni.fVariable.setLayoutQualifier(layoutQualifier.c_str());
    }

    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

void GrVkUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    SkTArray<UniformInfo*> uniformBufferUniform;
    // Used to collect all the variables that will be place inside the uniform buffer
    SkString uniformsString;
    SkASSERT(kVertex_GrShaderFlag == visibility || kFragment_GrShaderFlag == visibility);
    uint32_t uniformBinding = (visibility == kVertex_GrShaderFlag) ? kVertexBinding : kFragBinding;
    for (int i = 0; i < fUniforms.count(); ++i) {
        const UniformInfo& localUniform = fUniforms[i];
        if (visibility == localUniform.fVisibility) {
            if (GrSLTypeIsFloatType(localUniform.fVariable.getType())) {
                SkASSERT(uniformBinding == localUniform.fBinding);
                SkASSERT(kUniformBufferDescSet == localUniform.fSetNumber);
                localUniform.fVariable.appendDecl(fProgramBuilder->glslCaps(), &uniformsString);
                uniformsString.append(";\n");
            } else {
                SkASSERT(localUniform.fVariable.getType() == kSampler2D_GrSLType);
                SkASSERT(kSamplerDescSet == localUniform.fSetNumber);
                localUniform.fVariable.appendDecl(fProgramBuilder->glslCaps(), out);
                out->append(";\n");
            }
        }
    }
    if (!uniformsString.isEmpty()) {
        const char* stage = (visibility == kVertex_GrShaderFlag) ? "vertex" : "fragment";
        out->appendf("layout (set=%d, binding=%d) uniform %sUniformBuffer\n{\n",
                     kUniformBufferDescSet, uniformBinding, stage);
        out->appendf("%s\n};\n", uniformsString.c_str());
    }
}