/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/spirv.h"

namespace SkSL {

/**
 * Initializes the core SkSL types.
 */
BuiltinTypes::BuiltinTypes()
        : fFloat(Type::MakeScalarType(
                  "float", "f", Type::NumberKind::kFloat, /*priority=*/10, /*bitWidth=*/32))
        , fFloat2(Type::MakeVectorType("float2", "f2", *fFloat, /*columns=*/2))
        , fFloat3(Type::MakeVectorType("float3", "f3", *fFloat, /*columns=*/3))
        , fFloat4(Type::MakeVectorType("float4", "f4", *fFloat, /*columns=*/4))
        , fHalf(Type::MakeScalarType(
                  "half", "h", Type::NumberKind::kFloat, /*priority=*/9, /*bitWidth=*/16))
        , fHalf2(Type::MakeVectorType("half2", "h2", *fHalf, /*columns=*/2))
        , fHalf3(Type::MakeVectorType("half3", "h3", *fHalf, /*columns=*/3))
        , fHalf4(Type::MakeVectorType("half4", "h4", *fHalf, /*columns=*/4))
        , fInt(Type::MakeScalarType(
                  "int", "i", Type::NumberKind::kSigned, /*priority=*/7, /*bitWidth=*/32))
        , fInt2(Type::MakeVectorType("int2", "i2", *fInt, /*columns=*/2))
        , fInt3(Type::MakeVectorType("int3", "i3", *fInt, /*columns=*/3))
        , fInt4(Type::MakeVectorType("int4", "i4", *fInt, /*columns=*/4))
        , fUInt(Type::MakeScalarType(
                  "uint", "I", Type::NumberKind::kUnsigned, /*priority=*/6, /*bitWidth=*/32))
        , fUInt2(Type::MakeVectorType("uint2", "I2", *fUInt, /*columns=*/2))
        , fUInt3(Type::MakeVectorType("uint3", "I3", *fUInt, /*columns=*/3))
        , fUInt4(Type::MakeVectorType("uint4", "I4", *fUInt, /*columns=*/4))
        , fShort(Type::MakeScalarType(
                  "short", "s", Type::NumberKind::kSigned, /*priority=*/4, /*bitWidth=*/16))
        , fShort2(Type::MakeVectorType("short2", "s2", *fShort, /*columns=*/2))
        , fShort3(Type::MakeVectorType("short3", "s3", *fShort, /*columns=*/3))
        , fShort4(Type::MakeVectorType("short4", "s4", *fShort, /*columns=*/4))
        , fUShort(Type::MakeScalarType(
                  "ushort", "S", Type::NumberKind::kUnsigned, /*priority=*/3, /*bitWidth=*/16))
        , fUShort2(Type::MakeVectorType("ushort2", "S2", *fUShort, /*columns=*/2))
        , fUShort3(Type::MakeVectorType("ushort3", "S3", *fUShort, /*columns=*/3))
        , fUShort4(Type::MakeVectorType("ushort4", "S4", *fUShort, /*columns=*/4))
        , fBool(Type::MakeScalarType(
                  "bool", "b", Type::NumberKind::kBoolean, /*priority=*/0, /*bitWidth=*/1))
        , fBool2(Type::MakeVectorType("bool2", "b2", *fBool, /*columns=*/2))
        , fBool3(Type::MakeVectorType("bool3", "b3", *fBool, /*columns=*/3))
        , fBool4(Type::MakeVectorType("bool4", "b4", *fBool, /*columns=*/4))
        , fInvalid(Type::MakeSpecialType("<INVALID>", "O", Type::TypeKind::kOther))
        , fPoison(Type::MakeSpecialType(Compiler::POISON_TAG, "P", Type::TypeKind::kOther))
        , fVoid(Type::MakeSpecialType("void", "v", Type::TypeKind::kVoid))
        , fFloatLiteral(Type::MakeLiteralType("$floatLiteral", *fFloat, /*priority=*/8))
        , fIntLiteral(Type::MakeLiteralType("$intLiteral", *fInt, /*priority=*/5))
        , fFloat2x2(Type::MakeMatrixType("float2x2", "f22", *fFloat, /*columns=*/2, /*rows=*/2))
        , fFloat2x3(Type::MakeMatrixType("float2x3", "f23", *fFloat, /*columns=*/2, /*rows=*/3))
        , fFloat2x4(Type::MakeMatrixType("float2x4", "f24", *fFloat, /*columns=*/2, /*rows=*/4))
        , fFloat3x2(Type::MakeMatrixType("float3x2", "f32", *fFloat, /*columns=*/3, /*rows=*/2))
        , fFloat3x3(Type::MakeMatrixType("float3x3", "f33", *fFloat, /*columns=*/3, /*rows=*/3))
        , fFloat3x4(Type::MakeMatrixType("float3x4", "f34", *fFloat, /*columns=*/3, /*rows=*/4))
        , fFloat4x2(Type::MakeMatrixType("float4x2", "f42", *fFloat, /*columns=*/4, /*rows=*/2))
        , fFloat4x3(Type::MakeMatrixType("float4x3", "f43", *fFloat, /*columns=*/4, /*rows=*/3))
        , fFloat4x4(Type::MakeMatrixType("float4x4", "f44", *fFloat, /*columns=*/4, /*rows=*/4))
        , fHalf2x2(Type::MakeMatrixType("half2x2", "h22", *fHalf, /*columns=*/2, /*rows=*/2))
        , fHalf2x3(Type::MakeMatrixType("half2x3", "h23", *fHalf, /*columns=*/2, /*rows=*/3))
        , fHalf2x4(Type::MakeMatrixType("half2x4", "h24", *fHalf, /*columns=*/2, /*rows=*/4))
        , fHalf3x2(Type::MakeMatrixType("half3x2", "h32", *fHalf, /*columns=*/3, /*rows=*/2))
        , fHalf3x3(Type::MakeMatrixType("half3x3", "h33", *fHalf, /*columns=*/3, /*rows=*/3))
        , fHalf3x4(Type::MakeMatrixType("half3x4", "h34", *fHalf, /*columns=*/3, /*rows=*/4))
        , fHalf4x2(Type::MakeMatrixType("half4x2", "h42", *fHalf, /*columns=*/4, /*rows=*/2))
        , fHalf4x3(Type::MakeMatrixType("half4x3", "h43", *fHalf, /*columns=*/4, /*rows=*/3))
        , fHalf4x4(Type::MakeMatrixType("half4x4", "h44", *fHalf, /*columns=*/4, /*rows=*/4))
        , fTexture1D(Type::MakeTextureType("texture1D",
                                           SpvDim1D,
                                           /*isDepth=*/false,
                                           /*isArrayedTexture=*/false,
                                           /*isMultisampled=*/false,
                                           /*isSampled=*/true))
        , fTexture2D(Type::MakeTextureType("texture2D",
                                           SpvDim2D,
                                           /*isDepth=*/false,
                                           /*isArrayedTexture=*/false,
                                           /*isMultisampled=*/false,
                                           /*isSampled=*/true))
        , fTexture3D(Type::MakeTextureType("texture3D",
                                           SpvDim3D,
                                           /*isDepth=*/false,
                                           /*isArrayedTexture=*/false,
                                           /*isMultisampled=*/false,
                                           /*isSampled=*/true))
        , fTextureExternalOES(Type::MakeTextureType("textureExternalOES",
                                                    SpvDim2D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
        , fTexture2DRect(Type::MakeTextureType("texture2DRect",
                                               SpvDimRect,
                                               /*isDepth=*/false,
                                               /*isArrayedTexture=*/false,
                                               /*isMultisampled=*/false,
                                               /*isSampled=*/true))
        , fITexture2D(Type::MakeTextureType("itexture2D",
                                            SpvDim2D,
                                            /*isDepth=*/false,
                                            /*isArrayedTexture=*/false,
                                            /*isMultisampled=*/false,
                                            /*isSampled=*/true))
        , fSampler1D(Type::MakeSamplerType("sampler1D", *fTexture1D))
        , fSampler2D(Type::MakeSamplerType("sampler2D", *fTexture2D))
        , fSampler3D(Type::MakeSamplerType("sampler3D", *fTexture3D))
        , fSamplerExternalOES(Type::MakeSamplerType("samplerExternalOES", *fTextureExternalOES))
        , fSampler2DRect(Type::MakeSamplerType("sampler2DRect", *fTexture2DRect))

        , fISampler2D(Type::MakeSamplerType("isampler2D", *fITexture2D))

        , fSampler(Type::MakeSpecialType("sampler", "ss", Type::TypeKind::kSeparateSampler))

        , fSubpassInput(Type::MakeTextureType("subpassInput",
                                              SpvDimSubpassData,
                                              /*isDepth=*/false,
                                              /*isArrayedTexture=*/false,
                                              /*isMultisampled=*/false,
                                              /*isSampled=*/false))
        , fSubpassInputMS(Type::MakeTextureType("subpassInputMS",
                                                SpvDimSubpassData,
                                                /*isDepth=*/false,
                                                /*isArrayedTexture=*/false,
                                                /*isMultisampled=*/true,
                                                /*isSampled=*/false))

        , fGenType(Type::MakeGenericType("$genType", {fFloat.get(), fFloat2.get(), fFloat3.get(),
                                                      fFloat4.get()}))
        , fGenHType(Type::MakeGenericType("$genHType", {fHalf.get(), fHalf2.get(), fHalf3.get(),
                                                        fHalf4.get()}))
        , fGenIType(Type::MakeGenericType("$genIType", {fInt.get(), fInt2.get(), fInt3.get(),
                                                        fInt4.get()}))
        , fGenUType(Type::MakeGenericType("$genUType", {fUInt.get(), fUInt2.get(), fUInt3.get(),
                                                        fUInt4.get()}))
        , fGenBType(Type::MakeGenericType("$genBType", {fBool.get(), fBool2.get(), fBool3.get(),
                                                        fBool4.get()}))
        , fMat(Type::MakeGenericType("$mat", {fFloat2x2.get(), fFloat2x3.get(), fFloat2x4.get(),
                                              fFloat3x2.get(), fFloat3x3.get(), fFloat3x4.get(),
                                              fFloat4x2.get(), fFloat4x3.get(), fFloat4x4.get()}))
        , fHMat(Type::MakeGenericType(
                  "$hmat",
                  {fHalf2x2.get(), fHalf2x3.get(), fHalf2x4.get(), fHalf3x2.get(), fHalf3x3.get(),
                   fHalf3x4.get(), fHalf4x2.get(), fHalf4x3.get(), fHalf4x4.get()}))
        , fSquareMat(Type::MakeGenericType("$squareMat", {fInvalid.get(), fFloat2x2.get(),
                                                          fFloat3x3.get(), fFloat4x4.get()}))
        , fSquareHMat(Type::MakeGenericType("$squareHMat", {fInvalid.get(), fHalf2x2.get(),
                                                            fHalf3x3.get(), fHalf4x4.get()}))
        , fVec(Type::MakeGenericType("$vec", {fInvalid.get(), fFloat2.get(), fFloat3.get(),
                                              fFloat4.get()}))
        , fHVec(Type::MakeGenericType("$hvec", {fInvalid.get(), fHalf2.get(), fHalf3.get(),
                                                fHalf4.get()}))
        , fIVec(Type::MakeGenericType("$ivec", {fInvalid.get(), fInt2.get(), fInt3.get(),
                                                fInt4.get()}))
        , fUVec(Type::MakeGenericType("$uvec", {fInvalid.get(), fUInt2.get(), fUInt3.get(),
                                                fUInt4.get()}))
        , fSVec(Type::MakeGenericType("$svec", {fInvalid.get(), fShort2.get(), fShort3.get(),
                                                fShort4.get()}))
        , fUSVec(Type::MakeGenericType("$usvec", {fInvalid.get(), fUShort2.get(), fUShort3.get(),
                                                  fUShort4.get()}))
        , fBVec(Type::MakeGenericType("$bvec", {fInvalid.get(), fBool2.get(), fBool3.get(),
                                                fBool4.get()}))
        , fSkCaps(Type::MakeSpecialType("$sk_Caps", "O", Type::TypeKind::kOther))
        , fColorFilter(Type::MakeSpecialType("colorFilter", "CF", Type::TypeKind::kColorFilter))
        , fShader(Type::MakeSpecialType("shader", "SH", Type::TypeKind::kShader))
        , fBlender(Type::MakeSpecialType("blender", "B", Type::TypeKind::kBlender)) {}

}  // namespace SkSL
