//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "angle_gl.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/BuiltInFunctionEmulatorGLSL.h"
#include "compiler/translator/Cache.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/VersionGLSL.h"

void InitBuiltInFunctionEmulatorForGLSLWorkarounds(BuiltInFunctionEmulator *emu, sh::GLenum shaderType)
{
    // we use macros here instead of function definitions to work around more GLSL
    // compiler bugs, in particular on NVIDIA hardware on Mac OSX. Macros are
    // problematic because if the argument has side-effects they will be repeatedly
    // evaluated. This is unlikely to show up in real shaders, but is something to
    // consider.

    const TType *float1 = TCache::getType(EbtFloat);
    const TType *float2 = TCache::getType(EbtFloat, 2);
    const TType *float3 = TCache::getType(EbtFloat, 3);
    const TType *float4 = TCache::getType(EbtFloat, 4);

    if (shaderType == GL_FRAGMENT_SHADER)
    {
        emu->addEmulatedFunction(EOpCos, float1, "webgl_emu_precision float webgl_cos_emu(webgl_emu_precision float a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float2, "webgl_emu_precision vec2 webgl_cos_emu(webgl_emu_precision vec2 a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float3, "webgl_emu_precision vec3 webgl_cos_emu(webgl_emu_precision vec3 a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float4, "webgl_emu_precision vec4 webgl_cos_emu(webgl_emu_precision vec4 a) { return cos(a); }");
    }
    emu->addEmulatedFunction(EOpDistance, float1, float1, "#define webgl_distance_emu(x, y) ((x) >= (y) ? (x) - (y) : (y) - (x))");
    emu->addEmulatedFunction(EOpDot, float1, float1, "#define webgl_dot_emu(x, y) ((x) * (y))");
    emu->addEmulatedFunction(EOpLength, float1, "#define webgl_length_emu(x) ((x) >= 0.0 ? (x) : -(x))");
    emu->addEmulatedFunction(EOpNormalize, float1, "#define webgl_normalize_emu(x) ((x) == 0.0 ? 0.0 : ((x) > 0.0 ? 1.0 : -1.0))");
    emu->addEmulatedFunction(EOpReflect, float1, float1, "#define webgl_reflect_emu(I, N) ((I) - 2.0 * (N) * (I) * (N))");
}

// Emulate built-in functions missing from GLSL 1.30 and higher
void InitBuiltInFunctionEmulatorForGLSLMissingFunctions(BuiltInFunctionEmulator *emu, sh::GLenum shaderType,
                                                        int targetGLSLVersion)
{
    // Emulate packSnorm2x16, packHalf2x16, unpackSnorm2x16, and unpackHalf2x16 (GLSL 4.20)
    // by using floatBitsToInt, floatBitsToUint, intBitsToFloat, and uintBitsToFloat (GLSL 3.30).
    if (targetGLSLVersion >= GLSL_VERSION_330 && targetGLSLVersion < GLSL_VERSION_420)
    {
        const TType *float2 = TCache::getType(EbtFloat, 2);
        const TType *uint1 = TCache::getType(EbtUInt);

        emu->addEmulatedFunction(EOpPackSnorm2x16, float2,
            "uint webgl_packSnorm2x16_emu(vec2 v)\n"
            "{\n"
            "    int x = int(round(clamp(v.x, -1.0, 1.0) * 32767.0));\n"
            "    int y = int(round(clamp(v.y, -1.0, 1.0) * 32767.0));\n"
            "    return uint((y << 16) | (x & 0xFFFF));\n"
            "}\n");
        emu->addEmulatedFunction(EOpUnpackSnorm2x16, uint1,
            "float webgl_fromSnorm(uint x)\n"
            "{\n"
            "    int xi = (int(x) & 0x7FFF) - (int(x) & 0x8000);\n"
            "    return clamp(float(xi) / 32767.0, -1.0, 1.0);\n"
            "}\n"
            "\n"
            "vec2 webgl_unpackSnorm2x16_emu(uint u)\n"
            "{\n"
            "    uint y = (u >> 16);\n"
            "    uint x = u;\n"
            "    return vec2(webgl_fromSnorm(x), webgl_fromSnorm(y));\n"
            "}\n");
        // Functions uint webgl_f32tof16(float val) and float webgl_f16tof32(uint val) are
        // based on the OpenGL redbook Appendix Session "Floating-Point Formats Used in OpenGL".
        emu->addEmulatedFunction(EOpPackHalf2x16, float2,
            "uint webgl_f32tof16(float val)\n"
            "{\n"
            "    uint f32 = floatBitsToUint(val);\n"
            "    uint f16 = 0u;\n"
            "    uint sign = (f32 >> 16) & 0x8000u;\n"
            "    int exponent = int((f32 >> 23) & 0xFFu) - 127;\n"
            "    uint mantissa = f32 & 0x007FFFFFu;\n"
            "    if (exponent == 128)\n"
            "    {\n"
            "        // Infinity or NaN\n"
            "        // NaN bits that are masked out by 0x3FF get discarded.\n"
            "        // This can turn some NaNs to infinity, but this is allowed by the spec.\n"
            "        f16 = sign | (0x1Fu << 10);\n"
            "        f16 |= (mantissa & 0x3FFu);\n"
            "    }\n"
            "    else if (exponent > 15)\n"
            "    {\n"
            "        // Overflow - flush to Infinity\n"
            "        f16 = sign | (0x1Fu << 10);\n"
            "    }\n"
            "    else if (exponent > -15)\n"
            "    {\n"
            "        // Representable value\n"
            "        exponent += 15;\n"
            "        mantissa >>= 13;\n"
            "        f16 = sign | uint(exponent << 10) | mantissa;\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "        f16 = sign;\n"
            "    }\n"
            "    return f16;\n"
            "}\n"
            "\n"
            "uint webgl_packHalf2x16_emu(vec2 v)\n"
            "{\n"
            "    uint x = webgl_f32tof16(v.x);\n"
            "    uint y = webgl_f32tof16(v.y);\n"
            "    return (y << 16) | x;\n"
            "}\n");
        emu->addEmulatedFunction(EOpUnpackHalf2x16, uint1,
            "float webgl_f16tof32(uint val)\n"
            "{\n"
            "    uint sign = (val & 0x8000u) << 16;\n"
            "    int exponent = int((val & 0x7C00u) >> 10);\n"
            "    uint mantissa = val & 0x03FFu;\n"
            "    float f32 = 0.0;\n"
            "    if(exponent == 0)\n"
            "    {\n"
            "        if (mantissa != 0u)\n"
            "        {\n"
            "            const float scale = 1.0 / (1 << 24);\n"
            "            f32 = scale * mantissa;\n"
            "        }\n"
            "    }\n"
            "    else if (exponent == 31)\n"
            "    {\n"
            "        return uintBitsToFloat(sign | 0x7F800000u | mantissa);\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "         exponent -= 15;\n"
            "         float scale;\n"
            "         if(exponent < 0)\n"
            "         {\n"
            "             scale = 1.0 / (1 << -exponent);\n"
            "         }\n"
            "         else\n"
            "         {\n"
            "             scale = 1 << exponent;\n"
            "         }\n"
            "         float decimal = 1.0 + float(mantissa) / float(1 << 10);\n"
            "         f32 = scale * decimal;\n"
            "    }\n"
            "\n"
            "    if (sign != 0u)\n"
            "    {\n"
            "        f32 = -f32;\n"
            "    }\n"
            "\n"
            "    return f32;\n"
            "}\n"
            "\n"
            "vec2 webgl_unpackHalf2x16_emu(uint u)\n"
            "{\n"
            "    uint y = (u >> 16);\n"
            "    uint x = u & 0xFFFFu;\n"
            "    return vec2(webgl_f16tof32(x), webgl_f16tof32(y));\n"
            "}\n");
    }
}
