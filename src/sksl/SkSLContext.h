/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include <memory>

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context()
            : fInvalid_Type(Type::MakeOtherType("<INVALID>"))
            , fVoid_Type(Type::MakeOtherType("void"))
            , fNull_Type(Type::MakeOtherType("null"))
            , fFloatLiteral_Type(Type::MakeScalarType("$floatLiteral", Type::NumberKind::kFloat,
                                                                       /*priority=*/8))
            , fIntLiteral_Type(Type::MakeScalarType("$intLiteral", Type::NumberKind::kSigned,
                                                                   /*priority=*/5))
            , fFloat_Type(Type::MakeScalarType("float", Type::NumberKind::kFloat, /*priority=*/10,
                                                        /*highPrecision=*/true))
            , fFloat2_Type(Type::MakeVectorType("float2", *fFloat_Type, /*columns=*/2))
            , fFloat3_Type(Type::MakeVectorType("float3", *fFloat_Type, /*columns=*/3))
            , fFloat4_Type(Type::MakeVectorType("float4", *fFloat_Type, /*columns=*/4))
            , fHalf_Type(Type::MakeScalarType("half", Type::NumberKind::kFloat, /*priority=*/9))
            , fHalf2_Type(Type::MakeVectorType("half2", *fHalf_Type, /*columns=*/2))
            , fHalf3_Type(Type::MakeVectorType("half3", *fHalf_Type, /*columns=*/3))
            , fHalf4_Type(Type::MakeVectorType("half4", *fHalf_Type, /*columns=*/4))
            , fInt_Type(Type::MakeScalarType("int", Type::NumberKind::kSigned, /*priority=*/7,
                                                    /*highPrecision=*/true))
            , fInt2_Type(Type::MakeVectorType("int2", *fInt_Type, /*columns=*/2))
            , fInt3_Type(Type::MakeVectorType("int3", *fInt_Type, /*columns=*/3))
            , fInt4_Type(Type::MakeVectorType("int4", *fInt_Type, /*columns=*/4))
            , fUInt_Type(Type::MakeScalarType("uint", Type::NumberKind::kUnsigned, /*priority=*/6,
                                                      /*highPrecision=*/true))
            , fUInt2_Type(Type::MakeVectorType("uint2", *fUInt_Type, /*columns=*/2))
            , fUInt3_Type(Type::MakeVectorType("uint3", *fUInt_Type, /*columns=*/3))
            , fUInt4_Type(Type::MakeVectorType("uint4", *fUInt_Type, /*columns=*/4))
            , fShort_Type(Type::MakeScalarType("short", Type::NumberKind::kSigned, /*priority=*/4))
            , fShort2_Type(Type::MakeVectorType("short2", *fShort_Type, /*columns=*/2))
            , fShort3_Type(Type::MakeVectorType("short3", *fShort_Type, /*columns=*/3))
            , fShort4_Type(Type::MakeVectorType("short4", *fShort_Type, /*columns=*/4))
            , fUShort_Type(Type::MakeScalarType("ushort", Type::NumberKind::kUnsigned,
                                                          /*priority=*/3))
            , fUShort2_Type(Type::MakeVectorType("ushort2", *fUShort_Type, /*columns=*/2))
            , fUShort3_Type(Type::MakeVectorType("ushort3", *fUShort_Type, /*columns=*/3))
            , fUShort4_Type(Type::MakeVectorType("ushort4", *fUShort_Type, /*columns=*/4))
            , fByte_Type(Type::MakeScalarType("byte", Type::NumberKind::kSigned, /*priority=*/2))
            , fByte2_Type(Type::MakeVectorType("byte2", *fByte_Type, /*columns=*/2))
            , fByte3_Type(Type::MakeVectorType("byte3", *fByte_Type, /*columns=*/3))
            , fByte4_Type(Type::MakeVectorType("byte4", *fByte_Type, /*columns=*/4))
            , fUByte_Type(Type::MakeScalarType("ubyte", Type::NumberKind::kUnsigned,
                                                        /*priority=*/1))
            , fUByte2_Type(Type::MakeVectorType("ubyte2", *fUByte_Type, /*columns=*/2))
            , fUByte3_Type(Type::MakeVectorType("ubyte3", *fUByte_Type, /*columns=*/3))
            , fUByte4_Type(Type::MakeVectorType("ubyte4", *fUByte_Type, /*columns=*/4))
            , fBool_Type(Type::MakeScalarType("bool", Type::NumberKind::kBoolean, /*priority=*/0))
            , fBool2_Type(Type::MakeVectorType("bool2", *fBool_Type, /*columns=*/2))
            , fBool3_Type(Type::MakeVectorType("bool3", *fBool_Type, /*columns=*/3))
            , fBool4_Type(Type::MakeVectorType("bool4", *fBool_Type, /*columns=*/4))
            , fFloat2x2_Type(Type::MakeMatrixType("float2x2", *fFloat_Type,
                                                              /*columns=*/2, /*rows=*/2))
            , fFloat2x3_Type(Type::MakeMatrixType("float2x3", *fFloat_Type,
                                                              /*columns=*/2, /*rows=*/3))
            , fFloat2x4_Type(Type::MakeMatrixType("float2x4", *fFloat_Type,
                                                              /*columns=*/2, /*rows=*/4))
            , fFloat3x2_Type(Type::MakeMatrixType("float3x2", *fFloat_Type,
                                                              /*columns=*/3, /*rows=*/2))
            , fFloat3x3_Type(Type::MakeMatrixType("float3x3", *fFloat_Type,
                                                              /*columns=*/3, /*rows=*/3))
            , fFloat3x4_Type(Type::MakeMatrixType("float3x4", *fFloat_Type,
                                                              /*columns=*/3, /*rows=*/4))
            , fFloat4x2_Type(Type::MakeMatrixType("float4x2", *fFloat_Type,
                                                              /*columns=*/4, /*rows=*/2))
            , fFloat4x3_Type(Type::MakeMatrixType("float4x3", *fFloat_Type,
                                                              /*columns=*/4, /*rows=*/3))
            , fFloat4x4_Type(Type::MakeMatrixType("float4x4", *fFloat_Type,
                                                              /*columns=*/4, /*rows=*/4))
            , fHalf2x2_Type(Type::MakeMatrixType("half2x2", *fHalf_Type, /*columns=*/2, /*rows=*/2))
            , fHalf2x3_Type(Type::MakeMatrixType("half2x3", *fHalf_Type, /*columns=*/2, /*rows=*/3))
            , fHalf2x4_Type(Type::MakeMatrixType("half2x4", *fHalf_Type, /*columns=*/2, /*rows=*/4))
            , fHalf3x2_Type(Type::MakeMatrixType("half3x2", *fHalf_Type, /*columns=*/3, /*rows=*/2))
            , fHalf3x3_Type(Type::MakeMatrixType("half3x3", *fHalf_Type, /*columns=*/3, /*rows=*/3))
            , fHalf3x4_Type(Type::MakeMatrixType("half3x4", *fHalf_Type, /*columns=*/3, /*rows=*/4))
            , fHalf4x2_Type(Type::MakeMatrixType("half4x2", *fHalf_Type, /*columns=*/4, /*rows=*/2))
            , fHalf4x3_Type(Type::MakeMatrixType("half4x3", *fHalf_Type, /*columns=*/4, /*rows=*/3))
            , fHalf4x4_Type(Type::MakeMatrixType("half4x4", *fHalf_Type, /*columns=*/4, /*rows=*/4))
            , fTexture1D_Type(Type::MakeTextureType("texture1D",
                                                    SpvDim1D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
            , fTexture2D_Type(Type::MakeTextureType("texture2D",
                                                    SpvDim2D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
            , fTexture3D_Type(Type::MakeTextureType("texture3D",
                                                    SpvDim3D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
            , fTextureExternalOES_Type(Type::MakeTextureType("textureExternalOES",
                                                             SpvDim2D,
                                                             /*isDepth=*/false,
                                                             /*isArrayedTexture=*/false,
                                                             /*isMultisampled=*/false,
                                                             /*isSampled=*/true))
            , fTextureCube_Type(Type::MakeTextureType("textureCube",
                                                      SpvDimCube,
                                                      /*isDepth=*/false,
                                                      /*isArrayedTexture=*/false,
                                                      /*isMultisampled=*/false,
                                                      /*isSampled=*/true))
            , fTexture2DRect_Type(Type::MakeTextureType("texture2DRect",
                                                        SpvDimRect,
                                                        /*isDepth=*/false,
                                                        /*isArrayedTexture=*/false,
                                                        /*isMultisampled=*/false,
                                                        /*isSampled=*/true))
            , fTextureBuffer_Type(Type::MakeTextureType("textureBuffer",
                                                        SpvDimBuffer,
                                                        /*isDepth=*/false,
                                                        /*isArrayedTexture=*/false,
                                                        /*isMultisampled=*/false,
                                                        /*isSampled=*/true))
            , fITexture2D_Type(Type::MakeTextureType("itexture2D",
                                                     SpvDim2D,
                                                     /*isDepth=*/false,
                                                     /*isArrayedTexture=*/false,
                                                     /*isMultisampled=*/false,
                                                     /*isSampled=*/true))
            , fSampler1D_Type(Type::MakeSamplerType("sampler1D", *fTexture1D_Type))
            , fSampler2D_Type(Type::MakeSamplerType("sampler2D", *fTexture2D_Type))
            , fSampler3D_Type(Type::MakeSamplerType("sampler3D", *fTexture3D_Type))
            , fSamplerExternalOES_Type(
                      Type::MakeSamplerType("samplerExternalOES", *fTextureExternalOES_Type))
            , fSamplerCube_Type(Type::MakeSamplerType("samplerCube", *fTextureCube_Type))
            , fSampler2DRect_Type(Type::MakeSamplerType("sampler2DRect", *fTexture2DRect_Type))
            , fSampler1DArray_Type(Type::MakeOtherType("sampler1DArray"))
            , fSampler2DArray_Type(Type::MakeOtherType("sampler2DArray"))
            , fSamplerCubeArray_Type(Type::MakeOtherType("samplerCubeArray"))
            , fSamplerBuffer_Type(Type::MakeSamplerType("samplerBuffer", *fTextureBuffer_Type))
            , fSampler2DMS_Type(Type::MakeOtherType("sampler2DMS"))
            , fSampler2DMSArray_Type(Type::MakeOtherType("sampler2DMSArray"))
            , fSampler1DShadow_Type(Type::MakeOtherType("sampler1DShadow"))
            , fSampler2DShadow_Type(Type::MakeOtherType("sampler2DShadow"))
            , fSamplerCubeShadow_Type(Type::MakeOtherType("samplerCubeShadow"))
            , fSampler2DRectShadow_Type(Type::MakeOtherType("sampler2DRectShadow"))
            , fSampler1DArrayShadow_Type(Type::MakeOtherType("sampler1DArrayShadow"))
            , fSampler2DArrayShadow_Type(Type::MakeOtherType("sampler2DArrayShadow"))
            , fSamplerCubeArrayShadow_Type(Type::MakeOtherType("samplerCubeArrayShadow"))

            // Related to below FIXME, gsampler*s don't currently expand to cover integer case.
            , fISampler2D_Type(Type::MakeSamplerType("isampler2D", *fITexture2D_Type))

            , fSampler_Type(Type::MakeSimpleType("sampler", Type::TypeKind::kSeparateSampler))
            // FIXME express these as "gimage2D" that expand to image2D, iimage2D, and uimage2D.
            , fImage2D_Type(Type::MakeTextureType("image2D",
                                                  SpvDim2D,
                                                  /*isDepth=*/false,
                                                  /*isArrayedTexture=*/false,
                                                  /*isMultisampled=*/false,
                                                  /*isSampled=*/true))
            , fIImage2D_Type(Type::MakeTextureType("iimage2D",
                                                   SpvDim2D,
                                                   /*isDepth=*/false,
                                                   /*isArrayedTexture=*/false,
                                                   /*isMultisampled=*/false,
                                                   /*isSampled=*/true))

            // FIXME express these as "gsubpassInput" that expand to subpassInput, isubpassInput,
            // and usubpassInput.
            , fSubpassInput_Type(Type::MakeTextureType("subpassInput",
                                                       SpvDimSubpassData,
                                                       /*isDepth=*/false,
                                                       /*isArrayedTexture=*/false,
                                                       /*isMultisampled=*/false,
                                                       /*isSampled=*/false))
            , fSubpassInputMS_Type(Type::MakeTextureType("subpassInputMS",
                                                         SpvDimSubpassData,
                                                         /*isDepth=*/false,
                                                         /*isArrayedTexture=*/false,
                                                         /*isMultisampled=*/true,
                                                         /*isSampled=*/false))

            // FIXME figure out what we're supposed to do with the gsampler et al. types)
            , fGSampler1D_Type(Type::MakeGenericType("$gsampler1D", static_type(*fSampler1D_Type)))
            , fGSampler2D_Type(Type::MakeGenericType("$gsampler2D", static_type(*fSampler2D_Type)))
            , fGSampler3D_Type(Type::MakeGenericType("$gsampler3D", static_type(*fSampler3D_Type)))
            , fGSamplerCube_Type(
                      Type::MakeGenericType("$gsamplerCube", static_type(*fSamplerCube_Type)))
            , fGSampler2DRect_Type(
                      Type::MakeGenericType("$gsampler2DRect", static_type(*fSampler2DRect_Type)))
            , fGSampler1DArray_Type(
                      Type::MakeGenericType("$gsampler1DArray", static_type(*fSampler1DArray_Type)))
            , fGSampler2DArray_Type(
                      Type::MakeGenericType("$gsampler2DArray", static_type(*fSampler2DArray_Type)))
            , fGSamplerCubeArray_Type(Type::MakeGenericType("$gsamplerCubeArray",
                                                            static_type(*fSamplerCubeArray_Type)))
            , fGSamplerBuffer_Type(
                      Type::MakeGenericType("$gsamplerBuffer", static_type(*fSamplerBuffer_Type)))
            , fGSampler2DMS_Type(
                      Type::MakeGenericType("$gsampler2DMS", static_type(*fSampler2DMS_Type)))
            , fGSampler2DMSArray_Type(Type::MakeGenericType("$gsampler2DMSArray",
                                                            static_type(*fSampler2DMSArray_Type)))
            , fGSampler2DArrayShadow_Type(Type::MakeGenericType(
                      "$gsampler2DArrayShadow", static_type(*fSampler2DArrayShadow_Type)))
            , fGSamplerCubeArrayShadow_Type(Type::MakeGenericType(
                      "$gsamplerCubeArrayShadow", static_type(*fSamplerCubeArrayShadow_Type)))
            , fGenType_Type(Type::MakeGenericType("$genType",
                                                  {fFloat_Type.get(), fFloat2_Type.get(),
                                                   fFloat3_Type.get(), fFloat4_Type.get()}))
            , fGenHType_Type(Type::MakeGenericType(
                      "$genHType",
                      {fHalf_Type.get(), fHalf2_Type.get(), fHalf3_Type.get(), fHalf4_Type.get()}))
            , fGenIType_Type(Type::MakeGenericType(
                      "$genIType",
                      {fInt_Type.get(), fInt2_Type.get(), fInt3_Type.get(), fInt4_Type.get()}))
            , fGenUType_Type(Type::MakeGenericType(
                      "$genUType",
                      {fUInt_Type.get(), fUInt2_Type.get(), fUInt3_Type.get(), fUInt4_Type.get()}))
            , fGenBType_Type(Type::MakeGenericType(
                      "$genBType",
                      {fBool_Type.get(), fBool2_Type.get(), fBool3_Type.get(), fBool4_Type.get()}))
            , fMat_Type(Type::MakeGenericType(
                      "$mat",
                      {fFloat2x2_Type.get(), fFloat2x3_Type.get(), fFloat2x4_Type.get(),
                       fFloat3x2_Type.get(), fFloat3x3_Type.get(), fFloat3x4_Type.get(),
                       fFloat4x2_Type.get(), fFloat4x3_Type.get(), fFloat4x4_Type.get()}))
            , fMatH_Type(Type::MakeGenericType(
                      "$matH",
                      {fHalf2x2_Type.get(), fHalf2x3_Type.get(), fHalf2x4_Type.get(),
                       fHalf3x2_Type.get(), fHalf3x3_Type.get(), fHalf3x4_Type.get(),
                       fHalf4x2_Type.get(), fHalf4x3_Type.get(), fHalf4x4_Type.get()}))
            , fVec_Type(Type::MakeGenericType("$vec",
                                              {fInvalid_Type.get(), fFloat2_Type.get(),
                                               fFloat3_Type.get(), fFloat4_Type.get()}))
            , fGVec_Type(Type::MakeOtherType("$gvec"))
            , fGVec2_Type(Type::MakeOtherType("$gfloat2"))
            , fGVec3_Type(Type::MakeOtherType("$gfloat3"))
            , fGVec4_Type(Type::MakeGenericType("$gfloat4", static_type(*fFloat4_Type)))
            , fHVec_Type(Type::MakeGenericType("$hvec",
                                               {fInvalid_Type.get(), fHalf2_Type.get(),
                                                fHalf3_Type.get(), fHalf4_Type.get()}))
            , fIVec_Type(Type::MakeGenericType(
                      "$ivec",
                      {fInvalid_Type.get(), fInt2_Type.get(), fInt3_Type.get(), fInt4_Type.get()}))
            , fUVec_Type(Type::MakeGenericType("$uvec",
                                               {fInvalid_Type.get(), fUInt2_Type.get(),
                                                fUInt3_Type.get(), fUInt4_Type.get()}))
            , fSVec_Type(Type::MakeGenericType("$svec",
                                               {fInvalid_Type.get(), fShort2_Type.get(),
                                                fShort3_Type.get(), fShort4_Type.get()}))
            , fUSVec_Type(Type::MakeGenericType("$usvec",
                                                {fInvalid_Type.get(), fUShort2_Type.get(),
                                                 fUShort3_Type.get(), fUShort4_Type.get()}))
            , fByteVec_Type(Type::MakeGenericType("$bytevec",
                                                  {fInvalid_Type.get(), fByte2_Type.get(),
                                                   fByte3_Type.get(), fByte4_Type.get()}))
            , fUByteVec_Type(Type::MakeGenericType("$ubytevec",
                                                   {fInvalid_Type.get(), fUByte2_Type.get(),
                                                    fUByte3_Type.get(), fUByte4_Type.get()}))
            , fBVec_Type(Type::MakeGenericType("$bvec",
                                               {fInvalid_Type.get(), fBool2_Type.get(),
                                                fBool3_Type.get(), fBool4_Type.get()}))
            , fSkCaps_Type(Type::MakeOtherType("$sk_Caps"))
            , fFragmentProcessor_Type(fp_type(fInt_Type.get(), fBool_Type.get()))
            , fDefined_Expression(new Defined(fInvalid_Type.get())) {}

    static std::vector<const Type*> static_type(const Type& t) {
        return { &t, &t, &t, &t };
    }

    const std::unique_ptr<Type> fInvalid_Type;
    const std::unique_ptr<Type> fVoid_Type;
    const std::unique_ptr<Type> fNull_Type;
    const std::unique_ptr<Type> fFloatLiteral_Type;
    const std::unique_ptr<Type> fIntLiteral_Type;

    const std::unique_ptr<Type> fFloat_Type;
    const std::unique_ptr<Type> fFloat2_Type;
    const std::unique_ptr<Type> fFloat3_Type;
    const std::unique_ptr<Type> fFloat4_Type;

    const std::unique_ptr<Type> fHalf_Type;
    const std::unique_ptr<Type> fHalf2_Type;
    const std::unique_ptr<Type> fHalf3_Type;
    const std::unique_ptr<Type> fHalf4_Type;

    const std::unique_ptr<Type> fInt_Type;
    const std::unique_ptr<Type> fInt2_Type;
    const std::unique_ptr<Type> fInt3_Type;
    const std::unique_ptr<Type> fInt4_Type;

    const std::unique_ptr<Type> fUInt_Type;
    const std::unique_ptr<Type> fUInt2_Type;
    const std::unique_ptr<Type> fUInt3_Type;
    const std::unique_ptr<Type> fUInt4_Type;

    const std::unique_ptr<Type> fShort_Type;
    const std::unique_ptr<Type> fShort2_Type;
    const std::unique_ptr<Type> fShort3_Type;
    const std::unique_ptr<Type> fShort4_Type;

    const std::unique_ptr<Type> fUShort_Type;
    const std::unique_ptr<Type> fUShort2_Type;
    const std::unique_ptr<Type> fUShort3_Type;
    const std::unique_ptr<Type> fUShort4_Type;

    const std::unique_ptr<Type> fByte_Type;
    const std::unique_ptr<Type> fByte2_Type;
    const std::unique_ptr<Type> fByte3_Type;
    const std::unique_ptr<Type> fByte4_Type;

    const std::unique_ptr<Type> fUByte_Type;
    const std::unique_ptr<Type> fUByte2_Type;
    const std::unique_ptr<Type> fUByte3_Type;
    const std::unique_ptr<Type> fUByte4_Type;

    const std::unique_ptr<Type> fBool_Type;
    const std::unique_ptr<Type> fBool2_Type;
    const std::unique_ptr<Type> fBool3_Type;
    const std::unique_ptr<Type> fBool4_Type;

    const std::unique_ptr<Type> fFloat2x2_Type;
    const std::unique_ptr<Type> fFloat2x3_Type;
    const std::unique_ptr<Type> fFloat2x4_Type;
    const std::unique_ptr<Type> fFloat3x2_Type;
    const std::unique_ptr<Type> fFloat3x3_Type;
    const std::unique_ptr<Type> fFloat3x4_Type;
    const std::unique_ptr<Type> fFloat4x2_Type;
    const std::unique_ptr<Type> fFloat4x3_Type;
    const std::unique_ptr<Type> fFloat4x4_Type;

    const std::unique_ptr<Type> fHalf2x2_Type;
    const std::unique_ptr<Type> fHalf2x3_Type;
    const std::unique_ptr<Type> fHalf2x4_Type;
    const std::unique_ptr<Type> fHalf3x2_Type;
    const std::unique_ptr<Type> fHalf3x3_Type;
    const std::unique_ptr<Type> fHalf3x4_Type;
    const std::unique_ptr<Type> fHalf4x2_Type;
    const std::unique_ptr<Type> fHalf4x3_Type;
    const std::unique_ptr<Type> fHalf4x4_Type;

    const std::unique_ptr<Type> fTexture1D_Type;
    const std::unique_ptr<Type> fTexture2D_Type;
    const std::unique_ptr<Type> fTexture3D_Type;
    const std::unique_ptr<Type> fTextureExternalOES_Type;
    const std::unique_ptr<Type> fTextureCube_Type;
    const std::unique_ptr<Type> fTexture2DRect_Type;
    const std::unique_ptr<Type> fTextureBuffer_Type;
    const std::unique_ptr<Type> fITexture2D_Type;

    const std::unique_ptr<Type> fSampler1D_Type;
    const std::unique_ptr<Type> fSampler2D_Type;
    const std::unique_ptr<Type> fSampler3D_Type;
    const std::unique_ptr<Type> fSamplerExternalOES_Type;
    const std::unique_ptr<Type> fSamplerCube_Type;
    const std::unique_ptr<Type> fSampler2DRect_Type;
    const std::unique_ptr<Type> fSampler1DArray_Type;
    const std::unique_ptr<Type> fSampler2DArray_Type;
    const std::unique_ptr<Type> fSamplerCubeArray_Type;
    const std::unique_ptr<Type> fSamplerBuffer_Type;
    const std::unique_ptr<Type> fSampler2DMS_Type;
    const std::unique_ptr<Type> fSampler2DMSArray_Type;
    const std::unique_ptr<Type> fSampler1DShadow_Type;
    const std::unique_ptr<Type> fSampler2DShadow_Type;
    const std::unique_ptr<Type> fSamplerCubeShadow_Type;
    const std::unique_ptr<Type> fSampler2DRectShadow_Type;
    const std::unique_ptr<Type> fSampler1DArrayShadow_Type;
    const std::unique_ptr<Type> fSampler2DArrayShadow_Type;
    const std::unique_ptr<Type> fSamplerCubeArrayShadow_Type;

    const std::unique_ptr<Type> fISampler2D_Type;
    const std::unique_ptr<Type> fSampler_Type;

    const std::unique_ptr<Type> fImage2D_Type;
    const std::unique_ptr<Type> fIImage2D_Type;

    const std::unique_ptr<Type> fSubpassInput_Type;
    const std::unique_ptr<Type> fSubpassInputMS_Type;

    const std::unique_ptr<Type> fGSampler1D_Type;
    const std::unique_ptr<Type> fGSampler2D_Type;
    const std::unique_ptr<Type> fGSampler3D_Type;
    const std::unique_ptr<Type> fGSamplerCube_Type;
    const std::unique_ptr<Type> fGSampler2DRect_Type;
    const std::unique_ptr<Type> fGSampler1DArray_Type;
    const std::unique_ptr<Type> fGSampler2DArray_Type;
    const std::unique_ptr<Type> fGSamplerCubeArray_Type;
    const std::unique_ptr<Type> fGSamplerBuffer_Type;
    const std::unique_ptr<Type> fGSampler2DMS_Type;
    const std::unique_ptr<Type> fGSampler2DMSArray_Type;
    const std::unique_ptr<Type> fGSampler2DArrayShadow_Type;
    const std::unique_ptr<Type> fGSamplerCubeArrayShadow_Type;

    const std::unique_ptr<Type> fGenType_Type;
    const std::unique_ptr<Type> fGenHType_Type;
    const std::unique_ptr<Type> fGenIType_Type;
    const std::unique_ptr<Type> fGenUType_Type;
    const std::unique_ptr<Type> fGenBType_Type;

    const std::unique_ptr<Type> fMat_Type;
    const std::unique_ptr<Type> fMatH_Type;

    const std::unique_ptr<Type> fVec_Type;

    const std::unique_ptr<Type> fGVec_Type;
    const std::unique_ptr<Type> fGVec2_Type;
    const std::unique_ptr<Type> fGVec3_Type;
    const std::unique_ptr<Type> fGVec4_Type;
    const std::unique_ptr<Type> fHVec_Type;
    const std::unique_ptr<Type> fDVec_Type;
    const std::unique_ptr<Type> fIVec_Type;
    const std::unique_ptr<Type> fUVec_Type;
    const std::unique_ptr<Type> fSVec_Type;
    const std::unique_ptr<Type> fUSVec_Type;
    const std::unique_ptr<Type> fByteVec_Type;
    const std::unique_ptr<Type> fUByteVec_Type;

    const std::unique_ptr<Type> fBVec_Type;

    const std::unique_ptr<Type> fSkCaps_Type;
    const std::unique_ptr<Type> fFragmentProcessor_Type;

    // sentinel expression used to mark that a variable has a value during dataflow analysis (when
    // it could have several different values, or the analyzer is otherwise unable to assign it a
    // specific expression)
    const std::unique_ptr<Expression> fDefined_Expression;

private:
    class Defined final : public Expression {
    public:
        static constexpr Kind kExpressionKind = Kind::kDefined;

        Defined(const Type* type)
        : INHERITED(-1, kExpressionKind, type) {}

        bool hasProperty(Property property) const override {
            return false;
        }

        String description() const override {
            return "<defined>";
        }

        std::unique_ptr<Expression> clone() const override {
            return std::unique_ptr<Expression>(new Defined(&this->type()));
        }

        using INHERITED = Expression;
    };

    static std::unique_ptr<Type> fp_type(const Type* intType, const Type* boolType) {
        // Build fields for FragmentProcessors, which should parallel the
        // C++ API for GrFragmentProcessor.
        Modifiers mods(Layout(), Modifiers::kConst_Flag);
        std::vector<Type::Field> fields = {
            Type::Field(mods, "numTextureSamplers", intType),
            Type::Field(mods, "numChildProcessors", intType),
            Type::Field(mods, "usesLocalCoords", boolType),
            Type::Field(mods, "compatibleWithCoverageAsAlpha", boolType),
            Type::Field(mods, "preservesOpaqueInput", boolType),
            Type::Field(mods, "hasConstantOutputForConstantInput", boolType)
        };
        return Type::MakeOtherStruct("fragmentProcessor", std::move(fields));
    }
};

}  // namespace SkSL

#endif
