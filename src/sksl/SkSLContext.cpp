/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLContext.h"

#include "SkSLIRGenerator.h"

namespace SkSL {

static std::vector<IRNode::ID> static_type(IRNode::ID t) {
    return { t, t, t, t };
}

static IRNode::ID fp_type(IRNode::ID intType, IRNode::ID boolType) {
    // Build fields for FragmentProcessors, which should parallel the
    // C++ API for GrFragmentProcessor.
    Modifiers mods(Layout(), Modifiers::kConst_Flag);
    std::vector<Type::Field> fields = {
        Type::Field(mods, "numTextureSamplers", intType),
        Type::Field(mods, "numCoordTransforms", intType),
        Type::Field(mods, "numChildProcessors", intType),
        Type::Field(mods, "usesLocalCoords", boolType),
        Type::Field(mods, "compatibleWithCoverageAsAlpha", boolType),
        Type::Field(mods, "preservesOpaqueInput", boolType),
        Type::Field(mods, "hasConstantOutputForConstantInput", boolType)
    };
    return intType.fIRGenerator->createNode(new Type(intType.fIRGenerator, "fragmentProcessor",
                                                     fields));
}

Context::Context(IRGenerator* irGenerator)
    : fInvalid_Type(irGenerator->createNode(new Type(irGenerator, "<INVALID>")))
    , fVoid_Type(irGenerator->createNode(new Type(irGenerator, "void")))
    , fNull_Type(irGenerator->createNode(new Type(irGenerator, "null")))
    , fFloatLiteral_Type(irGenerator->createNode(new Type(irGenerator, "$floatLiteral", Type::kFloat_NumberKind, 3)))
    , fIntLiteral_Type(irGenerator->createNode(new Type(irGenerator, "$intLiteral", Type::kSigned_NumberKind, 1)))
    , fDouble_Type(irGenerator->createNode(new Type(irGenerator, "double", Type::kFloat_NumberKind, 6, true)))
    , fDouble2_Type(irGenerator->createNode(new Type(irGenerator, "double2", fDouble_Type, 2)))
    , fDouble3_Type(irGenerator->createNode(new Type(irGenerator, "double3", fDouble_Type, 3)))
    , fDouble4_Type(irGenerator->createNode(new Type(irGenerator, "double4", fDouble_Type, 4)))
    , fFloat_Type(irGenerator->createNode(new Type(irGenerator, "float", Type::kFloat_NumberKind, 5, true)))
    , fFloat2_Type(irGenerator->createNode(new Type(irGenerator, "float2", fFloat_Type, 2)))
    , fFloat3_Type(irGenerator->createNode(new Type(irGenerator, "float3", fFloat_Type, 3)))
    , fFloat4_Type(irGenerator->createNode(new Type(irGenerator, "float4", fFloat_Type, 4)))
    , fHalf_Type(irGenerator->createNode(new Type(irGenerator, "half", Type::kFloat_NumberKind, 4)))
    , fHalf2_Type(irGenerator->createNode(new Type(irGenerator, "half2", fHalf_Type, 2)))
    , fHalf3_Type(irGenerator->createNode(new Type(irGenerator, "half3", fHalf_Type, 3)))
    , fHalf4_Type(irGenerator->createNode(new Type(irGenerator, "half4", fHalf_Type, 4)))
    , fUInt_Type(irGenerator->createNode(new Type(irGenerator, "uint", Type::kUnsigned_NumberKind, 2, true)))
    , fUInt2_Type(irGenerator->createNode(new Type(irGenerator, "uint2", fUInt_Type, 2)))
    , fUInt3_Type(irGenerator->createNode(new Type(irGenerator, "uint3", fUInt_Type, 3)))
    , fUInt4_Type(irGenerator->createNode(new Type(irGenerator, "uint4", fUInt_Type, 4)))
    , fInt_Type(irGenerator->createNode(new Type(irGenerator, "int", Type::kSigned_NumberKind, 2, true)))
    , fInt2_Type(irGenerator->createNode(new Type(irGenerator, "int2", fInt_Type, 2)))
    , fInt3_Type(irGenerator->createNode(new Type(irGenerator, "int3", fInt_Type, 3)))
    , fInt4_Type(irGenerator->createNode(new Type(irGenerator, "int4", fInt_Type, 4)))
    , fUShort_Type(irGenerator->createNode(new Type(irGenerator, "ushort", Type::kUnsigned_NumberKind, 0)))
    , fUShort2_Type(irGenerator->createNode(new Type(irGenerator, "ushort2", fUShort_Type, 2)))
    , fUShort3_Type(irGenerator->createNode(new Type(irGenerator, "ushort3", fUShort_Type, 3)))
    , fUShort4_Type(irGenerator->createNode(new Type(irGenerator, "ushort4", fUShort_Type, 4)))
    , fShort_Type(irGenerator->createNode(new Type(irGenerator, "short", Type::kSigned_NumberKind, 0)))
    , fShort2_Type(irGenerator->createNode(new Type(irGenerator, "short2", fShort_Type, 2)))
    , fShort3_Type(irGenerator->createNode(new Type(irGenerator, "short3", fShort_Type, 3)))
    , fShort4_Type(irGenerator->createNode(new Type(irGenerator, "short4", fShort_Type, 4)))
    , fUByte_Type(irGenerator->createNode(new Type(irGenerator, "ubyte", Type::kUnsigned_NumberKind, 0)))
    , fUByte2_Type(irGenerator->createNode(new Type(irGenerator, "ubyte2", fUByte_Type, 2)))
    , fUByte3_Type(irGenerator->createNode(new Type(irGenerator, "ubyte3", fUByte_Type, 3)))
    , fUByte4_Type(irGenerator->createNode(new Type(irGenerator, "ubyte4", fUByte_Type, 4)))
    , fByte_Type(irGenerator->createNode(new Type(irGenerator, "byte", Type::kSigned_NumberKind, 0)))
    , fByte2_Type(irGenerator->createNode(new Type(irGenerator, "byte2", fByte_Type, 2)))
    , fByte3_Type(irGenerator->createNode(new Type(irGenerator, "byte3", fByte_Type, 3)))
    , fByte4_Type(irGenerator->createNode(new Type(irGenerator, "byte4", fByte_Type, 4)))
    , fBool_Type(irGenerator->createNode(new Type(irGenerator, "bool", Type::kNonnumeric_NumberKind, -1)))
    , fBool2_Type(irGenerator->createNode(new Type(irGenerator, "bool2", fBool_Type, 2)))
    , fBool3_Type(irGenerator->createNode(new Type(irGenerator, "bool3", fBool_Type, 3)))
    , fBool4_Type(irGenerator->createNode(new Type(irGenerator, "bool4", fBool_Type, 4)))
    , fFloat2x2_Type(irGenerator->createNode(new Type(irGenerator, "float2x2", fFloat_Type, 2, 2)))
    , fFloat2x3_Type(irGenerator->createNode(new Type(irGenerator, "float2x3", fFloat_Type, 2, 3)))
    , fFloat2x4_Type(irGenerator->createNode(new Type(irGenerator, "float2x4", fFloat_Type, 2, 4)))
    , fFloat3x2_Type(irGenerator->createNode(new Type(irGenerator, "float3x2", fFloat_Type, 3, 2)))
    , fFloat3x3_Type(irGenerator->createNode(new Type(irGenerator, "float3x3", fFloat_Type, 3, 3)))
    , fFloat3x4_Type(irGenerator->createNode(new Type(irGenerator, "float3x4", fFloat_Type, 3, 4)))
    , fFloat4x2_Type(irGenerator->createNode(new Type(irGenerator, "float4x2", fFloat_Type, 4, 2)))
    , fFloat4x3_Type(irGenerator->createNode(new Type(irGenerator, "float4x3", fFloat_Type, 4, 3)))
    , fFloat4x4_Type(irGenerator->createNode(new Type(irGenerator, "float4x4", fFloat_Type, 4, 4)))
    , fHalf2x2_Type(irGenerator->createNode(new Type(irGenerator, "half2x2", fHalf_Type, 2, 2)))
    , fHalf2x3_Type(irGenerator->createNode(new Type(irGenerator, "half2x3", fHalf_Type, 2, 3)))
    , fHalf2x4_Type(irGenerator->createNode(new Type(irGenerator, "half2x4", fHalf_Type, 2, 4)))
    , fHalf3x2_Type(irGenerator->createNode(new Type(irGenerator, "half3x2", fHalf_Type, 3, 2)))
    , fHalf3x3_Type(irGenerator->createNode(new Type(irGenerator, "half3x3", fHalf_Type, 3, 3)))
    , fHalf3x4_Type(irGenerator->createNode(new Type(irGenerator, "half3x4", fHalf_Type, 3, 4)))
    , fHalf4x2_Type(irGenerator->createNode(new Type(irGenerator, "half4x2", fHalf_Type, 4, 2)))
    , fHalf4x3_Type(irGenerator->createNode(new Type(irGenerator, "half4x3", fHalf_Type, 4, 3)))
    , fHalf4x4_Type(irGenerator->createNode(new Type(irGenerator, "half4x4", fHalf_Type, 4, 4)))
    , fDouble2x2_Type(irGenerator->createNode(new Type(irGenerator, "double2x2", fDouble_Type, 2, 2)))
    , fDouble2x3_Type(irGenerator->createNode(new Type(irGenerator, "double2x3", fDouble_Type, 2, 3)))
    , fDouble2x4_Type(irGenerator->createNode(new Type(irGenerator, "double2x4", fDouble_Type, 2, 4)))
    , fDouble3x2_Type(irGenerator->createNode(new Type(irGenerator, "double3x2", fDouble_Type, 3, 2)))
    , fDouble3x3_Type(irGenerator->createNode(new Type(irGenerator, "double3x3", fDouble_Type, 3, 3)))
    , fDouble3x4_Type(irGenerator->createNode(new Type(irGenerator, "double3x4", fDouble_Type, 3, 4)))
    , fDouble4x2_Type(irGenerator->createNode(new Type(irGenerator, "double4x2", fDouble_Type, 4, 2)))
    , fDouble4x3_Type(irGenerator->createNode(new Type(irGenerator, "double4x3", fDouble_Type, 4, 3)))
    , fDouble4x4_Type(irGenerator->createNode(new Type(irGenerator, "double4x4", fDouble_Type, 4, 4)))
    , fSampler1D_Type(irGenerator->createNode(new Type(irGenerator, "sampler1D", SpvDim1D, false, false, false, true)))
    , fSampler2D_Type(irGenerator->createNode(new Type(irGenerator, "sampler2D", SpvDim2D, false, false, false, true)))
    , fSampler3D_Type(irGenerator->createNode(new Type(irGenerator, "sampler3D", SpvDim3D, false, false, false, true)))
    , fSamplerExternalOES_Type(irGenerator->createNode(new Type(irGenerator, "samplerExternalOES", SpvDim2D, false, false,
                                        false, true)))
    , fSamplerCube_Type(irGenerator->createNode(new Type(irGenerator, "samplerCube", SpvDimCube, false, false, false, true)))
    , fSampler2DRect_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DRect", SpvDimRect, false, false, false, true)))
    , fSampler1DArray_Type(irGenerator->createNode(new Type(irGenerator, "sampler1DArray")))
    , fSampler2DArray_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DArray")))
    , fSamplerCubeArray_Type(irGenerator->createNode(new Type(irGenerator, "samplerCubeArray")))
    , fSamplerBuffer_Type(irGenerator->createNode(new Type(irGenerator, "samplerBuffer", SpvDimBuffer, false, false, false,
                                   true)))
    , fSampler2DMS_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DMS")))
    , fSampler2DMSArray_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DMSArray")))
    , fSampler1DShadow_Type(irGenerator->createNode(new Type(irGenerator, "sampler1DShadow")))
    , fSampler2DShadow_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DShadow")))
    , fSamplerCubeShadow_Type(irGenerator->createNode(new Type(irGenerator, "samplerCubeShadow")))
    , fSampler2DRectShadow_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DRectShadow")))
    , fSampler1DArrayShadow_Type(irGenerator->createNode(new Type(irGenerator, "sampler1DArrayShadow")))
    , fSampler2DArrayShadow_Type(irGenerator->createNode(new Type(irGenerator, "sampler2DArrayShadow")))
    , fSamplerCubeArrayShadow_Type(irGenerator->createNode(new Type(irGenerator, "samplerCubeArrayShadow")))

    // Related to below FIXME, gsampler*s don't currently expand to cover integer case.
    , fISampler2D_Type(irGenerator->createNode(new Type(irGenerator, "isampler2D", SpvDim2D, false, false, false, true)))

    // FIXME express these as "gimage2D" that expand to image2D, iimage2D, and uimage2D.
    , fImage2D_Type(irGenerator->createNode(new Type(irGenerator, "image2D", SpvDim2D, false, false, false, true)))
    , fIImage2D_Type(irGenerator->createNode(new Type(irGenerator, "iimage2D", SpvDim2D, false, false, false, true)))

    // FIXME express these as "gsubpassInput" that expand to subpassInput, isubpassInput,
    // and usubpassInput.
    , fSubpassInput_Type(irGenerator->createNode(new Type(irGenerator, "subpassInput", SpvDimSubpassData, false, false,
                                  false, false)))
    , fSubpassInputMS_Type(irGenerator->createNode(new Type(irGenerator, "subpassInputMS", SpvDimSubpassData, false, false,
                                    true, false)))

    // FIXME figure out what we're supposed to do with the gsampler et al. types)
    , fGSampler1D_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler1D", static_type(fSampler1D_Type))))
    , fGSampler2D_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2D", static_type(fSampler2D_Type))))
    , fGSampler3D_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler3D", static_type(fSampler3D_Type))))
    , fGSamplerCube_Type(irGenerator->createNode(new Type(irGenerator, "$gsamplerCube", static_type(fSamplerCube_Type))))
    , fGSampler2DRect_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2DRect", static_type(fSampler2DRect_Type))))
    , fGSampler1DArray_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler1DArray",
                                     static_type(fSampler1DArray_Type))))
    , fGSampler2DArray_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2DArray",
                                     static_type(fSampler2DArray_Type))))
    , fGSamplerCubeArray_Type(irGenerator->createNode(new Type(irGenerator, "$gsamplerCubeArray",
                                       static_type(fSamplerCubeArray_Type))))
    , fGSamplerBuffer_Type(irGenerator->createNode(new Type(irGenerator, "$gsamplerBuffer", static_type(fSamplerBuffer_Type))))
    , fGSampler2DMS_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2DMS", static_type(fSampler2DMS_Type))))
    , fGSampler2DMSArray_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2DMSArray",
                                       static_type(fSampler2DMSArray_Type))))
    , fGSampler2DArrayShadow_Type(irGenerator->createNode(new Type(irGenerator, "$gsampler2DArrayShadow",
                                           static_type(fSampler2DArrayShadow_Type))))
    , fGSamplerCubeArrayShadow_Type(irGenerator->createNode(new Type(irGenerator, "$gsamplerCubeArrayShadow",
                                             static_type(fSamplerCubeArrayShadow_Type))))
    , fGenType_Type(irGenerator->createNode(new Type(irGenerator, "$genType", { fFloat_Type, fFloat2_Type,
                                            fFloat3_Type, fFloat4_Type })))
    , fGenHType_Type(irGenerator->createNode(new Type(irGenerator, "$genHType", { fHalf_Type, fHalf2_Type,
                                             fHalf3_Type, fHalf4_Type })))
    , fGenDType_Type(irGenerator->createNode(new Type(irGenerator, "$genDType", { fDouble_Type, fDouble2_Type,
                                             fDouble3_Type, fDouble4_Type })))
    , fGenIType_Type(irGenerator->createNode(new Type(irGenerator, "$genIType", { fInt_Type, fInt2_Type,
                                             fInt3_Type, fInt4_Type })))
    , fGenUType_Type(irGenerator->createNode(new Type(irGenerator, "$genUType", { fUInt_Type, fUInt2_Type,
                                             fUInt3_Type, fUInt4_Type })))
    , fGenBType_Type(irGenerator->createNode(new Type(irGenerator, "$genBType", { fBool_Type, fBool2_Type,
                                             fBool3_Type, fBool4_Type })))
    , fMat_Type(irGenerator->createNode(new Type(irGenerator, "$mat", { fFloat2x2_Type,  fFloat2x3_Type,
                                   fFloat2x4_Type,  fFloat3x2_Type,
                                   fFloat3x3_Type,  fFloat3x4_Type,
                                   fFloat4x2_Type,  fFloat4x3_Type,
                                   fFloat4x4_Type,  fHalf2x2_Type,
                                   fHalf2x3_Type,   fHalf2x4_Type,
                                   fHalf3x2_Type,   fHalf3x3_Type,
                                   fHalf3x4_Type,   fHalf4x2_Type,
                                   fHalf4x3_Type,   fHalf4x4_Type,
                                   fDouble2x2_Type, fDouble2x3_Type,
                                   fDouble2x4_Type, fDouble3x2_Type,
                                   fDouble3x3_Type, fDouble3x4_Type,
                                   fDouble4x2_Type, fDouble4x3_Type,
                                   fDouble4x4_Type })))
    , fVec_Type(irGenerator->createNode(new Type(irGenerator, "$vec", { fInvalid_Type, fFloat2_Type,
                                           fFloat3_Type, fFloat4_Type })))
    , fGVec_Type(irGenerator->createNode(new Type(irGenerator, "$gvec")))
    , fGVec2_Type(irGenerator->createNode(new Type(irGenerator, "$gfloat2")))
    , fGVec3_Type(irGenerator->createNode(new Type(irGenerator, "$gfloat3")))
    , fGVec4_Type(irGenerator->createNode(new Type(irGenerator, "$gfloat4", static_type(fFloat4_Type))))
    , fHVec_Type(irGenerator->createNode(new Type(irGenerator, "$hvec", { fInvalid_Type, fHalf2_Type,
                                     fHalf3_Type, fHalf4_Type })))
    , fDVec_Type(irGenerator->createNode(new Type(irGenerator, "$dvec", { fInvalid_Type, fDouble2_Type,
                                     fDouble3_Type, fDouble4_Type })))
    , fIVec_Type(irGenerator->createNode(new Type(irGenerator, "$ivec", { fInvalid_Type, fInt2_Type,
                                     fInt3_Type, fInt4_Type })))
    , fUVec_Type(irGenerator->createNode(new Type(irGenerator, "$uvec", { fInvalid_Type, fUInt2_Type,
                                     fUInt3_Type, fUInt4_Type })))
    , fSVec_Type(irGenerator->createNode(new Type(irGenerator, "$svec", { fInvalid_Type, fShort2_Type,
                                     fShort3_Type, fShort4_Type })))
    , fUSVec_Type(irGenerator->createNode(new Type(irGenerator, "$usvec", { fInvalid_Type, fUShort2_Type,
                                       fUShort3_Type, fUShort4_Type })))
    , fByteVec_Type(irGenerator->createNode(new Type(irGenerator, "$bytevec", { fInvalid_Type, fByte2_Type,
                                     fByte3_Type, fByte4_Type })))
    , fUByteVec_Type(irGenerator->createNode(new Type(irGenerator, "$ubytevec", { fInvalid_Type, fUByte2_Type,
                                       fUByte3_Type, fUByte4_Type })))
    , fBVec_Type(irGenerator->createNode(new Type(irGenerator, "$bvec", { fInvalid_Type, fBool2_Type,
                                     fBool3_Type, fBool4_Type })))
    , fSkCaps_Type(irGenerator->createNode(new Type(irGenerator, "$sk_Caps")))
    , fSkArgs_Type(irGenerator->createNode(new Type(irGenerator, "$sk_Args")))
    , fFragmentProcessor_Type(fp_type(fInt_Type, fBool_Type))
    , fSkRasterPipeline_Type(irGenerator->createNode(new Type(irGenerator, "SkRasterPipeline")))
    , fDefined_Expression(irGenerator->createNode(new Defined(fInvalid_Type))) {}

IRNode::ID Context::Defined::clone() const {
    return fIRGenerator->createNode(new Defined(fType));
}

} // namespace
