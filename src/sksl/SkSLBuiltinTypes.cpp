/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLArrayType.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteralType.h"
#include "src/sksl/ir/SkSLMatrixType.h"
#include "src/sksl/ir/SkSLSamplerType.h"
#include "src/sksl/ir/SkSLScalarType.h"
#include "src/sksl/ir/SkSLTextureType.h"
#include "src/sksl/ir/SkSLVectorType.h"
#include "src/sksl/spirv.h"

namespace SkSL {

/**
 * Create a generic type which maps to the listed types--e.g. $genType is a generic type which
 * can match float, float2, float3 or float4.
 */
std::unique_ptr<GenericType> BuiltinTypes::MakeGenericType(const char* name,
                                                           std::vector<const Type*> types) {
    return std::make_unique<GenericType>(name, std::move(types));
}

/**
 * Create a "special" type with the given name, abbreviation, and TypeKind.
 */
std::unique_ptr<Type> BuiltinTypes::MakeSpecialType(const char* name,
                                                    const char* abbrev,
                                                    Type::TypeKind typeKind) {
    return std::unique_ptr<Type>(new Type(name, abbrev, typeKind));
}

/**
 * Initializes the core SkSL types.
 */
BuiltinTypes::BuiltinTypes()
        : fFloat(std::make_unique<ScalarType>("float", "f", Type::NumberKind::kFloat,
                                              /*priority=*/10, /*bitWidth=*/32))
        , fFloat2(std::make_unique<VectorType>("float2", "f2", *fFloat, /*columns=*/2))
        , fFloat3(std::make_unique<VectorType>("float3", "f3", *fFloat, /*columns=*/3))
        , fFloat4(std::make_unique<VectorType>("float4", "f4", *fFloat, /*columns=*/4))
        , fHalf(std::make_unique<ScalarType>("half", "h", Type::NumberKind::kFloat,
                                             /*priority=*/9, /*bitWidth=*/16))
        , fHalf2(std::make_unique<VectorType>("half2", "h2", *fHalf, /*columns=*/2))
        , fHalf3(std::make_unique<VectorType>("half3", "h3", *fHalf, /*columns=*/3))
        , fHalf4(std::make_unique<VectorType>("half4", "h4", *fHalf, /*columns=*/4))
        , fInt(std::make_unique<ScalarType>("int", "i", Type::NumberKind::kSigned, /*priority=*/7,
                                            /*bitWidth=*/32))
        , fInt2(std::make_unique<VectorType>("int2", "i2", *fInt, /*columns=*/2))
        , fInt3(std::make_unique<VectorType>("int3", "i3", *fInt, /*columns=*/3))
        , fInt4(std::make_unique<VectorType>("int4", "i4", *fInt, /*columns=*/4))
        , fUInt(std::make_unique<ScalarType>("uint", "I", Type::NumberKind::kUnsigned,
                                             /*priority=*/6, /*bitWidth=*/32))
        , fUInt2(std::make_unique<VectorType>("uint2", "I2", *fUInt, /*columns=*/2))
        , fUInt3(std::make_unique<VectorType>("uint3", "I3", *fUInt, /*columns=*/3))
        , fUInt4(std::make_unique<VectorType>("uint4", "I4", *fUInt, /*columns=*/4))
        , fShort(std::make_unique<ScalarType>("short", "s", Type::NumberKind::kSigned,
                                              /*priority=*/4, /*bitWidth=*/16))
        , fShort2(std::make_unique<VectorType>("short2", "s2", *fShort, /*columns=*/2))
        , fShort3(std::make_unique<VectorType>("short3", "s3", *fShort, /*columns=*/3))
        , fShort4(std::make_unique<VectorType>("short4", "s4", *fShort, /*columns=*/4))
        , fUShort(std::make_unique<ScalarType>("ushort", "S", Type::NumberKind::kUnsigned,
                                               /*priority=*/3, /*bitWidth=*/16))
        , fUShort2(std::make_unique<VectorType>("ushort2", "S2", *fUShort, /*columns=*/2))
        , fUShort3(std::make_unique<VectorType>("ushort3", "S3", *fUShort, /*columns=*/3))
        , fUShort4(std::make_unique<VectorType>("ushort4", "S4", *fUShort, /*columns=*/4))
        , fBool(std::make_unique<ScalarType>("bool", "b", Type::NumberKind::kBoolean,
                                             /*priority=*/0, /*bitWidth=*/1))
        , fBool2(std::make_unique<VectorType>("bool2", "b2", *fBool, /*columns=*/2))
        , fBool3(std::make_unique<VectorType>("bool3", "b3", *fBool, /*columns=*/3))
        , fBool4(std::make_unique<VectorType>("bool4", "b4", *fBool, /*columns=*/4))
        , fInvalid(MakeSpecialType("<INVALID>", "O", Type::TypeKind::kOther))
        , fPoison(MakeSpecialType(Compiler::POISON_TAG, "P", Type::TypeKind::kOther))
        , fVoid(MakeSpecialType("void", "v", Type::TypeKind::kVoid))
        , fFloatLiteral(std::make_unique<LiteralType>("$floatLiteral", *fFloat, /*priority=*/8))
        , fIntLiteral(std::make_unique<LiteralType>("$intLiteral", *fInt, /*priority=*/5))
        , fFloat2x2(std::make_unique<MatrixType>("float2x2", "f22", *fFloat, /*columns=*/2,
                                                 /*rows=*/2))
        , fFloat2x3(std::make_unique<MatrixType>("float2x3", "f23", *fFloat, /*columns=*/2,
                                                 /*rows=*/3))
        , fFloat2x4(std::make_unique<MatrixType>("float2x4", "f24", *fFloat, /*columns=*/2,
                                                 /*rows=*/4))
        , fFloat3x2(std::make_unique<MatrixType>("float3x2", "f32", *fFloat, /*columns=*/3,
                                                 /*rows=*/2))
        , fFloat3x3(std::make_unique<MatrixType>("float3x3", "f33", *fFloat, /*columns=*/3,
                                                 /*rows=*/3))
        , fFloat3x4(std::make_unique<MatrixType>("float3x4", "f34", *fFloat, /*columns=*/3,
                                                 /*rows=*/4))
        , fFloat4x2(std::make_unique<MatrixType>("float4x2", "f42", *fFloat, /*columns=*/4,
                                                 /*rows=*/2))
        , fFloat4x3(std::make_unique<MatrixType>("float4x3", "f43", *fFloat, /*columns=*/4,
                                                 /*rows=*/3))
        , fFloat4x4(std::make_unique<MatrixType>("float4x4", "f44", *fFloat, /*columns=*/4,
                                                 /*rows=*/4))
        , fHalf2x2(std::make_unique<MatrixType>("half2x2", "h22", *fHalf, /*columns=*/2,
                                                /*rows=*/2))
        , fHalf2x3(std::make_unique<MatrixType>("half2x3", "h23", *fHalf, /*columns=*/2,
                                                /*rows=*/3))
        , fHalf2x4(std::make_unique<MatrixType>("half2x4", "h24", *fHalf, /*columns=*/2,
                                                /*rows=*/4))
        , fHalf3x2(std::make_unique<MatrixType>("half3x2", "h32", *fHalf, /*columns=*/3,
                                                /*rows=*/2))
        , fHalf3x3(std::make_unique<MatrixType>("half3x3", "h33", *fHalf, /*columns=*/3,
                                                /*rows=*/3))
        , fHalf3x4(std::make_unique<MatrixType>("half3x4", "h34", *fHalf, /*columns=*/3,
                                                /*rows=*/4))
        , fHalf4x2(std::make_unique<MatrixType>("half4x2", "h42", *fHalf, /*columns=*/4,
                                                /*rows=*/2))
        , fHalf4x3(std::make_unique<MatrixType>("half4x3", "h43", *fHalf, /*columns=*/4,
                                                /*rows=*/3))
        , fHalf4x4(std::make_unique<MatrixType>("half4x4", "h44", *fHalf, /*columns=*/4,
                                                /*rows=*/4))
        , fTexture1D(std::make_unique<TextureType>("texture1D",
                                                   SpvDim1D,
                                                   /*isDepth=*/false,
                                                   /*isArrayedTexture=*/false,
                                                   /*isMultisampled=*/false,
                                                   /*isSampled=*/true))
        , fTexture2D(std::make_unique<TextureType>("texture2D",
                                                   SpvDim2D,
                                                   /*isDepth=*/false,
                                                   /*isArrayedTexture=*/false,
                                                   /*isMultisampled=*/false,
                                                   /*isSampled=*/true))
        , fTexture3D(std::make_unique<TextureType>("texture3D",
                                                   SpvDim3D,
                                                   /*isDepth=*/false,
                                                   /*isArrayedTexture=*/false,
                                                   /*isMultisampled=*/false,
                                                   /*isSampled=*/true))
        , fTextureExternalOES(std::make_unique<TextureType>("textureExternalOES",
                                                            SpvDim2D,
                                                            /*isDepth=*/false,
                                                            /*isArrayedTexture=*/false,
                                                            /*isMultisampled=*/false,
                                                            /*isSampled=*/true))
        , fTextureCube(std::make_unique<TextureType>("textureCube",
                                                     SpvDimCube,
                                                     /*isDepth=*/false,
                                                     /*isArrayedTexture=*/false,
                                                     /*isMultisampled=*/false,
                                                     /*isSampled=*/true))
        , fTexture2DRect(std::make_unique<TextureType>("texture2DRect",
                                                       SpvDimRect,
                                                       /*isDepth=*/false,
                                                       /*isArrayedTexture=*/false,
                                                       /*isMultisampled=*/false,
                                                       /*isSampled=*/true))
        , fITexture2D(std::make_unique<TextureType>("itexture2D",
                                                    SpvDim2D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
        , fSampler1D(std::make_unique<SamplerType>("sampler1D", *fTexture1D))
        , fSampler2D(std::make_unique<SamplerType>("sampler2D", *fTexture2D))
        , fSampler3D(std::make_unique<SamplerType>("sampler3D", *fTexture3D))
        , fSamplerExternalOES(std::make_unique<SamplerType>("samplerExternalOES",
                                                            *fTextureExternalOES))
        , fSampler2DRect(std::make_unique<SamplerType>("sampler2DRect", *fTexture2DRect))

        , fISampler2D(std::make_unique<SamplerType>("isampler2D", *fITexture2D))

        , fSampler(MakeSpecialType("sampler", "ss", Type::TypeKind::kSeparateSampler))

        , fSubpassInput(std::make_unique<TextureType>("subpassInput",
                                                      SpvDimSubpassData,
                                                      /*isDepth=*/false,
                                                      /*isArrayedTexture=*/false,
                                                      /*isMultisampled=*/false,
                                                      /*isSampled=*/false))
        , fSubpassInputMS(std::make_unique<TextureType>("subpassInputMS",
                                                        SpvDimSubpassData,
                                                        /*isDepth=*/false,
                                                        /*isArrayedTexture=*/false,
                                                        /*isMultisampled=*/true,
                                                        /*isSampled=*/false))

        , fGenType(MakeGenericType("$genType",
                                   {fFloat.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fGenHType(MakeGenericType("$genHType",
                                    {fHalf.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fGenIType(MakeGenericType("$genIType",
                                    {fInt.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fGenUType(MakeGenericType("$genUType",
                                    {fUInt.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fGenBType(MakeGenericType("$genBType",
                                    {fBool.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fMat(MakeGenericType("$mat",
                               {fFloat2x2.get(), fFloat2x3.get(), fFloat2x4.get(), fFloat3x2.get(),
                                fFloat3x3.get(), fFloat3x4.get(), fFloat4x2.get(), fFloat4x3.get(),
                                fFloat4x4.get()}))
        , fHMat(MakeGenericType("$hmat",
                                {fHalf2x2.get(), fHalf2x3.get(), fHalf2x4.get(), fHalf3x2.get(),
                                 fHalf3x3.get(), fHalf3x4.get(), fHalf4x2.get(), fHalf4x3.get(),
                                 fHalf4x4.get()}))
        , fSquareMat(MakeGenericType("$squareMat",
                                     {fFloat2x2.get(), fFloat3x3.get(), fFloat4x4.get()}))
        , fSquareHMat(MakeGenericType("$squareHMat",
                                      {fHalf2x2.get(), fHalf3x3.get(),fHalf4x4.get()}))
        , fVec(MakeGenericType("$vec",
                               {fInvalid.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fHVec(MakeGenericType("$hvec",
                                {fInvalid.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fIVec(MakeGenericType("$ivec",
                                {fInvalid.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fUVec(MakeGenericType("$uvec",
                                {fInvalid.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fSVec(MakeGenericType("$svec",
                                {fInvalid.get(), fShort2.get(), fShort3.get(), fShort4.get()}))
        , fUSVec(MakeGenericType("$usvec",
                                 {fInvalid.get(), fUShort2.get(), fUShort3.get(), fUShort4.get()}))
        , fBVec(MakeGenericType("$bvec",
                                {fInvalid.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fSkCaps(MakeSpecialType("$sk_Caps", "O", Type::TypeKind::kOther))
        , fFragmentProcessor(MakeSpecialType("fragmentProcessor", "fp",
                                             Type::TypeKind::kFragmentProcessor))
        , fColorFilter(MakeSpecialType("colorFilter", "CF", Type::TypeKind::kColorFilter))
        , fShader(MakeSpecialType("shader", "SH", Type::TypeKind::kShader)) {}

}  // namespace SkSL
