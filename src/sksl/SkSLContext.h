/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include <memory>

#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Contains the built-in, core types for SkSL.
 */
struct BuiltinTypes {
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

    const std::unique_ptr<Type> fImage2D;
    const std::unique_ptr<Type> fIImage2D;

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
};

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(ErrorReporter& errors);

    // The Context holds all of the built-in types.
    BuiltinTypes fTypes;

    // The Context holds a reference to our error reporter.
    ErrorReporter& fErrors;

    // A sentinel expression used to mark that a variable has a value during dataflow analysis (when
    // it could have several different values, or the analyzer is otherwise unable to assign it a
    // specific expression)
    const std::unique_ptr<Expression> fDefined_Expression;
};

}  // namespace SkSL

#endif
