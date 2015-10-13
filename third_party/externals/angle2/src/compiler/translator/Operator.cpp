//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Operator.h"

const char *GetOperatorString(TOperator op)
{
    switch (op)
    {
      // Note: ops from EOpNull to EOpPrototype can't be handled here.

      case EOpNegative: return "-";
      case EOpPositive: return "+";
      case EOpLogicalNot: return "!";
      case EOpVectorLogicalNot: return "not";
      case EOpBitwiseNot: return "~";

      case EOpPostIncrement: return "++";
      case EOpPostDecrement: return "--";
      case EOpPreIncrement: return "++";
      case EOpPreDecrement: return "--";

      case EOpAdd: return "+";
      case EOpSub: return "-";
      case EOpMul: return "*";
      case EOpDiv: return "/";
      case EOpIMod: return "%";
      case EOpEqual: return "==";
      case EOpNotEqual: return "!=";
      case EOpVectorEqual: return "equal";
      case EOpVectorNotEqual: return "notEqual";
      case EOpLessThan: return "<";
      case EOpGreaterThan: return ">";
      case EOpLessThanEqual: return "<=";
      case EOpGreaterThanEqual: return ">=";
      case EOpComma: return ",";

      // Fall-through.
      case EOpVectorTimesScalar:
      case EOpVectorTimesMatrix:
      case EOpMatrixTimesVector:
      case EOpMatrixTimesScalar: return "*";

      case EOpLogicalOr: return "||";
      case EOpLogicalXor: return "^^";
      case EOpLogicalAnd: return "&&";

      case EOpBitShiftLeft: return "<<";
      case EOpBitShiftRight: return ">>";

      case EOpBitwiseAnd: return "&";
      case EOpBitwiseXor: return "^";
      case EOpBitwiseOr: return "|";

      // Fall-through.
      case EOpIndexDirect:
      case EOpIndexIndirect: return "[]";

      case EOpIndexDirectStruct:
      case EOpIndexDirectInterfaceBlock: return ".";

      case EOpVectorSwizzle: return ".";

      case EOpRadians: return "radians";
      case EOpDegrees: return "degrees";
      case EOpSin: return "sin";
      case EOpCos: return "cos";
      case EOpTan: return "tan";
      case EOpAsin: return "asin";
      case EOpAcos: return "acos";
      case EOpAtan: return "atan";

      case EOpSinh: return "sinh";
      case EOpCosh: return "cosh";
      case EOpTanh: return "tanh";
      case EOpAsinh: return "asinh";
      case EOpAcosh: return "acosh";
      case EOpAtanh: return "atanh";

      case EOpPow: return "pow";
      case EOpExp: return "exp";
      case EOpLog: return "log";
      case EOpExp2: return "exp2";
      case EOpLog2: return "log2";
      case EOpSqrt: return "sqrt";
      case EOpInverseSqrt: return "inversesqrt";

      case EOpAbs: return "abs";
      case EOpSign: return "sign";
      case EOpFloor: return "floor";
      case EOpTrunc: return "trunc";
      case EOpRound: return "round";
      case EOpRoundEven: return "roundEven";
      case EOpCeil: return "ceil";
      case EOpFract: return "fract";
      case EOpMod: return "mod";
      case EOpModf: return "modf";
      case EOpMin: return "min";
      case EOpMax: return "max";
      case EOpClamp: return "clamp";
      case EOpMix: return "mix";
      case EOpStep: return "step";
      case EOpSmoothStep: return "smoothstep";
      case EOpIsNan: return "isnan";
      case EOpIsInf: return "isinf";

      case EOpFloatBitsToInt: return "floatBitsToInt";
      case EOpFloatBitsToUint: return "floatBitsToUint";
      case EOpIntBitsToFloat: return "intBitsToFloat";
      case EOpUintBitsToFloat: return "uintBitsToFloat";

      case EOpPackSnorm2x16: return "packSnorm2x16";
      case EOpPackUnorm2x16: return "packUnorm2x16";
      case EOpPackHalf2x16: return "packHalf2x16";
      case EOpUnpackSnorm2x16: return "unpackSnorm2x16";
      case EOpUnpackUnorm2x16: return "unpackUnorm2x16";
      case EOpUnpackHalf2x16: return "unpackHalf2x16";

      case EOpLength: return "length";
      case EOpDistance: return "distance";
      case EOpDot: return "dot";
      case EOpCross: return "cross";
      case EOpNormalize: return "normalize";
      case EOpFaceForward: return "faceforward";
      case EOpReflect: return "reflect";
      case EOpRefract: return "refract";

      case EOpDFdx: return "dFdx";
      case EOpDFdy: return "dFdy";
      case EOpFwidth: return "fwidth";

      case EOpMatrixTimesMatrix: return "*";

      case EOpOuterProduct: return "outerProduct";
      case EOpTranspose: return "transpose";
      case EOpDeterminant: return "determinant";
      case EOpInverse: return "inverse";

      case EOpAny: return "any";
      case EOpAll: return "all";

      case EOpKill: return "kill";
      case EOpReturn: return "return";
      case EOpBreak: return "break";
      case EOpContinue: return "continue";

      case EOpConstructInt: return "int";
      case EOpConstructUInt: return "uint";
      case EOpConstructBool: return "bool";
      case EOpConstructFloat: return "float";
      case EOpConstructVec2: return "vec2";
      case EOpConstructVec3: return "vec3";
      case EOpConstructVec4: return "vec4";
      case EOpConstructBVec2: return "bvec2";
      case EOpConstructBVec3: return "bvec3";
      case EOpConstructBVec4: return "bvec4";
      case EOpConstructIVec2: return "ivec2";
      case EOpConstructIVec3: return "ivec3";
      case EOpConstructIVec4: return "ivec4";
      case EOpConstructUVec2: return "uvec2";
      case EOpConstructUVec3: return "uvec3";
      case EOpConstructUVec4: return "uvec4";
      case EOpConstructMat2: return "mat2";
      case EOpConstructMat2x3: return "mat2x3";
      case EOpConstructMat2x4: return "mat2x4";
      case EOpConstructMat3x2: return "mat3x2";
      case EOpConstructMat3: return "mat3";
      case EOpConstructMat3x4: return "mat3x4";
      case EOpConstructMat4x2: return "mat4x2";
      case EOpConstructMat4x3: return "mat4x3";
      case EOpConstructMat4: return "mat4";
      // Note: EOpConstructStruct can't be handled here

      case EOpAssign: return "=";
      case EOpInitialize: return "=";
      case EOpAddAssign: return "+=";
      case EOpSubAssign: return "-=";

      // Fall-through.
      case EOpMulAssign:
      case EOpVectorTimesMatrixAssign:
      case EOpVectorTimesScalarAssign:
      case EOpMatrixTimesScalarAssign:
      case EOpMatrixTimesMatrixAssign: return "*=";

      case EOpDivAssign: return "/=";
      case EOpIModAssign: return "%=";
      case EOpBitShiftLeftAssign: return "<<=";
      case EOpBitShiftRightAssign: return ">>=";
      case EOpBitwiseAndAssign: return "&=";
      case EOpBitwiseXorAssign: return "^=";
      case EOpBitwiseOrAssign: return "|=";

      default: break;
    }
    return "";
}

