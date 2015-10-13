//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_OPERATOR_H_
#define COMPILER_TRANSLATOR_OPERATOR_H_

//
// Operators used by the high-level (parse tree) representation.
//
enum TOperator
{
    EOpNull,            // if in a node, should only mean a node is still being built
    EOpSequence,        // denotes a list of statements, or parameters, etc.
    EOpFunctionCall,
    EOpFunction,        // For function definition
    EOpParameters,      // an aggregate listing the parameters to a function

    EOpDeclaration,
    EOpInvariantDeclaration, // Specialized declarations for attributing invariance
    EOpPrototype,

    //
    // Unary operators
    //

    EOpNegative,
    EOpPositive,
    EOpLogicalNot,
    EOpVectorLogicalNot,
    EOpBitwiseNot,

    EOpPostIncrement,
    EOpPostDecrement,
    EOpPreIncrement,
    EOpPreDecrement,

    //
    // binary operations
    //

    EOpAdd,
    EOpSub,
    EOpMul,
    EOpDiv,
    EOpIMod,
    EOpEqual,
    EOpNotEqual,
    EOpVectorEqual,
    EOpVectorNotEqual,
    EOpLessThan,
    EOpGreaterThan,
    EOpLessThanEqual,
    EOpGreaterThanEqual,
    EOpComma,

    EOpVectorTimesScalar,
    EOpVectorTimesMatrix,
    EOpMatrixTimesVector,
    EOpMatrixTimesScalar,

    EOpLogicalOr,
    EOpLogicalXor,
    EOpLogicalAnd,

    EOpBitShiftLeft,
    EOpBitShiftRight,

    EOpBitwiseAnd,
    EOpBitwiseXor,
    EOpBitwiseOr,

    EOpIndexDirect,
    EOpIndexIndirect,
    EOpIndexDirectStruct,
    EOpIndexDirectInterfaceBlock,

    EOpVectorSwizzle,

    //
    // Built-in functions potentially mapped to operators
    //

    EOpRadians,
    EOpDegrees,
    EOpSin,
    EOpCos,
    EOpTan,
    EOpAsin,
    EOpAcos,
    EOpAtan,

    EOpSinh,
    EOpCosh,
    EOpTanh,
    EOpAsinh,
    EOpAcosh,
    EOpAtanh,

    EOpPow,
    EOpExp,
    EOpLog,
    EOpExp2,
    EOpLog2,
    EOpSqrt,
    EOpInverseSqrt,

    EOpAbs,
    EOpSign,
    EOpFloor,
    EOpTrunc,
    EOpRound,
    EOpRoundEven,
    EOpCeil,
    EOpFract,
    EOpMod,
    EOpModf,
    EOpMin,
    EOpMax,
    EOpClamp,
    EOpMix,
    EOpStep,
    EOpSmoothStep,
    EOpIsNan,
    EOpIsInf,

    EOpFloatBitsToInt,
    EOpFloatBitsToUint,
    EOpIntBitsToFloat,
    EOpUintBitsToFloat,

    EOpPackSnorm2x16,
    EOpPackUnorm2x16,
    EOpPackHalf2x16,
    EOpUnpackSnorm2x16,
    EOpUnpackUnorm2x16,
    EOpUnpackHalf2x16,

    EOpLength,
    EOpDistance,
    EOpDot,
    EOpCross,
    EOpNormalize,
    EOpFaceForward,
    EOpReflect,
    EOpRefract,

    EOpDFdx,            // Fragment only, OES_standard_derivatives extension
    EOpDFdy,            // Fragment only, OES_standard_derivatives extension
    EOpFwidth,          // Fragment only, OES_standard_derivatives extension

    EOpMatrixTimesMatrix,

    EOpOuterProduct,
    EOpTranspose,
    EOpDeterminant,
    EOpInverse,

    EOpAny,
    EOpAll,

    //
    // Branch
    //

    EOpKill,            // Fragment only
    EOpReturn,
    EOpBreak,
    EOpContinue,

    //
    // Constructors
    //

    EOpConstructInt,
    EOpConstructUInt,
    EOpConstructBool,
    EOpConstructFloat,
    EOpConstructVec2,
    EOpConstructVec3,
    EOpConstructVec4,
    EOpConstructBVec2,
    EOpConstructBVec3,
    EOpConstructBVec4,
    EOpConstructIVec2,
    EOpConstructIVec3,
    EOpConstructIVec4,
    EOpConstructUVec2,
    EOpConstructUVec3,
    EOpConstructUVec4,
    EOpConstructMat2,
    EOpConstructMat2x3,
    EOpConstructMat2x4,
    EOpConstructMat3x2,
    EOpConstructMat3,
    EOpConstructMat3x4,
    EOpConstructMat4x2,
    EOpConstructMat4x3,
    EOpConstructMat4,
    EOpConstructStruct,

    //
    // moves
    //

    EOpAssign,
    EOpInitialize,
    EOpAddAssign,
    EOpSubAssign,

    EOpMulAssign,
    EOpVectorTimesMatrixAssign,
    EOpVectorTimesScalarAssign,
    EOpMatrixTimesScalarAssign,
    EOpMatrixTimesMatrixAssign,

    EOpDivAssign,
    EOpIModAssign,
    EOpBitShiftLeftAssign,
    EOpBitShiftRightAssign,
    EOpBitwiseAndAssign,
    EOpBitwiseXorAssign,
    EOpBitwiseOrAssign
};

// Returns the string corresponding to the operator in GLSL
const char* GetOperatorString(TOperator op);

#endif  // COMPILER_TRANSLATOR_OPERATOR_H_
