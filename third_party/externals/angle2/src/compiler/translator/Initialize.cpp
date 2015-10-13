//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Create symbols that declare built-in definitions, add built-ins that
// cannot be expressed in the files, and establish mappings between 
// built-in functions and operators.
//

#include "compiler/translator/Initialize.h"
#include "compiler/translator/Cache.h"

#include "compiler/translator/IntermNode.h"
#include "angle_gl.h"

void InsertBuiltInFunctions(sh::GLenum type, ShShaderSpec spec, const ShBuiltInResources &resources, TSymbolTable &symbolTable)
{
    const TType *float1 = TCache::getType(EbtFloat);
    const TType *float2 = TCache::getType(EbtFloat, 2);
    const TType *float3 = TCache::getType(EbtFloat, 3);
    const TType *float4 = TCache::getType(EbtFloat, 4);
    const TType *int1 = TCache::getType(EbtInt);
    const TType *int2 = TCache::getType(EbtInt, 2);
    const TType *int3 = TCache::getType(EbtInt, 3);
    const TType *uint1 = TCache::getType(EbtUInt);
    const TType *bool1 = TCache::getType(EbtBool);
    const TType *genType = TCache::getType(EbtGenType);
    const TType *genIType = TCache::getType(EbtGenIType);
    const TType *genUType = TCache::getType(EbtGenUType);
    const TType *genBType = TCache::getType(EbtGenBType);

    //
    // Angle and Trigonometric Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpRadians, genType, "radians", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpDegrees, genType, "degrees", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpSin, genType, "sin", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpCos, genType, "cos", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpTan, genType, "tan", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAsin, genType, "asin", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAcos, genType, "acos", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAtan, genType, "atan", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAtan, genType, "atan", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpSinh, genType, "sinh", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpCosh, genType, "cosh", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTanh, genType, "tanh", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpAsinh, genType, "asinh", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpAcosh, genType, "acosh", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpAtanh, genType, "atanh", genType);

    //
    // Exponential Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpPow, genType, "pow", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpExp, genType, "exp", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLog, genType, "log", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpExp2, genType, "exp2", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLog2, genType, "log2", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpSqrt, genType, "sqrt", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpInverseSqrt, genType, "inversesqrt", genType);

    //
    // Common Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAbs, genType, "abs", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpAbs, genIType, "abs", genIType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpSign, genType, "sign", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpSign, genIType, "sign", genIType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpFloor, genType, "floor", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTrunc, genType, "trunc", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpRound, genType, "round", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpRoundEven, genType, "roundEven", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpCeil, genType, "ceil", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpFract, genType, "fract", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMod, genType, "mod", genType, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMod, genType, "mod", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMin, genType, "min", genType, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMin, genType, "min", genType, genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMin, genIType, "min", genIType, genIType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMin, genIType, "min", genIType, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMin, genUType, "min", genUType, genUType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMin, genUType, "min", genUType, uint1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMax, genType, "max", genType, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMax, genType, "max", genType, genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMax, genIType, "max", genIType, genIType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMax, genIType, "max", genIType, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMax, genUType, "max", genUType, genUType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMax, genUType, "max", genUType, uint1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpClamp, genType, "clamp", genType, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpClamp, genType, "clamp", genType, genType, genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpClamp, genIType, "clamp", genIType, int1, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpClamp, genIType, "clamp", genIType, genIType, genIType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpClamp, genUType, "clamp", genUType, uint1, uint1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpClamp, genUType, "clamp", genUType, genUType, genUType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMix, genType, "mix", genType, genType, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMix, genType, "mix", genType, genType, genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMix, genType, "mix", genType, genType, genBType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpStep, genType, "step", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpStep, genType, "step", float1, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpSmoothStep, genType, "smoothstep", genType, genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpSmoothStep, genType, "smoothstep", float1, float1, genType);

    const TType *outFloat1 = TCache::getType(EbtFloat, EvqOut);
    const TType *outFloat2 = TCache::getType(EbtFloat, EvqOut, 2);
    const TType *outFloat3 = TCache::getType(EbtFloat, EvqOut, 3);
    const TType *outFloat4 = TCache::getType(EbtFloat, EvqOut, 4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpModf, float1, "modf", float1, outFloat1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpModf, float2, "modf", float2, outFloat2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpModf, float3, "modf", float3, outFloat3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpModf, float4, "modf", float4, outFloat4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpIsNan, genBType, "isnan", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpIsInf, genBType, "isinf", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpFloatBitsToInt, genIType, "floatBitsToInt", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpFloatBitsToUint, genUType, "floatBitsToUint", genType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpIntBitsToFloat, genType, "intBitsToFloat", genIType);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpUintBitsToFloat, genType, "uintBitsToFloat", genUType);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpPackSnorm2x16, uint1, "packSnorm2x16", float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpPackUnorm2x16, uint1, "packUnorm2x16", float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpPackHalf2x16, uint1, "packHalf2x16", float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpUnpackSnorm2x16, float2, "unpackSnorm2x16", uint1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpUnpackUnorm2x16, float2, "unpackUnorm2x16", uint1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpUnpackHalf2x16, float2, "unpackHalf2x16", uint1);

    //
    // Geometric Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLength, float1, "length", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpDistance, float1, "distance", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpDot, float1, "dot", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpCross, float3, "cross", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpNormalize, genType, "normalize", genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpFaceForward, genType, "faceforward", genType, genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpReflect, genType, "reflect", genType, genType);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpRefract, genType, "refract", genType, genType, float1);

    const TType *mat2 = TCache::getType(EbtFloat, 2, 2);
    const TType *mat3 = TCache::getType(EbtFloat, 3, 3);
    const TType *mat4 = TCache::getType(EbtFloat, 4, 4);
    const TType *mat2x3 = TCache::getType(EbtFloat, 2, 3);
    const TType *mat3x2 = TCache::getType(EbtFloat, 3, 2);
    const TType *mat2x4 = TCache::getType(EbtFloat, 2, 4);
    const TType *mat4x2 = TCache::getType(EbtFloat, 4, 2);
    const TType *mat3x4 = TCache::getType(EbtFloat, 3, 4);
    const TType *mat4x3 = TCache::getType(EbtFloat, 4, 3);

    //
    // Matrix Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMul, mat2, "matrixCompMult", mat2, mat2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMul, mat3, "matrixCompMult", mat3, mat3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpMul, mat4, "matrixCompMult", mat4, mat4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat2x3, "matrixCompMult", mat2x3, mat2x3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat3x2, "matrixCompMult", mat3x2, mat3x2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat2x4, "matrixCompMult", mat2x4, mat2x4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat4x2, "matrixCompMult", mat4x2, mat4x2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat3x4, "matrixCompMult", mat3x4, mat3x4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpMul, mat4x3, "matrixCompMult", mat4x3, mat4x3);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat2, "outerProduct", float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat3, "outerProduct", float3, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat4, "outerProduct", float4, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat2x3, "outerProduct", float3, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat3x2, "outerProduct", float2, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat2x4, "outerProduct", float4, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat4x2, "outerProduct", float2, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat3x4, "outerProduct", float4, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpOuterProduct, mat4x3, "outerProduct", float3, float4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat2, "transpose", mat2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat3, "transpose", mat3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat4, "transpose", mat4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat2x3, "transpose", mat3x2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat3x2, "transpose", mat2x3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat2x4, "transpose", mat4x2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat4x2, "transpose", mat2x4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat3x4, "transpose", mat4x3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpTranspose, mat4x3, "transpose", mat3x4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpDeterminant, float1, "determinant", mat2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpDeterminant, float1, "determinant", mat3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpDeterminant, float1, "determinant", mat4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpInverse, mat2, "inverse", mat2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpInverse, mat3, "inverse", mat3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpInverse, mat4, "inverse", mat4);

    const TType *vec = TCache::getType(EbtVec);
    const TType *ivec = TCache::getType(EbtIVec);
    const TType *uvec = TCache::getType(EbtUVec);
    const TType *bvec = TCache::getType(EbtBVec);

    //
    // Vector relational functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLessThan, bvec, "lessThan", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLessThan, bvec, "lessThan", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpLessThan, bvec, "lessThan", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLessThanEqual, bvec, "lessThanEqual", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpLessThanEqual, bvec, "lessThanEqual", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpLessThanEqual, bvec, "lessThanEqual", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpGreaterThan, bvec, "greaterThan", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpGreaterThan, bvec, "greaterThan", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpGreaterThan, bvec, "greaterThan", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpGreaterThanEqual, bvec, "greaterThanEqual", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpGreaterThanEqual, bvec, "greaterThanEqual", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpGreaterThanEqual, bvec, "greaterThanEqual", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorEqual, bvec, "equal", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorEqual, bvec, "equal", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpVectorEqual, bvec, "equal", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorEqual, bvec, "equal", bvec, bvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorNotEqual, bvec, "notEqual", vec, vec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorNotEqual, bvec, "notEqual", ivec, ivec);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpVectorNotEqual, bvec, "notEqual", uvec, uvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorNotEqual, bvec, "notEqual", bvec, bvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAny, bool1, "any", bvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpAll, bool1, "all", bvec);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, EOpVectorLogicalNot, bvec, "not", bvec);

    const TType *sampler2D = TCache::getType(EbtSampler2D);
    const TType *samplerCube = TCache::getType(EbtSamplerCube);

    //
    // Texture Functions for GLSL ES 1.0
    //
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", sampler2D, float2);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float3);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float4);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCube", samplerCube, float3);

    if (resources.OES_EGL_image_external)
    {
        const TType *samplerExternalOES = TCache::getType(EbtSamplerExternalOES);

        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", samplerExternalOES, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", samplerExternalOES, float3);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", samplerExternalOES, float4);
    }

    if (resources.ARB_texture_rectangle)
    {
        const TType *sampler2DRect = TCache::getType(EbtSampler2DRect);

        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRect", sampler2DRect, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRectProj", sampler2DRect, float3);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRectProj", sampler2DRect, float4);
    }

    if (resources.EXT_shader_texture_lod)
    {
        /* The *Grad* variants are new to both vertex and fragment shaders; the fragment
         * shader specific pieces are added separately below.
         */
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DGradEXT", sampler2D, float2, float2, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DProjGradEXT", sampler2D, float3, float2, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DProjGradEXT", sampler2D, float4, float2, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "textureCubeGradEXT", samplerCube, float3, float3, float3);
    }

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", sampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCube", samplerCube, float3, float1);

        if (resources.OES_standard_derivatives)
        {
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, EOpDFdx, "GL_OES_standard_derivatives", genType, "dFdx", genType);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, EOpDFdy, "GL_OES_standard_derivatives", genType, "dFdy", genType);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, EOpFwidth, "GL_OES_standard_derivatives", genType, "fwidth", genType);
        }

        if (resources.EXT_shader_texture_lod)
        {
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DLodEXT", sampler2D, float2, float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DProjLodEXT", sampler2D, float3, float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "texture2DProjLodEXT", sampler2D, float4, float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, "GL_EXT_shader_texture_lod", float4, "textureCubeLodEXT", samplerCube, float3, float1);
        }
    }

    if (type == GL_VERTEX_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DLod", sampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProjLod", sampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProjLod", sampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCubeLod", samplerCube, float3, float1);
    }

    const TType *gvec4 = TCache::getType(EbtGVec4);

    const TType *gsampler2D = TCache::getType(EbtGSampler2D);
    const TType *gsamplerCube = TCache::getType(EbtGSamplerCube);
    const TType *gsampler3D = TCache::getType(EbtGSampler3D);
    const TType *gsampler2DArray = TCache::getType(EbtGSampler2DArray);

    //
    // Texture Functions for GLSL ES 3.0
    //
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler2D, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler3D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsamplerCube, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler2DArray, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler2D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler2D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler3D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLod", gsampler2D, float2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLod", gsampler3D, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLod", gsamplerCube, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLod", gsampler2DArray, float3, float1);

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler3D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsamplerCube, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texture", gsampler2DArray, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProj", gsampler3D, float4, float1);
    }

    const TType *sampler2DShadow = TCache::getType(EbtSampler2DShadow);
    const TType *samplerCubeShadow = TCache::getType(EbtSamplerCubeShadow);
    const TType *sampler2DArrayShadow = TCache::getType(EbtSampler2DArrayShadow);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DShadow, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", samplerCubeShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DArrayShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProj", sampler2DShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureLod", sampler2DShadow, float3, float1);

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DShadow, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", samplerCubeShadow, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProj", sampler2DShadow, float4, float1);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", gsampler2D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", gsampler3D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", gsamplerCube, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", gsampler2DArray, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", sampler2DShadow, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", samplerCubeShadow, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", sampler2DArrayShadow, int1);

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpDFdx, genType, "dFdx", genType);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpDFdy, genType, "dFdy", genType);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, EOpFwidth, genType, "fwidth", genType);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler2D, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler3D, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureOffset", sampler2DShadow, float3, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler2DArray, float3, int2);

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler2D, float2, int2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler3D, float3, int3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureOffset", sampler2DShadow, float3, int2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureOffset", gsampler2DArray, float3, int2, float1);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler2D, float3, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler2D, float4, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler3D, float4, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjOffset", sampler2DShadow, float4, int2);

    if (type == GL_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler2D, float3, int2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler2D, float4, int2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjOffset", gsampler3D, float4, int3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjOffset", sampler2DShadow, float4, int2, float1);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLodOffset", gsampler2D, float2, float1, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLodOffset", gsampler3D, float3, float1, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureLodOffset", sampler2DShadow, float3, float1, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureLodOffset", gsampler2DArray, float3, float1, int2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLod", gsampler2D, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLod", gsampler2D, float4, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLod", gsampler3D, float4, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjLod", sampler2DShadow, float4, float1);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLodOffset", gsampler2D, float3, float1, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLodOffset", gsampler2D, float4, float1, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjLodOffset", gsampler3D, float4, float1, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjLodOffset", sampler2DShadow, float4, float1, int2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetch", gsampler2D, int2, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetch", gsampler3D, int3, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetch", gsampler2DArray, int3, int1);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetchOffset", gsampler2D, int2, int1, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetchOffset", gsampler3D, int3, int1, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "texelFetchOffset", gsampler2DArray, int3, int1, int2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGrad", gsampler2D, float2, float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGrad", gsampler3D, float3, float3, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGrad", gsamplerCube, float3, float3, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureGrad", sampler2DShadow, float3, float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureGrad", samplerCubeShadow, float4, float3, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGrad", gsampler2DArray, float3, float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureGrad", sampler2DArrayShadow, float4, float2, float2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGradOffset", gsampler2D, float2, float2, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGradOffset", gsampler3D, float3, float3, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureGradOffset", sampler2DShadow, float3, float2, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureGradOffset", gsampler2DArray, float3, float2, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureGradOffset", sampler2DArrayShadow, float4, float2, float2, int2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGrad", gsampler2D, float3, float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGrad", gsampler2D, float4, float2, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGrad", gsampler3D, float4, float3, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjGrad", sampler2DShadow, float4, float2, float2);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGradOffset", gsampler2D, float3, float2, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGradOffset", gsampler2D, float4, float2, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, gvec4, "textureProjGradOffset", gsampler3D, float4, float3, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProjGradOffset", sampler2DShadow, float4, float2, float2, int2);

    //
    // Depth range in window coordinates
    //
    TFieldList *fields = NewPoolTFieldList();
    TSourceLoc zeroSourceLoc = {0, 0, 0, 0};
    TField *near = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("near"), zeroSourceLoc);
    TField *far = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("far"), zeroSourceLoc);
    TField *diff = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("diff"), zeroSourceLoc);
    fields->push_back(near);
    fields->push_back(far);
    fields->push_back(diff);
    TStructure *depthRangeStruct = new TStructure(NewPoolTString("gl_DepthRangeParameters"), fields);
    TVariable *depthRangeParameters = new TVariable(&depthRangeStruct->name(), depthRangeStruct, true);
    symbolTable.insert(COMMON_BUILTINS, depthRangeParameters);
    TVariable *depthRange = new TVariable(NewPoolTString("gl_DepthRange"), TType(depthRangeStruct));
    depthRange->setQualifier(EvqUniform);
    symbolTable.insert(COMMON_BUILTINS, depthRange);

    //
    // Implementation dependent built-in constants.
    //
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexAttribs", resources.MaxVertexAttribs);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexUniformVectors", resources.MaxVertexUniformVectors);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexTextureImageUnits", resources.MaxVertexTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxCombinedTextureImageUnits", resources.MaxCombinedTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxTextureImageUnits", resources.MaxTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxFragmentUniformVectors", resources.MaxFragmentUniformVectors);

    symbolTable.insertConstInt(ESSL1_BUILTINS, "gl_MaxVaryingVectors", resources.MaxVaryingVectors);

    if (spec != SH_CSS_SHADERS_SPEC)
    {
        symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxDrawBuffers", resources.MaxDrawBuffers);
        if (resources.EXT_blend_func_extended)
        {
            symbolTable.insertConstIntExt(COMMON_BUILTINS, "GL_EXT_blend_func_extended",
                                          "gl_MaxDualSourceDrawBuffersEXT",
                                          resources.MaxDualSourceDrawBuffers);
        }
    }

    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxVertexOutputVectors", resources.MaxVertexOutputVectors);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxFragmentInputVectors", resources.MaxFragmentInputVectors);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MinProgramTexelOffset", resources.MinProgramTexelOffset);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxProgramTexelOffset", resources.MaxProgramTexelOffset);
}

void IdentifyBuiltIns(sh::GLenum type, ShShaderSpec spec,
                      const ShBuiltInResources &resources,
                      TSymbolTable &symbolTable)
{
    //
    // Insert some special built-in variables that are not in
    // the built-in header files.
    //
    switch (type)
    {
      case GL_FRAGMENT_SHADER:
        symbolTable.insert(COMMON_BUILTINS, new TVariable(NewPoolTString("gl_FragCoord"),
            TType(EbtFloat, EbpMedium, EvqFragCoord, 4)));
        symbolTable.insert(COMMON_BUILTINS, new TVariable(NewPoolTString("gl_FrontFacing"),
            TType(EbtBool,  EbpUndefined, EvqFrontFacing, 1)));
        symbolTable.insert(COMMON_BUILTINS, new TVariable(NewPoolTString("gl_PointCoord"),
            TType(EbtFloat, EbpMedium, EvqPointCoord, 2)));

        //
        // In CSS Shaders, gl_FragColor, gl_FragData, and gl_MaxDrawBuffers are not available.
        // Instead, css_MixColor and css_ColorMatrix are available.
        //
        if (spec != SH_CSS_SHADERS_SPEC)
        {
            symbolTable.insert(ESSL1_BUILTINS, new TVariable(NewPoolTString("gl_FragColor"),
                TType(EbtFloat, EbpMedium, EvqFragColor, 4)));
            TType fragData(EbtFloat, EbpMedium, EvqFragData, 4, 1, true);
            fragData.setArraySize(resources.MaxDrawBuffers);
            symbolTable.insert(ESSL1_BUILTINS, new TVariable(NewPoolTString("gl_FragData"), fragData));

            if (resources.EXT_blend_func_extended)
            {
                symbolTable.insert(
                    ESSL1_BUILTINS, "GL_EXT_blend_func_extended",
                    new TVariable(NewPoolTString("gl_SecondaryFragColorEXT"),
                                  TType(EbtFloat, EbpMedium, EvqSecondaryFragColorEXT, 4)));
                TType secondaryFragData(EbtFloat, EbpMedium, EvqSecondaryFragDataEXT, 4, 1, true);
                secondaryFragData.setArraySize(resources.MaxDualSourceDrawBuffers);
                symbolTable.insert(
                    ESSL1_BUILTINS, "GL_EXT_blend_func_extended",
                    new TVariable(NewPoolTString("gl_SecondaryFragDataEXT"), secondaryFragData));
            }

            if (resources.EXT_frag_depth)
            {
                symbolTable.insert(ESSL1_BUILTINS, "GL_EXT_frag_depth", new TVariable(NewPoolTString("gl_FragDepthEXT"),
                    TType(EbtFloat, resources.FragmentPrecisionHigh ? EbpHigh : EbpMedium, EvqFragDepth, 1)));
            }

            if (resources.EXT_shader_framebuffer_fetch || resources.NV_shader_framebuffer_fetch)
            {
                TType lastFragData(EbtFloat, EbpMedium, EvqLastFragData, 4, 1, true);
                lastFragData.setArraySize(resources.MaxDrawBuffers);

                if (resources.EXT_shader_framebuffer_fetch)
                {
                    symbolTable.insert(ESSL1_BUILTINS, "GL_EXT_shader_framebuffer_fetch",
                        new TVariable(NewPoolTString("gl_LastFragData"), lastFragData));
                }
                else if (resources.NV_shader_framebuffer_fetch)
                {
                    symbolTable.insert(ESSL1_BUILTINS, "GL_NV_shader_framebuffer_fetch",
                        new TVariable(NewPoolTString("gl_LastFragColor"),
                        TType(EbtFloat, EbpMedium, EvqLastFragColor, 4)));
                    symbolTable.insert(ESSL1_BUILTINS, "GL_NV_shader_framebuffer_fetch",
                        new TVariable(NewPoolTString("gl_LastFragData"), lastFragData));
                }
            }
            else if (resources.ARM_shader_framebuffer_fetch)
            {
                symbolTable.insert(ESSL1_BUILTINS, "GL_ARM_shader_framebuffer_fetch",
                    new TVariable(NewPoolTString("gl_LastFragColorARM"),
                    TType(EbtFloat, EbpMedium, EvqLastFragColor, 4)));
            }
        }
        else
        {
            symbolTable.insert(ESSL1_BUILTINS, new TVariable(NewPoolTString("css_MixColor"),
                TType(EbtFloat, EbpMedium, EvqGlobal, 4)));
            symbolTable.insert(ESSL1_BUILTINS, new TVariable(NewPoolTString("css_ColorMatrix"),
                TType(EbtFloat, EbpMedium, EvqGlobal, 4, 4)));
        }

        break;

      case GL_VERTEX_SHADER:
        symbolTable.insert(COMMON_BUILTINS, new TVariable(NewPoolTString("gl_Position"),
            TType(EbtFloat, EbpHigh, EvqPosition, 4)));
        symbolTable.insert(COMMON_BUILTINS, new TVariable(NewPoolTString("gl_PointSize"),
            TType(EbtFloat, EbpMedium, EvqPointSize, 1)));
        symbolTable.insert(ESSL3_BUILTINS, new TVariable(NewPoolTString("gl_InstanceID"),
            TType(EbtInt, EbpHigh, EvqInstanceID, 1)));
        break;

      default:
        assert(false && "Language not supported");
    }
}

void InitExtensionBehavior(const ShBuiltInResources& resources,
                           TExtensionBehavior& extBehavior)
{
    if (resources.OES_standard_derivatives)
        extBehavior["GL_OES_standard_derivatives"] = EBhUndefined;
    if (resources.OES_EGL_image_external)
        extBehavior["GL_OES_EGL_image_external"] = EBhUndefined;
    if (resources.ARB_texture_rectangle)
        extBehavior["GL_ARB_texture_rectangle"] = EBhUndefined;
    if (resources.EXT_blend_func_extended)
        extBehavior["GL_EXT_blend_func_extended"] = EBhUndefined;
    if (resources.EXT_draw_buffers)
        extBehavior["GL_EXT_draw_buffers"] = EBhUndefined;
    if (resources.EXT_frag_depth)
        extBehavior["GL_EXT_frag_depth"] = EBhUndefined;
    if (resources.EXT_shader_texture_lod)
        extBehavior["GL_EXT_shader_texture_lod"] = EBhUndefined;
    if (resources.EXT_shader_framebuffer_fetch)
        extBehavior["GL_EXT_shader_framebuffer_fetch"] = EBhUndefined;
    if (resources.NV_shader_framebuffer_fetch)
        extBehavior["GL_NV_shader_framebuffer_fetch"] = EBhUndefined;
    if (resources.ARM_shader_framebuffer_fetch)
        extBehavior["GL_ARM_shader_framebuffer_fetch"] = EBhUndefined;
}

void ResetExtensionBehavior(TExtensionBehavior &extBehavior)
{
    for (auto ext_iter = extBehavior.begin();
         ext_iter != extBehavior.end();
         ++ext_iter)
    {
        ext_iter->second = EBhUndefined;
    }
}
