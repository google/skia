/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"

namespace SkSL {

/**
 * Initializes the core SkSL types.
 */
 static std::unique_ptr<Type> make_fp_type(const Type* intType, const Type* boolType) {
    // Build fields for FragmentProcessors, which should parallel the C++ API for
    // GrFragmentProcessor.
    Modifiers mods(Layout(), Modifiers::kConst_Flag);
    std::vector<Type::Field> fields = {
            Type::Field(mods, "numTextureSamplers", intType),
            Type::Field(mods, "numChildProcessors", intType),
            Type::Field(mods, "usesLocalCoords", boolType),
            Type::Field(mods, "compatibleWithCoverageAsAlpha", boolType),
            Type::Field(mods, "preservesOpaqueInput", boolType),
            Type::Field(mods, "hasConstantOutputForConstantInput", boolType)};
    return Type::MakeOtherStruct("fragmentProcessor", std::move(fields));
}

BuiltinTypes::BuiltinTypes()
        : fFloat(Type::MakeScalarType(
                  "float", Type::NumberKind::kFloat, /*priority=*/10, /*highPrecision=*/true))
        , fFloat2(Type::MakeVectorType("float2", *fFloat, /*columns=*/2))
        , fFloat3(Type::MakeVectorType("float3", *fFloat, /*columns=*/3))
        , fFloat4(Type::MakeVectorType("float4", *fFloat, /*columns=*/4))
        , fHalf(Type::MakeScalarType("half", Type::NumberKind::kFloat, /*priority=*/9))
        , fHalf2(Type::MakeVectorType("half2", *fHalf, /*columns=*/2))
        , fHalf3(Type::MakeVectorType("half3", *fHalf, /*columns=*/3))
        , fHalf4(Type::MakeVectorType("half4", *fHalf, /*columns=*/4))
        , fInt(Type::MakeScalarType(
                  "int", Type::NumberKind::kSigned, /*priority=*/7, /*highPrecision=*/true))
        , fInt2(Type::MakeVectorType("int2", *fInt, /*columns=*/2))
        , fInt3(Type::MakeVectorType("int3", *fInt, /*columns=*/3))
        , fInt4(Type::MakeVectorType("int4", *fInt, /*columns=*/4))
        , fUInt(Type::MakeScalarType(
                  "uint", Type::NumberKind::kUnsigned, /*priority=*/6, /*highPrecision=*/true))
        , fUInt2(Type::MakeVectorType("uint2", *fUInt, /*columns=*/2))
        , fUInt3(Type::MakeVectorType("uint3", *fUInt, /*columns=*/3))
        , fUInt4(Type::MakeVectorType("uint4", *fUInt, /*columns=*/4))
        , fShort(Type::MakeScalarType("short", Type::NumberKind::kSigned, /*priority=*/4))
        , fShort2(Type::MakeVectorType("short2", *fShort, /*columns=*/2))
        , fShort3(Type::MakeVectorType("short3", *fShort, /*columns=*/3))
        , fShort4(Type::MakeVectorType("short4", *fShort, /*columns=*/4))
        , fUShort(Type::MakeScalarType("ushort", Type::NumberKind::kUnsigned, /*priority=*/3))
        , fUShort2(Type::MakeVectorType("ushort2", *fUShort, /*columns=*/2))
        , fUShort3(Type::MakeVectorType("ushort3", *fUShort, /*columns=*/3))
        , fUShort4(Type::MakeVectorType("ushort4", *fUShort, /*columns=*/4))
        , fByte(Type::MakeScalarType("byte", Type::NumberKind::kSigned, /*priority=*/2))
        , fByte2(Type::MakeVectorType("byte2", *fByte, /*columns=*/2))
        , fByte3(Type::MakeVectorType("byte3", *fByte, /*columns=*/3))
        , fByte4(Type::MakeVectorType("byte4", *fByte, /*columns=*/4))
        , fUByte(Type::MakeScalarType("ubyte", Type::NumberKind::kUnsigned, /*priority=*/1))
        , fUByte2(Type::MakeVectorType("ubyte2", *fUByte, /*columns=*/2))
        , fUByte3(Type::MakeVectorType("ubyte3", *fUByte, /*columns=*/3))
        , fUByte4(Type::MakeVectorType("ubyte4", *fUByte, /*columns=*/4))
        , fBool(Type::MakeScalarType("bool", Type::NumberKind::kBoolean, /*priority=*/0))
        , fBool2(Type::MakeVectorType("bool2", *fBool, /*columns=*/2))
        , fBool3(Type::MakeVectorType("bool3", *fBool, /*columns=*/3))
        , fBool4(Type::MakeVectorType("bool4", *fBool, /*columns=*/4))
        , fInvalid(Type::MakeOtherType("<INVALID>"))
        , fVoid(Type::MakeOtherType("void"))
        , fFloatLiteral(Type::MakeLiteralType("$floatLiteral", *fFloat, /*priority=*/8))
        , fIntLiteral(Type::MakeLiteralType("$intLiteral", *fInt, /*priority=*/5))
        , fFloat2x2(Type::MakeMatrixType("float2x2", *fFloat, /*columns=*/2, /*rows=*/2))
        , fFloat2x3(Type::MakeMatrixType("float2x3", *fFloat, /*columns=*/2, /*rows=*/3))
        , fFloat2x4(Type::MakeMatrixType("float2x4", *fFloat, /*columns=*/2, /*rows=*/4))
        , fFloat3x2(Type::MakeMatrixType("float3x2", *fFloat, /*columns=*/3, /*rows=*/2))
        , fFloat3x3(Type::MakeMatrixType("float3x3", *fFloat, /*columns=*/3, /*rows=*/3))
        , fFloat3x4(Type::MakeMatrixType("float3x4", *fFloat, /*columns=*/3, /*rows=*/4))
        , fFloat4x2(Type::MakeMatrixType("float4x2", *fFloat, /*columns=*/4, /*rows=*/2))
        , fFloat4x3(Type::MakeMatrixType("float4x3", *fFloat, /*columns=*/4, /*rows=*/3))
        , fFloat4x4(Type::MakeMatrixType("float4x4", *fFloat, /*columns=*/4, /*rows=*/4))
        , fHalf2x2(Type::MakeMatrixType("half2x2", *fHalf, /*columns=*/2, /*rows=*/2))
        , fHalf2x3(Type::MakeMatrixType("half2x3", *fHalf, /*columns=*/2, /*rows=*/3))
        , fHalf2x4(Type::MakeMatrixType("half2x4", *fHalf, /*columns=*/2, /*rows=*/4))
        , fHalf3x2(Type::MakeMatrixType("half3x2", *fHalf, /*columns=*/3, /*rows=*/2))
        , fHalf3x3(Type::MakeMatrixType("half3x3", *fHalf, /*columns=*/3, /*rows=*/3))
        , fHalf3x4(Type::MakeMatrixType("half3x4", *fHalf, /*columns=*/3, /*rows=*/4))
        , fHalf4x2(Type::MakeMatrixType("half4x2", *fHalf, /*columns=*/4, /*rows=*/2))
        , fHalf4x3(Type::MakeMatrixType("half4x3", *fHalf, /*columns=*/4, /*rows=*/3))
        , fHalf4x4(Type::MakeMatrixType("half4x4", *fHalf, /*columns=*/4, /*rows=*/4))
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
        , fTextureCube(Type::MakeTextureType("textureCube",
                                             SpvDimCube,
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

        , fSampler(Type::MakeSimpleType("sampler", Type::TypeKind::kSeparateSampler))
        , fImage2D(Type::MakeTextureType("image2D",
                                         SpvDim2D,
                                         /*isDepth=*/false,
                                         /*isArrayedTexture=*/false,
                                         /*isMultisampled=*/false,
                                         /*isSampled=*/true))
        , fIImage2D(Type::MakeTextureType("iimage2D",
                                          SpvDim2D,
                                          /*isDepth=*/false,
                                          /*isArrayedTexture=*/false,
                                          /*isMultisampled=*/false,
                                          /*isSampled=*/true))

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

        , fGenType(Type::MakeGenericType(
                  "$genType", {fFloat.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fGenHType(Type::MakeGenericType("$genHType",
                                          {fHalf.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fGenIType(Type::MakeGenericType("$genIType",
                                          {fInt.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fGenUType(Type::MakeGenericType("$genUType",
                                          {fUInt.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fGenBType(Type::MakeGenericType("$genBType",
                                          {fBool.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fMat(Type::MakeGenericType("$mat",
                                     {fFloat2x2.get(), fFloat2x3.get(), fFloat2x4.get(),
                                      fFloat3x2.get(), fFloat3x3.get(), fFloat3x4.get(),
                                      fFloat4x2.get(), fFloat4x3.get(), fFloat4x4.get()}))
        , fHMat(Type::MakeGenericType(
                  "$hmat",
                  {fHalf2x2.get(), fHalf2x3.get(), fHalf2x4.get(), fHalf3x2.get(), fHalf3x3.get(),
                   fHalf3x4.get(), fHalf4x2.get(), fHalf4x3.get(), fHalf4x4.get()}))
        , fSquareMat(Type::MakeGenericType("$squareMat",
                                           {fFloat2x2.get(), fFloat3x3.get(), fFloat4x4.get()}))
        , fSquareHMat(Type::MakeGenericType("$squareHMat",
                                            {fHalf2x2.get(), fHalf3x3.get(), fHalf4x4.get()}))
        , fVec(Type::MakeGenericType("$vec",
                                     {fInvalid.get(), fFloat2.get(), fFloat3.get(), fFloat4.get()}))
        , fHVec(Type::MakeGenericType("$hvec",
                                      {fInvalid.get(), fHalf2.get(), fHalf3.get(), fHalf4.get()}))
        , fIVec(Type::MakeGenericType("$ivec",
                                      {fInvalid.get(), fInt2.get(), fInt3.get(), fInt4.get()}))
        , fUVec(Type::MakeGenericType("$uvec",
                                      {fInvalid.get(), fUInt2.get(), fUInt3.get(), fUInt4.get()}))
        , fSVec(Type::MakeGenericType(
                  "$svec", {fInvalid.get(), fShort2.get(), fShort3.get(), fShort4.get()}))
        , fUSVec(Type::MakeGenericType(
                  "$usvec", {fInvalid.get(), fUShort2.get(), fUShort3.get(), fUShort4.get()}))
        , fByteVec(Type::MakeGenericType(
                  "$bytevec", {fInvalid.get(), fByte2.get(), fByte3.get(), fByte4.get()}))
        , fUByteVec(Type::MakeGenericType(
                  "$ubytevec", {fInvalid.get(), fUByte2.get(), fUByte3.get(), fUByte4.get()}))
        , fBVec(Type::MakeGenericType("$bvec",
                                      {fInvalid.get(), fBool2.get(), fBool3.get(), fBool4.get()}))
        , fSkCaps(Type::MakeOtherType("$sk_Caps"))
        , fFragmentProcessor(make_fp_type(fInt.get(), fBool.get())) {}

/**
 * Used as a sentinel expression during dataflow analysis, when an exact value for a variable can't
 * be determined at compile-time.
 */
class DefinedExpression final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kDefined;

    DefinedExpression(const Type* type)
    : INHERITED(/*offset=*/-1, kExpressionKind, type) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    String description() const override {
        return "<defined>";
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<DefinedExpression>(&this->type());
    }

    using INHERITED = Expression;
};

Context::Context(ErrorReporter& errors)
        : fErrors(errors)
        , fDefined_Expression(std::make_unique<DefinedExpression>(fTypes.fInvalid.get())) {}

}  // namespace SkSL

