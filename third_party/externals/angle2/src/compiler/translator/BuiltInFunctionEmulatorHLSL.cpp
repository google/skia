//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "angle_gl.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/BuiltInFunctionEmulatorHLSL.h"
#include "compiler/translator/SymbolTable.h"

void InitBuiltInFunctionEmulatorForHLSL(BuiltInFunctionEmulator *emu)
{
    TType *float1 = new TType(EbtFloat);
    TType *float2 = new TType(EbtFloat, 2);
    TType *float3 = new TType(EbtFloat, 3);
    TType *float4 = new TType(EbtFloat, 4);

    emu->addEmulatedFunction(EOpMod, float1, float1,
        "float webgl_mod_emu(float x, float y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float2, float2,
        "float2 webgl_mod_emu(float2 x, float2 y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float2, float1,
        "float2 webgl_mod_emu(float2 x, float y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float3, float3,
        "float3 webgl_mod_emu(float3 x, float3 y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float3, float1,
        "float3 webgl_mod_emu(float3 x, float y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float4, float4,
        "float4 webgl_mod_emu(float4 x, float4 y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpMod, float4, float1,
        "float4 webgl_mod_emu(float4 x, float y)\n"
        "{\n"
        "    return x - y * floor(x / y);\n"
        "}\n"
        "\n");

    emu->addEmulatedFunction(EOpFaceForward, float1, float1, float1,
        "float webgl_faceforward_emu(float N, float I, float Nref)\n"
        "{\n"
        "    if(dot(Nref, I) >= 0)\n"
        "    {\n"
        "        return -N;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        return N;\n"
        "    }\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpFaceForward, float2, float2, float2,
        "float2 webgl_faceforward_emu(float2 N, float2 I, float2 Nref)\n"
        "{\n"
        "    if(dot(Nref, I) >= 0)\n"
        "    {\n"
        "        return -N;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        return N;\n"
        "    }\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpFaceForward, float3, float3, float3,
        "float3 webgl_faceforward_emu(float3 N, float3 I, float3 Nref)\n"
        "{\n"
        "    if(dot(Nref, I) >= 0)\n"
        "    {\n"
        "        return -N;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        return N;\n"
        "    }\n"
        "}\n"
        "\n");
    emu->addEmulatedFunction(EOpFaceForward, float4, float4, float4,
        "float4 webgl_faceforward_emu(float4 N, float4 I, float4 Nref)\n"
        "{\n"
        "    if(dot(Nref, I) >= 0)\n"
        "    {\n"
        "        return -N;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        return N;\n"
        "    }\n"
        "}\n"
        "\n");

    emu->addEmulatedFunction(EOpAtan, float1, float1,
        "float webgl_atan_emu(float y, float x)\n"
        "{\n"
        "    if(x == 0 && y == 0) x = 1;\n"   // Avoid producing a NaN
        "    return atan2(y, x);\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtan, float2, float2,
        "float2 webgl_atan_emu(float2 y, float2 x)\n"
        "{\n"
        "    if(x[0] == 0 && y[0] == 0) x[0] = 1;\n"
        "    if(x[1] == 0 && y[1] == 0) x[1] = 1;\n"
        "    return float2(atan2(y[0], x[0]), atan2(y[1], x[1]));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtan, float3, float3,
        "float3 webgl_atan_emu(float3 y, float3 x)\n"
        "{\n"
        "    if(x[0] == 0 && y[0] == 0) x[0] = 1;\n"
        "    if(x[1] == 0 && y[1] == 0) x[1] = 1;\n"
        "    if(x[2] == 0 && y[2] == 0) x[2] = 1;\n"
        "    return float3(atan2(y[0], x[0]), atan2(y[1], x[1]), atan2(y[2], x[2]));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtan, float4, float4,
        "float4 webgl_atan_emu(float4 y, float4 x)\n"
        "{\n"
        "    if(x[0] == 0 && y[0] == 0) x[0] = 1;\n"
        "    if(x[1] == 0 && y[1] == 0) x[1] = 1;\n"
        "    if(x[2] == 0 && y[2] == 0) x[2] = 1;\n"
        "    if(x[3] == 0 && y[3] == 0) x[3] = 1;\n"
        "    return float4(atan2(y[0], x[0]), atan2(y[1], x[1]), atan2(y[2], x[2]), atan2(y[3], x[3]));\n"
        "}\n");

    emu->addEmulatedFunction(EOpAsinh, float1,
        "float webgl_asinh_emu(in float x) {\n"
        "    return log(x + sqrt(pow(x, 2.0) + 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAsinh, float2,
        "float2 webgl_asinh_emu(in float2 x) {\n"
        "    return log(x + sqrt(pow(x, 2.0) + 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAsinh, float3,
        "float3 webgl_asinh_emu(in float3 x) {\n"
        "    return log(x + sqrt(pow(x, 2.0) + 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAsinh, float4,
        "float4 webgl_asinh_emu(in float4 x) {\n"
        "    return log(x + sqrt(pow(x, 2.0) + 1.0));\n"
        "}\n");

    emu->addEmulatedFunction(EOpAcosh, float1,
        "float webgl_acosh_emu(in float x) {\n"
        "    return log(x + sqrt(x + 1.0) * sqrt(x - 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAcosh, float2,
        "float2 webgl_acosh_emu(in float2 x) {\n"
        "    return log(x + sqrt(x + 1.0) * sqrt(x - 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAcosh, float3,
        "float3 webgl_acosh_emu(in float3 x) {\n"
        "    return log(x + sqrt(x + 1.0) * sqrt(x - 1.0));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAcosh, float4,
        "float4 webgl_acosh_emu(in float4 x) {\n"
        "    return log(x + sqrt(x + 1.0) * sqrt(x - 1.0));\n"
        "}\n");

    emu->addEmulatedFunction(EOpAtanh, float1,
        "float webgl_atanh_emu(in float x) {\n"
        "    return 0.5 * log((1.0 + x) / (1.0 - x));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtanh, float2,
        "float2 webgl_atanh_emu(in float2 x) {\n"
        "    return 0.5 * log((1.0 + x) / (1.0 - x));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtanh, float3,
        "float3 webgl_atanh_emu(in float3 x) {\n"
        "    return 0.5 * log((1.0 + x) / (1.0 - x));\n"
        "}\n");
    emu->addEmulatedFunction(EOpAtanh, float4,
        "float4 webgl_atanh_emu(in float4 x) {\n"
        "    return 0.5 * log((1.0 + x) / (1.0 - x));\n"
        "}\n");

    emu->addEmulatedFunction(EOpRoundEven, float1,
        "float webgl_roundEven_emu(in float x) {\n"
        "    return (frac(x) == 0.5 && trunc(x) % 2.0 == 0.0) ? trunc(x) : round(x);\n"
        "}\n");
    emu->addEmulatedFunction(EOpRoundEven, float2,
        "float2 webgl_roundEven_emu(in float2 x) {\n"
        "    float2 v;\n"
        "    v[0] = (frac(x[0]) == 0.5 && trunc(x[0]) % 2.0 == 0.0) ? trunc(x[0]) : round(x[0]);\n"
        "    v[1] = (frac(x[1]) == 0.5 && trunc(x[1]) % 2.0 == 0.0) ? trunc(x[1]) : round(x[1]);\n"
        "    return v;\n"
        "}\n");
    emu->addEmulatedFunction(EOpRoundEven, float3,
        "float3 webgl_roundEven_emu(in float3 x) {\n"
        "    float3 v;\n"
        "    v[0] = (frac(x[0]) == 0.5 && trunc(x[0]) % 2.0 == 0.0) ? trunc(x[0]) : round(x[0]);\n"
        "    v[1] = (frac(x[1]) == 0.5 && trunc(x[1]) % 2.0 == 0.0) ? trunc(x[1]) : round(x[1]);\n"
        "    v[2] = (frac(x[2]) == 0.5 && trunc(x[2]) % 2.0 == 0.0) ? trunc(x[2]) : round(x[2]);\n"
        "    return v;\n"
        "}\n");
    emu->addEmulatedFunction(EOpRoundEven, float4,
        "float4 webgl_roundEven_emu(in float4 x) {\n"
        "    float4 v;\n"
        "    v[0] = (frac(x[0]) == 0.5 && trunc(x[0]) % 2.0 == 0.0) ? trunc(x[0]) : round(x[0]);\n"
        "    v[1] = (frac(x[1]) == 0.5 && trunc(x[1]) % 2.0 == 0.0) ? trunc(x[1]) : round(x[1]);\n"
        "    v[2] = (frac(x[2]) == 0.5 && trunc(x[2]) % 2.0 == 0.0) ? trunc(x[2]) : round(x[2]);\n"
        "    v[3] = (frac(x[3]) == 0.5 && trunc(x[3]) % 2.0 == 0.0) ? trunc(x[3]) : round(x[3]);\n"
        "    return v;\n"
        "}\n");

    emu->addEmulatedFunction(EOpPackSnorm2x16, float2,
        "int webgl_toSnorm(in float x) {\n"
        "    return int(round(clamp(x, -1.0, 1.0) * 32767.0));\n"
        "}\n"
        "\n"
        "uint webgl_packSnorm2x16_emu(in float2 v) {\n"
        "    int x = webgl_toSnorm(v.x);\n"
        "    int y = webgl_toSnorm(v.y);\n"
        "    return (asuint(y) << 16) | (asuint(x) & 0xffffu);\n"
        "}\n");
    emu->addEmulatedFunction(EOpPackUnorm2x16, float2,
        "uint webgl_toUnorm(in float x) {\n"
        "    return uint(round(clamp(x, 0.0, 1.0) * 65535.0));\n"
        "}\n"
        "\n"
        "uint webgl_packUnorm2x16_emu(in float2 v) {\n"
        "    uint x = webgl_toUnorm(v.x);\n"
        "    uint y = webgl_toUnorm(v.y);\n"
        "    return (y << 16) | x;\n"
        "}\n");
    emu->addEmulatedFunction(EOpPackHalf2x16, float2,
        "uint webgl_packHalf2x16_emu(in float2 v) {\n"
        "    uint x = f32tof16(v.x);\n"
        "    uint y = f32tof16(v.y);\n"
        "    return (y << 16) | x;\n"
        "}\n");

    TType *uint1 = new TType(EbtUInt);

    emu->addEmulatedFunction(EOpUnpackSnorm2x16, uint1,
        "float webgl_fromSnorm(in uint x) {\n"
        "    int xi = asint(x & 0x7fffu) - asint(x & 0x8000u);\n"
        "    return clamp(float(xi) / 32767.0, -1.0, 1.0);\n"
        "}\n"
        "\n"
        "float2 webgl_unpackSnorm2x16_emu(in uint u) {\n"
        "    uint y = (u >> 16);\n"
        "    uint x = u;\n"
        "    return float2(webgl_fromSnorm(x), webgl_fromSnorm(y));\n"
        "}\n");
    emu->addEmulatedFunction(EOpUnpackUnorm2x16, uint1,
        "float webgl_fromUnorm(in uint x) {\n"
        "    return float(x) / 65535.0;\n"
        "}\n"
        "\n"
        "float2 webgl_unpackUnorm2x16_emu(in uint u) {\n"
        "    uint y = (u >> 16);\n"
        "    uint x = u & 0xffffu;\n"
        "    return float2(webgl_fromUnorm(x), webgl_fromUnorm(y));\n"
        "}\n");
    emu->addEmulatedFunction(EOpUnpackHalf2x16, uint1,
        "float2 webgl_unpackHalf2x16_emu(in uint u) {\n"
        "    uint y = (u >> 16);\n"
        "    uint x = u & 0xffffu;\n"
        "    return float2(f16tof32(x), f16tof32(y));\n"
        "}\n");

    // The matrix resulting from outer product needs to be transposed
    // (matrices are stored as transposed to simplify element access in HLSL).
    // So the function should return transpose(c * r) where c is a column vector
    // and r is a row vector. This can be simplified by using the following
    // formula:
    //   transpose(c * r) = transpose(r) * transpose(c)
    // transpose(r) and transpose(c) are in a sense free, since to get the
    // transpose of r, we simply can build a column matrix out of the original
    // vector instead of a row matrix.
    emu->addEmulatedFunction(EOpOuterProduct, float2, float2,
        "float2x2 webgl_outerProduct_emu(in float2 c, in float2 r) {\n"
        "    return mul(float2x1(r), float1x2(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float3, float3,
        "float3x3 webgl_outerProduct_emu(in float3 c, in float3 r) {\n"
        "    return mul(float3x1(r), float1x3(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float4, float4,
        "float4x4 webgl_outerProduct_emu(in float4 c, in float4 r) {\n"
        "    return mul(float4x1(r), float1x4(c));\n"
        "}\n");

    emu->addEmulatedFunction(EOpOuterProduct, float3, float2,
        "float2x3 webgl_outerProduct_emu(in float3 c, in float2 r) {\n"
        "    return mul(float2x1(r), float1x3(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float2, float3,
        "float3x2 webgl_outerProduct_emu(in float2 c, in float3 r) {\n"
        "    return mul(float3x1(r), float1x2(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float4, float2,
        "float2x4 webgl_outerProduct_emu(in float4 c, in float2 r) {\n"
        "    return mul(float2x1(r), float1x4(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float2, float4,
        "float4x2 webgl_outerProduct_emu(in float2 c, in float4 r) {\n"
        "    return mul(float4x1(r), float1x2(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float4, float3,
        "float3x4 webgl_outerProduct_emu(in float4 c, in float3 r) {\n"
        "    return mul(float3x1(r), float1x4(c));\n"
        "}\n");
    emu->addEmulatedFunction(EOpOuterProduct, float3, float4,
        "float4x3 webgl_outerProduct_emu(in float3 c, in float4 r) {\n"
        "    return mul(float4x1(r), float1x3(c));\n"
        "}\n");

    TType *mat2 = new TType(EbtFloat, 2, 2);
    TType *mat3 = new TType(EbtFloat, 3, 3);
    TType *mat4 = new TType(EbtFloat, 4, 4);

    // Remember here that the parameter matrix is actually the transpose
    // of the matrix that we're trying to invert, and the resulting matrix
    // should also be the transpose of the inverse.

    // When accessing the parameter matrix with m[a][b] it can be thought of so
    // that a is the column and b is the row of the matrix that we're inverting.

    // We calculate the inverse as the adjugate matrix divided by the
    // determinant of the matrix being inverted. However, as the result needs
    // to be transposed, we actually use of the transpose of the adjugate matrix
    // which happens to be the cofactor matrix. That's stored in "cof".

    // We don't need to care about divide-by-zero since results are undefined
    // for singular or poorly-conditioned matrices.

    emu->addEmulatedFunction(EOpInverse, mat2,
        "float2x2 webgl_inverse_emu(in float2x2 m) {\n"
        "    float2x2 cof = { m[1][1], -m[0][1], -m[1][0], m[0][0] };\n"
        "    return cof / determinant(transpose(m));\n"
        "}\n");

    // cofAB is the cofactor for column A and row B.

    emu->addEmulatedFunction(EOpInverse, mat3,
        "float3x3 webgl_inverse_emu(in float3x3 m) {\n"
        "    float cof00 = m[1][1] * m[2][2] - m[2][1] * m[1][2];\n"
        "    float cof01 = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]);\n"
        "    float cof02 = m[1][0] * m[2][1] - m[2][0] * m[1][1];\n"
        "    float cof10 = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]);\n"
        "    float cof11 = m[0][0] * m[2][2] - m[2][0] * m[0][2];\n"
        "    float cof12 = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]);\n"
        "    float cof20 = m[0][1] * m[1][2] - m[1][1] * m[0][2];\n"
        "    float cof21 = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]);\n"
        "    float cof22 = m[0][0] * m[1][1] - m[1][0] * m[0][1];\n"
        "    float3x3 cof = { cof00, cof10, cof20, cof01, cof11, cof21, cof02, cof12, cof22 };\n"
        "    return cof / determinant(transpose(m));\n"
        "}\n");

    emu->addEmulatedFunction(EOpInverse, mat4,
        "float4x4 webgl_inverse_emu(in float4x4 m) {\n"
        "    float cof00 = m[1][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[1][3] + m[3][1] * m[1][2] * m[2][3]"
                       " - m[1][1] * m[3][2] * m[2][3] - m[2][1] * m[1][2] * m[3][3] - m[3][1] * m[2][2] * m[1][3];\n"
        "    float cof01 = -(m[1][0] * m[2][2] * m[3][3] + m[2][0] * m[3][2] * m[1][3] + m[3][0] * m[1][2] * m[2][3]"
                       " - m[1][0] * m[3][2] * m[2][3] - m[2][0] * m[1][2] * m[3][3] - m[3][0] * m[2][2] * m[1][3]);\n"
        "    float cof02 = m[1][0] * m[2][1] * m[3][3] + m[2][0] * m[3][1] * m[1][3] + m[3][0] * m[1][1] * m[2][3]"
                       " - m[1][0] * m[3][1] * m[2][3] - m[2][0] * m[1][1] * m[3][3] - m[3][0] * m[2][1] * m[1][3];\n"
        "    float cof03 = -(m[1][0] * m[2][1] * m[3][2] + m[2][0] * m[3][1] * m[1][2] + m[3][0] * m[1][1] * m[2][2]"
                       " - m[1][0] * m[3][1] * m[2][2] - m[2][0] * m[1][1] * m[3][2] - m[3][0] * m[2][1] * m[1][2]);\n"
        "    float cof10 = -(m[0][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[2][3]"
                       " - m[0][1] * m[3][2] * m[2][3] - m[2][1] * m[0][2] * m[3][3] - m[3][1] * m[2][2] * m[0][3]);\n"
        "    float cof11 = m[0][0] * m[2][2] * m[3][3] + m[2][0] * m[3][2] * m[0][3] + m[3][0] * m[0][2] * m[2][3]"
                       " - m[0][0] * m[3][2] * m[2][3] - m[2][0] * m[0][2] * m[3][3] - m[3][0] * m[2][2] * m[0][3];\n"
        "    float cof12 = -(m[0][0] * m[2][1] * m[3][3] + m[2][0] * m[3][1] * m[0][3] + m[3][0] * m[0][1] * m[2][3]"
                       " - m[0][0] * m[3][1] * m[2][3] - m[2][0] * m[0][1] * m[3][3] - m[3][0] * m[2][1] * m[0][3]);\n"
        "    float cof13 = m[0][0] * m[2][1] * m[3][2] + m[2][0] * m[3][1] * m[0][2] + m[3][0] * m[0][1] * m[2][2]"
                       " - m[0][0] * m[3][1] * m[2][2] - m[2][0] * m[0][1] * m[3][2] - m[3][0] * m[2][1] * m[0][2];\n"
        "    float cof20 = m[0][1] * m[1][2] * m[3][3] + m[1][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[1][3]"
                       " - m[0][1] * m[3][2] * m[1][3] - m[1][1] * m[0][2] * m[3][3] - m[3][1] * m[1][2] * m[0][3];\n"
        "    float cof21 = -(m[0][0] * m[1][2] * m[3][3] + m[1][0] * m[3][2] * m[0][3] + m[3][0] * m[0][2] * m[1][3]"
                       " - m[0][0] * m[3][2] * m[1][3] - m[1][0] * m[0][2] * m[3][3] - m[3][0] * m[1][2] * m[0][3]);\n"
        "    float cof22 = m[0][0] * m[1][1] * m[3][3] + m[1][0] * m[3][1] * m[0][3] + m[3][0] * m[0][1] * m[1][3]"
                       " - m[0][0] * m[3][1] * m[1][3] - m[1][0] * m[0][1] * m[3][3] - m[3][0] * m[1][1] * m[0][3];\n"
        "    float cof23 = -(m[0][0] * m[1][1] * m[3][2] + m[1][0] * m[3][1] * m[0][2] + m[3][0] * m[0][1] * m[1][2]"
                       " - m[0][0] * m[3][1] * m[1][2] - m[1][0] * m[0][1] * m[3][2] - m[3][0] * m[1][1] * m[0][2]);\n"
        "    float cof30 = -(m[0][1] * m[1][2] * m[2][3] + m[1][1] * m[2][2] * m[0][3] + m[2][1] * m[0][2] * m[1][3]"
                       " - m[0][1] * m[2][2] * m[1][3] - m[1][1] * m[0][2] * m[2][3] - m[2][1] * m[1][2] * m[0][3]);\n"
        "    float cof31 = m[0][0] * m[1][2] * m[2][3] + m[1][0] * m[2][2] * m[0][3] + m[2][0] * m[0][2] * m[1][3]"
                       " - m[0][0] * m[2][2] * m[1][3] - m[1][0] * m[0][2] * m[2][3] - m[2][0] * m[1][2] * m[0][3];\n"
        "    float cof32 = -(m[0][0] * m[1][1] * m[2][3] + m[1][0] * m[2][1] * m[0][3] + m[2][0] * m[0][1] * m[1][3]"
                       " - m[0][0] * m[2][1] * m[1][3] - m[1][0] * m[0][1] * m[2][3] - m[2][0] * m[1][1] * m[0][3]);\n"
        "    float cof33 = m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2] + m[2][0] * m[0][1] * m[1][2]"
                       " - m[0][0] * m[2][1] * m[1][2] - m[1][0] * m[0][1] * m[2][2] - m[2][0] * m[1][1] * m[0][2];\n"
        "    float4x4 cof = { cof00, cof10, cof20, cof30, cof01, cof11, cof21, cof31,"
                            " cof02, cof12, cof22, cof32, cof03, cof13, cof23, cof33 };\n"
        "    return cof / determinant(transpose(m));\n"
        "}\n");

    TType *bool1 = new TType(EbtBool);
    TType *bool2 = new TType(EbtBool, 2);
    TType *bool3 = new TType(EbtBool, 3);
    TType *bool4 = new TType(EbtBool, 4);

    // Emulate ESSL3 variant of mix that takes last argument as boolean vector.
    // genType mix (genType x, genType y, genBType a): Selects which vector each returned component comes from.
    // For a component of 'a' that is false, the corresponding component of 'x' is returned.For a component of 'a' that is true,
    // the corresponding component of 'y' is returned.
    emu->addEmulatedFunction(EOpMix, float1, float1, bool1,
        "float webgl_mix_emu(float x, float y, bool a)\n"
        "{\n"
        "    return a ? y : x;\n"
        "}\n");
    emu->addEmulatedFunction(EOpMix, float2, float2, bool2,
        "float2 webgl_mix_emu(float2 x, float2 y, bool2 a)\n"
        "{\n"
        "    return a ? y : x;\n"
        "}\n");
    emu->addEmulatedFunction(EOpMix, float3, float3, bool3,
        "float3 webgl_mix_emu(float3 x, float3 y, bool3 a)\n"
        "{\n"
        "    return a ? y : x;\n"
        "}\n");
    emu->addEmulatedFunction(EOpMix, float4, float4, bool4,
        "float4 webgl_mix_emu(float4 x, float4 y, bool4 a)\n"
        "{\n"
        "    return a ? y : x;\n"
        "}\n");

}
