/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/spirv.h"

namespace SkSL {

/** Create a scalar type. */
std::unique_ptr<Type> BuiltinTypes::MakeScalarType(const char* name,
                                                   Type::NumberKind numberKind,
                                                   int priority,
                                                   bool highPrecision) {
    return std::unique_ptr<Type>(new Type(name, numberKind, priority, highPrecision));
}

/** Create a type for literal scalars. */
std::unique_ptr<Type> BuiltinTypes::MakeLiteralType(const char* name,
                                                    const Type& scalarType,
                                                    int priority) {
    return std::unique_ptr<Type>(new Type(name, scalarType, priority));
}

/** Create a vector type. */
std::unique_ptr<Type> BuiltinTypes::MakeVectorType(const char* name,
                                                   const Type& componentType,
                                                   int columns) {
    return std::unique_ptr<Type>(new Type(name, Type::TypeKind::kVector, componentType, columns));
}

/**
 * Create a generic type which maps to the listed types--e.g. $genType is a generic type which
 * can match float, float2, float3 or float4.
 */
std::unique_ptr<Type> BuiltinTypes::MakeGenericType(const char* name,
                                                    std::vector<const Type*> types) {
    return std::unique_ptr<Type>(new Type(name, std::move(types)));
}

/** Create a matrix type. */
std::unique_ptr<Type> BuiltinTypes::MakeMatrixType(const char* name,
                                                   const Type& componentType,
                                                   int columns,
                                                   int rows) {
    return std::unique_ptr<Type>(new Type(name, componentType, columns, rows));
}

/** Create a texture type. */
std::unique_ptr<Type> BuiltinTypes::MakeTextureType(const char* name,
                                                    SpvDim_ dimensions,
                                                    bool isDepth,
                                                    bool isArrayedTexture,
                                                    bool isMultisampled,
                                                    bool isSampled) {
    return std::unique_ptr<Type>(
            new Type(name, dimensions, isDepth, isArrayedTexture, isMultisampled, isSampled));
}

/** Create a sampler type. */
std::unique_ptr<Type> BuiltinTypes::MakeSamplerType(const char* name, const Type& textureType) {
    return std::unique_ptr<Type>(new Type(name, textureType));
}

/** Create a separate-sampler type. */
std::unique_ptr<Type> BuiltinTypes::MakeSeparateSamplerType(const char* name) {
    return std::unique_ptr<Type>(new Type(name, Type::TypeKind::kSeparateSampler));
}

/** Create a void type. */
std::unique_ptr<Type> BuiltinTypes::MakeVoidType(const char* name) {
    return std::unique_ptr<Type>(new Type(name, Type::TypeKind::kVoid));
}

/**
 * Create an "other" (special) type with the given name. These types cannot be directly
 * referenced from user code.
 */
std::unique_ptr<Type> BuiltinTypes::MakeOtherType(const char* name) {
    return std::unique_ptr<Type>(new Type(name));
}

/**
 * Initializes the core SkSL types.
 */
BuiltinTypes::BuiltinTypes()
        : fFloat(MakeScalarType(
                  "float", Type::NumberKind::kFloat, /*priority=*/10, /*highPrecision=*/true))
        , fFloat2(MakeVectorType("float2", *fFloat, /*columns=*/2))
        , fFloat3(MakeVectorType("float3", *fFloat, /*columns=*/3))
        , fFloat4(MakeVectorType("float4", *fFloat, /*columns=*/4))
        , fHalf(MakeScalarType("half", Type::NumberKind::kFloat, /*priority=*/9))
        , fHalf2(MakeVectorType("half2", *fHalf, /*columns=*/2))
        , fHalf3(MakeVectorType("half3", *fHalf, /*columns=*/3))
        , fHalf4(MakeVectorType("half4", *fHalf, /*columns=*/4))
        , fInt(MakeScalarType(
                  "int", Type::NumberKind::kSigned, /*priority=*/7, /*highPrecision=*/true))
        , fInt2(MakeVectorType("int2", *fInt, /*columns=*/2))
        , fInt3(MakeVectorType("int3", *fInt, /*columns=*/3))
        , fInt4(MakeVectorType("int4", *fInt, /*columns=*/4))
        , fUInt(MakeScalarType(
                  "uint", Type::NumberKind::kUnsigned, /*priority=*/6, /*highPrecision=*/true))
        , fUInt2(MakeVectorType("uint2", *fUInt, /*columns=*/2))
        , fUInt3(MakeVectorType("uint3", *fUInt, /*columns=*/3))
        , fUInt4(MakeVectorType("uint4", *fUInt, /*columns=*/4))
        , fShort(MakeScalarType("short", Type::NumberKind::kSigned, /*priority=*/4))
        , fShort2(MakeVectorType("short2", *fShort, /*columns=*/2))
        , fShort3(MakeVectorType("short3", *fShort, /*columns=*/3))
        , fShort4(MakeVectorType("short4", *fShort, /*columns=*/4))
        , fUShort(MakeScalarType("ushort", Type::NumberKind::kUnsigned, /*priority=*/3))
        , fUShort2(MakeVectorType("ushort2", *fUShort, /*columns=*/2))
        , fUShort3(MakeVectorType("ushort3", *fUShort, /*columns=*/3))
        , fUShort4(MakeVectorType("ushort4", *fUShort, /*columns=*/4))
        , fByte(MakeScalarType("byte", Type::NumberKind::kSigned, /*priority=*/2))
        , fByte2(MakeVectorType("byte2", *fByte, /*columns=*/2))
        , fByte3(MakeVectorType("byte3", *fByte, /*columns=*/3))
        , fByte4(MakeVectorType("byte4", *fByte, /*columns=*/4))
        , fUByte(MakeScalarType("ubyte", Type::NumberKind::kUnsigned, /*priority=*/1))
        , fUByte2(MakeVectorType("ubyte2", *fUByte, /*columns=*/2))
        , fUByte3(MakeVectorType("ubyte3", *fUByte, /*columns=*/3))
        , fUByte4(MakeVectorType("ubyte4", *fUByte, /*columns=*/4))
        , fBool(MakeScalarType("bool", Type::NumberKind::kBoolean, /*priority=*/0))
        , fBool2(MakeVectorType("bool2", *fBool, /*columns=*/2))
        , fBool3(MakeVectorType("bool3", *fBool, /*columns=*/3))
        , fBool4(MakeVectorType("bool4", *fBool, /*columns=*/4))
        , fInvalid(MakeOtherType("<INVALID>"))
        , fVoid(MakeVoidType("void"))
        , fFloatLiteral(MakeLiteralType("$floatLiteral", *fFloat, /*priority=*/8))
        , fIntLiteral(MakeLiteralType("$intLiteral", *fInt, /*priority=*/5))
        , fFloat2x2(MakeMatrixType("float2x2", *fFloat, /*columns=*/2, /*rows=*/2))
        , fFloat2x3(MakeMatrixType("float2x3", *fFloat, /*columns=*/2, /*rows=*/3))
        , fFloat2x4(MakeMatrixType("float2x4", *fFloat, /*columns=*/2, /*rows=*/4))
        , fFloat3x2(MakeMatrixType("float3x2", *fFloat, /*columns=*/3, /*rows=*/2))
        , fFloat3x3(MakeMatrixType("float3x3", *fFloat, /*columns=*/3, /*rows=*/3))
        , fFloat3x4(MakeMatrixType("float3x4", *fFloat, /*columns=*/3, /*rows=*/4))
        , fFloat4x2(MakeMatrixType("float4x2", *fFloat, /*columns=*/4, /*rows=*/2))
        , fFloat4x3(MakeMatrixType("float4x3", *fFloat, /*columns=*/4, /*rows=*/3))
        , fFloat4x4(MakeMatrixType("float4x4", *fFloat, /*columns=*/4, /*rows=*/4))
        , fHalf2x2(MakeMatrixType("half2x2", *fHalf, /*columns=*/2, /*rows=*/2))
        , fHalf2x3(MakeMatrixType("half2x3", *fHalf, /*columns=*/2, /*rows=*/3))
        , fHalf2x4(MakeMatrixType("half2x4", *fHalf, /*columns=*/2, /*rows=*/4))
        , fHalf3x2(MakeMatrixType("half3x2", *fHalf, /*columns=*/3, /*rows=*/2))
        , fHalf3x3(MakeMatrixType("half3x3", *fHalf, /*columns=*/3, /*rows=*/3))
        , fHalf3x4(MakeMatrixType("half3x4", *fHalf, /*columns=*/3, /*rows=*/4))
        , fHalf4x2(MakeMatrixType("half4x2", *fHalf, /*columns=*/4, /*rows=*/2))
        , fHalf4x3(MakeMatrixType("half4x3", *fHalf, /*columns=*/4, /*rows=*/3))
        , fHalf4x4(MakeMatrixType("half4x4", *fHalf, /*columns=*/4, /*rows=*/4))
        , fTexture1D(MakeTextureType("texture1D",
                                     SpvDim1D,
                                     /*isDepth=*/false,
                                     /*isArrayedTexture=*/false,
                                     /*isMultisampled=*/false,
                                     /*isSampled=*/true))
        , fTexture2D(MakeTextureType("texture2D",
                                     SpvDim2D,
                                     /*isDepth=*/false,
                                     /*isArrayedTexture=*/false,
                                     /*isMultisampled=*/false,
                                     /*isSampled=*/true))
        , fTexture3D(MakeTextureType("texture3D",
                                     SpvDim3D,
                                     /*isDepth=*/false,
                                     /*isArrayedTexture=*/false,
                                     /*isMultisampled=*/false,
                                     /*isSampled=*/true))
        , fTextureExternalOES(MakeTextureType("textureExternalOES",
                                              SpvDim2D,
                                              /*isDepth=*/false,
                                              /*isArrayedTexture=*/false,
                                              /*isMultisampled=*/false,
                                              /*isSampled=*/true))
        , fTextureCube(MakeTextureType("textureCube",
                                       SpvDimCube,
                                       /*isDepth=*/false,
                                       /*isArrayedTexture=*/false,
                                       /*isMultisampled=*/false,
                                       /*isSampled=*/true))
        , fTexture2DRect(MakeTextureType("texture2DRect",
                                         SpvDimRect,
                                         /*isDepth=*/false,
                                         /*isArrayedTexture=*/false,
                                         /*isMultisampled=*/false,
                                         /*isSampled=*/true))
        , fITexture2D(MakeTextureType("itexture2D",
                                      SpvDim2D,
                                      /*isDepth=*/false,
                                      /*isArrayedTexture=*/false,
                                      /*isMultisampled=*/false,
                                      /*isSampled=*/true))
        , fSampler1D(MakeSamplerType("sampler1D", *fTexture1D))
        , fSampler2D(MakeSamplerType("sampler2D", *fTexture2D))
        , fSampler3D(MakeSamplerType("sampler3D", *fTexture3D))
        , fSamplerExternalOES(MakeSamplerType("samplerExternalOES", *fTextureExternalOES))
        , fSampler2DRect(MakeSamplerType("sampler2DRect", *fTexture2DRect))

        , fISampler2D(MakeSamplerType("isampler2D", *fITexture2D))

        , fSampler(MakeSeparateSamplerType("sampler"))

        , fSubpassInput(MakeTextureType("subpassInput",
                                        SpvDimSubpassData,
                                        /*isDepth=*/false,
                                        /*isArrayedTexture=*/false,
                                        /*isMultisampled=*/false,
                                        /*isSampled=*/false))
        , fSubpassInputMS(MakeTextureType("subpassInputMS",
                                          SpvDimSubpassData,
                                          /*isDepth=*/false,
                                          /*isArrayedTexture=*/false,
                                          /*isMultisampled=*/true,
                                          /*isSampled=*/false))

        , fGenType(MakeGenericType("$genType",
                                   {fFloat.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fGenHType(MakeGenericType("$genHType",
                                    {fHalf.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fGenIType(
                  MakeGenericType("$genIType", {fInt.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fGenUType(MakeGenericType("$genUType",
                                    {fUInt.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fGenBType(MakeGenericType("$genBType",
                                    {fBool.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fMat(MakeGenericType("$mat",
                               {fFloat2x2.get(), fFloat2x3.get(), fFloat2x4.get(), fFloat3x2.get(),
                                fFloat3x3.get(), fFloat3x4.get(), fFloat4x2.get(), fFloat4x3.get(),
                                fFloat4x4.get()}))
        , fHMat(MakeGenericType(
                  "$hmat",
                  {fHalf2x2.get(), fHalf2x3.get(), fHalf2x4.get(), fHalf3x2.get(), fHalf3x3.get(),
                   fHalf3x4.get(), fHalf4x2.get(), fHalf4x3.get(), fHalf4x4.get()}))
        , fSquareMat(MakeGenericType("$squareMat",
                                     {fFloat2x2.get(), fFloat3x3.get(), fFloat4x4.get()}))
        , fSquareHMat(
                  MakeGenericType("$squareHMat", {fHalf2x2.get(), fHalf3x3.get(), fHalf4x4.get()}))
        , fVec(MakeGenericType("$vec",
                               {fInvalid.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fHVec(MakeGenericType("$hvec",
                                {fInvalid.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fIVec(MakeGenericType("$ivec", {fInvalid.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fUVec(MakeGenericType("$uvec",
                                {fInvalid.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fSVec(MakeGenericType("$svec",
                                {fInvalid.get(), fShort2.get(), fShort3.get(), fShort4.get()}))
        , fUSVec(MakeGenericType("$usvec",
                                 {fInvalid.get(), fUShort2.get(), fUShort3.get(), fUShort4.get()}))
        , fByteVec(MakeGenericType("$bytevec",
                                   {fInvalid.get(), fByte2.get(), fByte3.get(), fByte4.get()}))
        , fUByteVec(MakeGenericType("$ubytevec",
                                    {fInvalid.get(), fUByte2.get(), fUByte3.get(), fUByte4.get()}))
        , fBVec(MakeGenericType("$bvec",
                                {fInvalid.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fSkCaps(MakeOtherType("$sk_Caps"))
        , fFragmentProcessor(MakeOtherType("fragmentProcessor")) {}

}  // namespace SkSL
