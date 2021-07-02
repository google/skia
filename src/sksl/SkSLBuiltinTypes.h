/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BUILTIN_TYPES
#define SKSL_BUILTIN_TYPES

#include <memory>

#include "src/sksl/ir/SkSLArrayType.h"
#include "src/sksl/ir/SkSLGenericType.h"
#include "src/sksl/ir/SkSLLiteralType.h"
#include "src/sksl/ir/SkSLMatrixType.h"
#include "src/sksl/ir/SkSLSamplerType.h"
#include "src/sksl/ir/SkSLScalarType.h"
#include "src/sksl/ir/SkSLTextureType.h"
#include "src/sksl/ir/SkSLVectorType.h"

namespace SkSL {

/**
 * Contains the built-in, core types for SkSL.
 */
class BuiltinTypes {
public:
    BuiltinTypes();

    const std::unique_ptr<ScalarType> fFloat;
    const std::unique_ptr<VectorType> fFloat2;
    const std::unique_ptr<VectorType> fFloat3;
    const std::unique_ptr<VectorType> fFloat4;

    const std::unique_ptr<ScalarType> fHalf;
    const std::unique_ptr<VectorType> fHalf2;
    const std::unique_ptr<VectorType> fHalf3;
    const std::unique_ptr<VectorType> fHalf4;

    const std::unique_ptr<ScalarType> fInt;
    const std::unique_ptr<VectorType> fInt2;
    const std::unique_ptr<VectorType> fInt3;
    const std::unique_ptr<VectorType> fInt4;

    const std::unique_ptr<ScalarType> fUInt;
    const std::unique_ptr<VectorType> fUInt2;
    const std::unique_ptr<VectorType> fUInt3;
    const std::unique_ptr<VectorType> fUInt4;

    const std::unique_ptr<ScalarType> fShort;
    const std::unique_ptr<VectorType> fShort2;
    const std::unique_ptr<VectorType> fShort3;
    const std::unique_ptr<VectorType> fShort4;

    const std::unique_ptr<ScalarType> fUShort;
    const std::unique_ptr<VectorType> fUShort2;
    const std::unique_ptr<VectorType> fUShort3;
    const std::unique_ptr<VectorType> fUShort4;

    const std::unique_ptr<ScalarType> fBool;
    const std::unique_ptr<VectorType> fBool2;
    const std::unique_ptr<VectorType> fBool3;
    const std::unique_ptr<VectorType> fBool4;

    const std::unique_ptr<Type> fInvalid;
    const std::unique_ptr<Type> fPoison;
    const std::unique_ptr<Type> fVoid;
    const std::unique_ptr<LiteralType> fFloatLiteral;
    const std::unique_ptr<LiteralType> fIntLiteral;

    const std::unique_ptr<MatrixType> fFloat2x2;
    const std::unique_ptr<MatrixType> fFloat2x3;
    const std::unique_ptr<MatrixType> fFloat2x4;
    const std::unique_ptr<MatrixType> fFloat3x2;
    const std::unique_ptr<MatrixType> fFloat3x3;
    const std::unique_ptr<MatrixType> fFloat3x4;
    const std::unique_ptr<MatrixType> fFloat4x2;
    const std::unique_ptr<MatrixType> fFloat4x3;
    const std::unique_ptr<MatrixType> fFloat4x4;

    const std::unique_ptr<MatrixType> fHalf2x2;
    const std::unique_ptr<MatrixType> fHalf2x3;
    const std::unique_ptr<MatrixType> fHalf2x4;
    const std::unique_ptr<MatrixType> fHalf3x2;
    const std::unique_ptr<MatrixType> fHalf3x3;
    const std::unique_ptr<MatrixType> fHalf3x4;
    const std::unique_ptr<MatrixType> fHalf4x2;
    const std::unique_ptr<MatrixType> fHalf4x3;
    const std::unique_ptr<MatrixType> fHalf4x4;

    const std::unique_ptr<TextureType> fTexture1D;
    const std::unique_ptr<TextureType> fTexture2D;
    const std::unique_ptr<TextureType> fTexture3D;
    const std::unique_ptr<TextureType> fTextureExternalOES;
    const std::unique_ptr<TextureType> fTextureCube;
    const std::unique_ptr<TextureType> fTexture2DRect;
    const std::unique_ptr<TextureType> fITexture2D;

    const std::unique_ptr<SamplerType> fSampler1D;
    const std::unique_ptr<SamplerType> fSampler2D;
    const std::unique_ptr<SamplerType> fSampler3D;
    const std::unique_ptr<SamplerType> fSamplerExternalOES;
    const std::unique_ptr<SamplerType> fSampler2DRect;

    const std::unique_ptr<SamplerType> fISampler2D;
    const std::unique_ptr<Type> fSampler;

    const std::unique_ptr<Type> fSubpassInput;
    const std::unique_ptr<Type> fSubpassInputMS;

    const std::unique_ptr<GenericType> fGenType;
    const std::unique_ptr<GenericType> fGenHType;
    const std::unique_ptr<GenericType> fGenIType;
    const std::unique_ptr<GenericType> fGenUType;
    const std::unique_ptr<GenericType> fGenBType;

    const std::unique_ptr<GenericType> fMat;
    const std::unique_ptr<GenericType> fHMat;
    const std::unique_ptr<GenericType> fSquareMat;
    const std::unique_ptr<GenericType> fSquareHMat;

    const std::unique_ptr<GenericType> fVec;

    const std::unique_ptr<GenericType> fHVec;
    const std::unique_ptr<GenericType> fDVec;
    const std::unique_ptr<GenericType> fIVec;
    const std::unique_ptr<GenericType> fUVec;
    const std::unique_ptr<GenericType> fSVec;
    const std::unique_ptr<GenericType> fUSVec;
    const std::unique_ptr<GenericType> fByteVec;
    const std::unique_ptr<GenericType> fUByteVec;

    const std::unique_ptr<Type> fBVec;

    const std::unique_ptr<Type> fSkCaps;
    const std::unique_ptr<Type> fFragmentProcessor;

    const std::unique_ptr<Type> fColorFilter;
    const std::unique_ptr<Type> fShader;

private:
    static std::unique_ptr<GenericType> MakeGenericType(const char* name,
                                                        std::vector<const Type*> types);

    std::unique_ptr<Type> MakeSpecialType(const char* name, const char* abbrev,
                                          Type::TypeKind typeKind);
};

}  // namespace SkSL

#endif
