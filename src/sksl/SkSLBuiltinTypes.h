/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BUILTIN_TYPES
#define SKSL_BUILTIN_TYPES

#include <memory>

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Contains the built-in, core types for SkSL.
 */
class BuiltinTypes {
public:
    BuiltinTypes();

    const std::unique_ptr<Type> fFloat;
    const std::unique_ptr<Type> fFloat2;
    const std::unique_ptr<Type> fFloat3;
    const std::unique_ptr<Type> fFloat4;

    const std::unique_ptr<Type> fHalf;
    const std::unique_ptr<Type> fHalf2;
    const std::unique_ptr<Type> fHalf3;
    const std::unique_ptr<Type> fHalf4;

    const std::unique_ptr<Type> fInt;
    const std::unique_ptr<Type> fInt2;
    const std::unique_ptr<Type> fInt3;
    const std::unique_ptr<Type> fInt4;

    const std::unique_ptr<Type> fUInt;
    const std::unique_ptr<Type> fUInt2;
    const std::unique_ptr<Type> fUInt3;
    const std::unique_ptr<Type> fUInt4;

    const std::unique_ptr<Type> fShort;
    const std::unique_ptr<Type> fShort2;
    const std::unique_ptr<Type> fShort3;
    const std::unique_ptr<Type> fShort4;

    const std::unique_ptr<Type> fUShort;
    const std::unique_ptr<Type> fUShort2;
    const std::unique_ptr<Type> fUShort3;
    const std::unique_ptr<Type> fUShort4;

    const std::unique_ptr<Type> fByte;
    const std::unique_ptr<Type> fByte2;
    const std::unique_ptr<Type> fByte3;
    const std::unique_ptr<Type> fByte4;

    const std::unique_ptr<Type> fUByte;
    const std::unique_ptr<Type> fUByte2;
    const std::unique_ptr<Type> fUByte3;
    const std::unique_ptr<Type> fUByte4;

    const std::unique_ptr<Type> fBool;
    const std::unique_ptr<Type> fBool2;
    const std::unique_ptr<Type> fBool3;
    const std::unique_ptr<Type> fBool4;

    const std::unique_ptr<Type> fInvalid;
    const std::unique_ptr<Type> fVoid;
    const std::unique_ptr<Type> fFloatLiteral;
    const std::unique_ptr<Type> fIntLiteral;

    const std::unique_ptr<Type> fFloat2x2;
    const std::unique_ptr<Type> fFloat2x3;
    const std::unique_ptr<Type> fFloat2x4;
    const std::unique_ptr<Type> fFloat3x2;
    const std::unique_ptr<Type> fFloat3x3;
    const std::unique_ptr<Type> fFloat3x4;
    const std::unique_ptr<Type> fFloat4x2;
    const std::unique_ptr<Type> fFloat4x3;
    const std::unique_ptr<Type> fFloat4x4;

    const std::unique_ptr<Type> fHalf2x2;
    const std::unique_ptr<Type> fHalf2x3;
    const std::unique_ptr<Type> fHalf2x4;
    const std::unique_ptr<Type> fHalf3x2;
    const std::unique_ptr<Type> fHalf3x3;
    const std::unique_ptr<Type> fHalf3x4;
    const std::unique_ptr<Type> fHalf4x2;
    const std::unique_ptr<Type> fHalf4x3;
    const std::unique_ptr<Type> fHalf4x4;

    const std::unique_ptr<Type> fTexture1D;
    const std::unique_ptr<Type> fTexture2D;
    const std::unique_ptr<Type> fTexture3D;
    const std::unique_ptr<Type> fTextureExternalOES;
    const std::unique_ptr<Type> fTextureCube;
    const std::unique_ptr<Type> fTexture2DRect;
    const std::unique_ptr<Type> fITexture2D;

    const std::unique_ptr<Type> fSampler1D;
    const std::unique_ptr<Type> fSampler2D;
    const std::unique_ptr<Type> fSampler3D;
    const std::unique_ptr<Type> fSamplerExternalOES;
    const std::unique_ptr<Type> fSampler2DRect;

    const std::unique_ptr<Type> fISampler2D;
    const std::unique_ptr<Type> fSampler;

    const std::unique_ptr<Type> fSubpassInput;
    const std::unique_ptr<Type> fSubpassInputMS;

    const std::unique_ptr<Type> fGenType;
    const std::unique_ptr<Type> fGenHType;
    const std::unique_ptr<Type> fGenIType;
    const std::unique_ptr<Type> fGenUType;
    const std::unique_ptr<Type> fGenBType;

    const std::unique_ptr<Type> fMat;
    const std::unique_ptr<Type> fHMat;
    const std::unique_ptr<Type> fSquareMat;
    const std::unique_ptr<Type> fSquareHMat;

    const std::unique_ptr<Type> fVec;

    const std::unique_ptr<Type> fHVec;
    const std::unique_ptr<Type> fDVec;
    const std::unique_ptr<Type> fIVec;
    const std::unique_ptr<Type> fUVec;
    const std::unique_ptr<Type> fSVec;
    const std::unique_ptr<Type> fUSVec;
    const std::unique_ptr<Type> fByteVec;
    const std::unique_ptr<Type> fUByteVec;

    const std::unique_ptr<Type> fBVec;

    const std::unique_ptr<Type> fSkCaps;
    const std::unique_ptr<Type> fFragmentProcessor;

private:
    static std::unique_ptr<Type> MakeScalarType(const char* name, Type::NumberKind numberKind,
                                                int priority, bool highPrecision = false);
    static std::unique_ptr<Type> MakeLiteralType(const char* name, const Type& scalarType,
                                                 int priority);
    static std::unique_ptr<Type> MakeVectorType(const char* name, const Type& componentType,
                                                int columns);
    static std::unique_ptr<Type> MakeGenericType(const char* name, std::vector<const Type*> types);
    static std::unique_ptr<Type> MakeMatrixType(const char* name, const Type& componentType,
                                                int columns, int rows);
    static std::unique_ptr<Type> MakeTextureType(const char* name, SpvDim_ dimensions,
                                                 bool isDepth, bool isArrayedTexture,
                                                 bool isMultisampled, bool isSampled);
    static std::unique_ptr<Type> MakeSamplerType(const char* name, const Type& textureType);
    static std::unique_ptr<Type> MakeSeparateSamplerType(const char* name);
    static std::unique_ptr<Type> MakeOtherType(const char* name);
    static std::unique_ptr<Type> MakeVoidType(const char* name);
};

}  // namespace SkSL

#endif
