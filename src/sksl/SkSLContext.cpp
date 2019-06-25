/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace SkSL {

static std::vector<const Type*> static_type(const Type& t) {
    return { &t, &t, &t, &t };
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
    return irGenerator->createNode(new Type("fragmentProcessor", fields));
}

Context::Context(IRGenerator* irGenerator)
    : fInvalid_Type(irGenerator->createNode(new Type("<INVALID>")))
    , fVoid_Type(irGenerator->createNode(new Type("void")))
    , fNull_Type(irGenerator->createNode(new Type("null")))
    , fFloatLiteral_Type(irGenerator->createNode(new Type("$floatLiteral", Type::kFloat_NumberKind, 3)))
    , fIntLiteral_Type(irGenerator->createNode(new Type("$intLiteral", Type::kSigned_NumberKind, 1)))
    , fDouble_Type(irGenerator->createNode(new Type("double", Type::kFloat_NumberKind, 6, true)))
    , fDouble2_Type(irGenerator->createNode(new Type("double2", *fDouble_Type, 2)))
    , fDouble3_Type(irGenerator->createNode(new Type("double3", *fDouble_Type, 3)))
    , fDouble4_Type(irGenerator->createNode(new Type("double4", *fDouble_Type, 4)))
    , fFloat_Type(irGenerator->createNode(new Type("float", Type::kFloat_NumberKind, 5, true)))
    , fFloat2_Type(irGenerator->createNode(new Type("float2", *fFloat_Type, 2)))
    , fFloat3_Type(irGenerator->createNode(new Type("float3", *fFloat_Type, 3)))
    , fFloat4_Type(irGenerator->createNode(new Type("float4", *fFloat_Type, 4)))
    , fHalf_Type(irGenerator->createNode(new Type("half", Type::kFloat_NumberKind, 4)))
    , fHalf2_Type(irGenerator->createNode(new Type("half2", *fHalf_Type, 2)))
    , fHalf3_Type(irGenerator->createNode(new Type("half3", *fHalf_Type, 3)))
    , fHalf4_Type(irGenerator->createNode(new Type("half4", *fHalf_Type, 4)))
    , fUInt_Type(irGenerator->createNode(new Type("uint", Type::kUnsigned_NumberKind, 2, true)))
    , fUInt2_Type(irGenerator->createNode(new Type("uint2", *fUInt_Type, 2)))
    , fUInt3_Type(irGenerator->createNode(new Type("uint3", *fUInt_Type, 3)))
    , fUInt4_Type(irGenerator->createNode(new Type("uint4", *fUInt_Type, 4)))
    , fInt_Type(irGenerator->createNode(new Type("int", Type::kSigned_NumberKind, 2, true)))
    , fInt2_Type(irGenerator->createNode(new Type("int2", *fInt_Type, 2)))
    , fInt3_Type(irGenerator->createNode(new Type("int3", *fInt_Type, 3)))
    , fInt4_Type(irGenerator->createNode(new Type("int4", *fInt_Type, 4)))
    , fUShort_Type(irGenerator->createNode(new Type("ushort", Type::kUnsigned_NumberKind, 0)))
    , fUShort2_Type(irGenerator->createNode(new Type("ushort2", *fUShort_Type, 2)))
    , fUShort3_Type(irGenerator->createNode(new Type("ushort3", *fUShort_Type, 3)))
    , fUShort4_Type(irGenerator->createNode(new Type("ushort4", *fUShort_Type, 4)))
    , fShort_Type(irGenerator->createNode(new Type("short", Type::kSigned_NumberKind, 0)))
    , fShort2_Type(irGenerator->createNode(new Type("short2", *fShort_Type, 2)))
    , fShort3_Type(irGenerator->createNode(new Type("short3", *fShort_Type, 3)))
    , fShort4_Type(irGenerator->createNode(new Type("short4", *fShort_Type, 4)))
    , fUByte_Type(irGenerator->createNode(new Type("ubyte", Type::kUnsigned_NumberKind, 0)))
    , fUByte2_Type(irGenerator->createNode(new Type("ubyte2", *fUByte_Type, 2)))
    , fUByte3_Type(irGenerator->createNode(new Type("ubyte3", *fUByte_Type, 3)))
    , fUByte4_Type(irGenerator->createNode(new Type("ubyte4", *fUByte_Type, 4)))
    , fByte_Type(irGenerator->createNode(new Type("byte", Type::kSigned_NumberKind, 0)))
    , fByte2_Type(irGenerator->createNode(new Type("byte2", *fByte_Type, 2)))
    , fByte3_Type(irGenerator->createNode(new Type("byte3", *fByte_Type, 3)))
    , fByte4_Type(irGenerator->createNode(new Type("byte4", *fByte_Type, 4)))
    , fBool_Type(irGenerator->createNode(new Type("bool", Type::kNonnumeric_NumberKind, -1)))
    , fBool2_Type(irGenerator->createNode(new Type("bool2", *fBool_Type, 2)))
    , fBool3_Type(irGenerator->createNode(new Type("bool3", *fBool_Type, 3)))
    , fBool4_Type(irGenerator->createNode(new Type("bool4", *fBool_Type, 4)))
    , fFloat2x2_Type(irGenerator->createNode(new Type("float2x2", *fFloat_Type, 2, 2)))
    , fFloat2x3_Type(irGenerator->createNode(new Type("float2x3", *fFloat_Type, 2, 3)))
    , fFloat2x4_Type(irGenerator->createNode(new Type("float2x4", *fFloat_Type, 2, 4)))
    , fFloat3x2_Type(irGenerator->createNode(new Type("float3x2", *fFloat_Type, 3, 2)))
    , fFloat3x3_Type(irGenerator->createNode(new Type("float3x3", *fFloat_Type, 3, 3)))
    , fFloat3x4_Type(irGenerator->createNode(new Type("float3x4", *fFloat_Type, 3, 4)))
    , fFloat4x2_Type(irGenerator->createNode(new Type("float4x2", *fFloat_Type, 4, 2)))
    , fFloat4x3_Type(irGenerator->createNode(new Type("float4x3", *fFloat_Type, 4, 3)))
    , fFloat4x4_Type(irGenerator->createNode(new Type("float4x4", *fFloat_Type, 4, 4)))
    , fHalf2x2_Type(irGenerator->createNode(new Type("half2x2", *fHalf_Type, 2, 2)))
    , fHalf2x3_Type(irGenerator->createNode(new Type("half2x3", *fHalf_Type, 2, 3)))
    , fHalf2x4_Type(irGenerator->createNode(new Type("half2x4", *fHalf_Type, 2, 4)))
    , fHalf3x2_Type(irGenerator->createNode(new Type("half3x2", *fHalf_Type, 3, 2)))
    , fHalf3x3_Type(irGenerator->createNode(new Type("half3x3", *fHalf_Type, 3, 3)))
    , fHalf3x4_Type(irGenerator->createNode(new Type("half3x4", *fHalf_Type, 3, 4)))
    , fHalf4x2_Type(irGenerator->createNode(new Type("half4x2", *fHalf_Type, 4, 2)))
    , fHalf4x3_Type(irGenerator->createNode(new Type("half4x3", *fHalf_Type, 4, 3)))
    , fHalf4x4_Type(irGenerator->createNode(new Type("half4x4", *fHalf_Type, 4, 4)))
    , fDouble2x2_Type(irGenerator->createNode(new Type("double2x2", *fDouble_Type, 2, 2)))
    , fDouble2x3_Type(irGenerator->createNode(new Type("double2x3", *fDouble_Type, 2, 3)))
    , fDouble2x4_Type(irGenerator->createNode(new Type("double2x4", *fDouble_Type, 2, 4)))
    , fDouble3x2_Type(irGenerator->createNode(new Type("double3x2", *fDouble_Type, 3, 2)))
    , fDouble3x3_Type(irGenerator->createNode(new Type("double3x3", *fDouble_Type, 3, 3)))
    , fDouble3x4_Type(irGenerator->createNode(new Type("double3x4", *fDouble_Type, 3, 4)))
    , fDouble4x2_Type(irGenerator->createNode(new Type("double4x2", *fDouble_Type, 4, 2)))
    , fDouble4x3_Type(irGenerator->createNode(new Type("double4x3", *fDouble_Type, 4, 3)))
    , fDouble4x4_Type(irGenerator->createNode(new Type("double4x4", *fDouble_Type, 4, 4)))
    , fSampler1D_Type(irGenerator->createNode(new Type("sampler1D", SpvDim1D, false, false, false, true)))
    , fSampler2D_Type(irGenerator->createNode(new Type("sampler2D", SpvDim2D, false, false, false, true)))
    , fSampler3D_Type(irGenerator->createNode(new Type("sampler3D", SpvDim3D, false, false, false, true)))
    , fSamplerExternalOES_Type(irGenerator->createNode(new Type("samplerExternalOES", SpvDim2D, false, false,
                                        false, true)))
    , fSamplerCube_Type(irGenerator->createNode(new Type("samplerCube", SpvDimCube, false, false, false, true)))
    , fSampler2DRect_Type(irGenerator->createNode(new Type("sampler2DRect", SpvDimRect, false, false, false, true)))
    , fSampler1DArray_Type(irGenerator->createNode(new Type("sampler1DArray")))
    , fSampler2DArray_Type(irGenerator->createNode(new Type("sampler2DArray")))
    , fSamplerCubeArray_Type(irGenerator->createNode(new Type("samplerCubeArray")))
    , fSamplerBuffer_Type(irGenerator->createNode(new Type("samplerBuffer", SpvDimBuffer, false, false, false,
                                   true)))
    , fSampler2DMS_Type(irGenerator->createNode(new Type("sampler2DMS")))
    , fSampler2DMSArray_Type(irGenerator->createNode(new Type("sampler2DMSArray")))
    , fSampler1DShadow_Type(irGenerator->createNode(new Type("sampler1DShadow")))
    , fSampler2DShadow_Type(irGenerator->createNode(new Type("sampler2DShadow")))
    , fSamplerCubeShadow_Type(irGenerator->createNode(new Type("samplerCubeShadow")))
    , fSampler2DRectShadow_Type(irGenerator->createNode(new Type("sampler2DRectShadow")))
    , fSampler1DArrayShadow_Type(irGenerator->createNode(new Type("sampler1DArrayShadow")))
    , fSampler2DArrayShadow_Type(irGenerator->createNode(new Type("sampler2DArrayShadow")))
    , fSamplerCubeArrayShadow_Type(irGenerator->createNode(new Type("samplerCubeArrayShadow")))

    // Related to below FIXME, gsampler*s don't currently expand to cover integer case.
    , fISampler2D_Type(irGenerator->createNode(new Type("isampler2D", SpvDim2D, false, false, false, true)))

    // FIXME express these as "gimage2D" that expand to image2D, iimage2D, and uimage2D.
    , fImage2D_Type(irGenerator->createNode(new Type("image2D", SpvDim2D, false, false, false, true)))
    , fIImage2D_Type(irGenerator->createNode(new Type("iimage2D", SpvDim2D, false, false, false, true)))

    // FIXME express these as "gsubpassInput" that expand to subpassInput, isubpassInput,
    // and usubpassInput.
    , fSubpassInput_Type(irGenerator->createNode(new Type("subpassInput", SpvDimSubpassData, false, false,
                                  false, false)))
    , fSubpassInputMS_Type(irGenerator->createNode(new Type("subpassInputMS", SpvDimSubpassData, false, false,
                                    true, false)))

    // FIXME figure out what we're supposed to do with the gsampler et al. types)
    , fGSampler1D_Type(irGenerator->createNode(new Type("$gsampler1D", static_type(*fSampler1D_Type))))
    , fGSampler2D_Type(irGenerator->createNode(new Type("$gsampler2D", static_type(*fSampler2D_Type))))
    , fGSampler3D_Type(irGenerator->createNode(new Type("$gsampler3D", static_type(*fSampler3D_Type))))
    , fGSamplerCube_Type(irGenerator->createNode(new Type("$gsamplerCube", static_type(*fSamplerCube_Type))))
    , fGSampler2DRect_Type(irGenerator->createNode(new Type("$gsampler2DRect", static_type(*fSampler2DRect_Type))))
    , fGSampler1DArray_Type(irGenerator->createNode(new Type("$gsampler1DArray",
                                     static_type(*fSampler1DArray_Type))))
    , fGSampler2DArray_Type(irGenerator->createNode(new Type("$gsampler2DArray",
                                     static_type(*fSampler2DArray_Type))))
    , fGSamplerCubeArray_Type(irGenerator->createNode(new Type("$gsamplerCubeArray",
                                       static_type(*fSamplerCubeArray_Type))))
    , fGSamplerBuffer_Type(irGenerator->createNode(new Type("$gsamplerBuffer", static_type(*fSamplerBuffer_Type))))
    , fGSampler2DMS_Type(irGenerator->createNode(new Type("$gsampler2DMS", static_type(*fSampler2DMS_Type))))
    , fGSampler2DMSArray_Type(irGenerator->createNode(new Type("$gsampler2DMSArray",
                                       static_type(*fSampler2DMSArray_Type))))
    , fGSampler2DArrayShadow_Type(irGenerator->createNode(new Type("$gsampler2DArrayShadow",
                                           static_type(*fSampler2DArrayShadow_Type))))
    , fGSamplerCubeArrayShadow_Type(irGenerator->createNode(new Type("$gsamplerCubeArrayShadow",
                                             static_type(*fSamplerCubeArrayShadow_Type))))
    , fGenType_Type(irGenerator->createNode(new Type("$genType", { fFloat_Type.get(), fFloat2_Type.get(),
                                           fFloat3_Type.get(), fFloat4_Type.get() })))
    , fGenHType_Type(irGenerator->createNode(new Type("$genHType", { fHalf_Type.get(), fHalf2_Type.get(),
                                             fHalf3_Type.get(), fHalf4_Type.get() })))
    , fGenDType_Type(irGenerator->createNode(new Type("$genDType", { fDouble_Type.get(), fDouble2_Type.get(),
                                             fDouble3_Type.get(), fDouble4_Type.get() })))
    , fGenIType_Type(irGenerator->createNode(new Type("$genIType", { fInt_Type.get(), fInt2_Type.get(),
                                             fInt3_Type.get(), fInt4_Type.get() })))
    , fGenUType_Type(irGenerator->createNode(new Type("$genUType", { fUInt_Type.get(), fUInt2_Type.get(),
                                             fUInt3_Type.get(), fUInt4_Type.get() })))
    , fGenBType_Type(irGenerator->createNode(new Type("$genBType", { fBool_Type.get(), fBool2_Type.get(),
                                             fBool3_Type.get(), fBool4_Type.get() })))
    , fMat_Type(irGenerator->createNode(new Type("$mat", { fFloat2x2_Type.get(),  fFloat2x3_Type.get(),
                                   fFloat2x4_Type.get(),  fFloat3x2_Type.get(),
                                   fFloat3x3_Type.get(),  fFloat3x4_Type.get(),
                                   fFloat4x2_Type.get(),  fFloat4x3_Type.get(),
                                   fFloat4x4_Type.get(),  fHalf2x2_Type.get(),
                                   fHalf2x3_Type.get(),   fHalf2x4_Type.get(),
                                   fHalf3x2_Type.get(),   fHalf3x3_Type.get(),
                                   fHalf3x4_Type.get(),   fHalf4x2_Type.get(),
                                   fHalf4x3_Type.get(),   fHalf4x4_Type.get(),
                                   fDouble2x2_Type.get(), fDouble2x3_Type.get(),
                                   fDouble2x4_Type.get(), fDouble3x2_Type.get(),
                                   fDouble3x3_Type.get(), fDouble3x4_Type.get(),
                                   fDouble4x2_Type.get(), fDouble4x3_Type.get(),
                                   fDouble4x4_Type.get() })))
    , fVec_Type(irGenerator->createNode(new Type("$vec", { fInvalid_Type.get(), fFloat2_Type.get(),
                                           fFloat3_Type.get(), fFloat4_Type.get() })))
    , fGVec_Type(irGenerator->createNode(new Type("$gvec")))
    , fGVec2_Type(irGenerator->createNode(new Type("$gfloat2")))
    , fGVec3_Type(irGenerator->createNode(new Type("$gfloat3")))
    , fGVec4_Type(irGenerator->createNode(new Type("$gfloat4", static_type(*fFloat4_Type))))
    , fHVec_Type(irGenerator->createNode(new Type("$hvec", { fInvalid_Type.get(), fHalf2_Type.get(),
                                     fHalf3_Type.get(), fHalf4_Type.get() })))
    , fDVec_Type(irGenerator->createNode(new Type("$dvec", { fInvalid_Type.get(), fDouble2_Type.get(),
                                     fDouble3_Type.get(), fDouble4_Type.get() })))
    , fIVec_Type(irGenerator->createNode(new Type("$ivec", { fInvalid_Type.get(), fInt2_Type.get(),
                                     fInt3_Type.get(), fInt4_Type.get() })))
    , fUVec_Type(irGenerator->createNode(new Type("$uvec", { fInvalid_Type.get(), fUInt2_Type.get(),
                                     fUInt3_Type.get(), fUInt4_Type.get() })))
    , fSVec_Type(irGenerator->createNode(new Type("$svec", { fInvalid_Type.get(), fShort2_Type.get(),
                                     fShort3_Type.get(), fShort4_Type.get() })))
    , fUSVec_Type(irGenerator->createNode(new Type("$usvec", { fInvalid_Type.get(), fUShort2_Type.get(),
                                       fUShort3_Type.get(), fUShort4_Type.get() })))
    , fByteVec_Type(irGenerator->createNode(new Type("$bytevec", { fInvalid_Type.get(), fByte2_Type.get(),
                                     fByte3_Type.get(), fByte4_Type.get() })))
    , fUByteVec_Type(irGenerator->createNode(new Type("$ubytevec", { fInvalid_Type.get(), fUByte2_Type.get(),
                                       fUByte3_Type.get(), fUByte4_Type.get() })))
    , fBVec_Type(irGenerator->createNode(new Type("$bvec", { fInvalid_Type.get(), fBool2_Type.get(),
                                     fBool3_Type.get(), fBool4_Type.get() })))
    , fSkCaps_Type(irGenerator->createNode(new Type("$sk_Caps")))
    , fSkArgs_Type(irGenerator->createNode(new Type("$sk_Args")))
    , fFragmentProcessor_Type(fp_type(fInt_Type.get(), fBool_Type.get()))
    , fSkRasterPipeline_Type(irGenerator->createNode(new Type("SkRasterPipeline")))
    , fDefined_Expression(new Defined(*fInvalid_Type)) {}

IRNode::ID Context::Defined::clone(const IRGenerator& irGenerator) const override {
    return irGenerator.createNode(new Defined(fType));
}

} // namespace
