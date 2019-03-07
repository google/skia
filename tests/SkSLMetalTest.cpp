/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

static void test(skiatest::Reporter* r, const char* src, const SkSL::Program::Settings& settings,
                 const char* expected, SkSL::Program::Inputs* inputs,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, program);
    *inputs = program->fInputs;
    REPORTER_ASSERT(r, compiler.toMetal(*program, &output));
    if (program) {
        SkSL::String skExpected(expected);
        if (output != skExpected) {
            SkDebugf("MSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                     expected, output.c_str());
        }
        REPORTER_ASSERT(r, output == skExpected);
    }
}

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 const char* expected, SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, src, settings, expected, &inputs, kind);
}

DEF_TEST(SkSLMetalHelloWorld, r) {
    test(r,
         "void main() { sk_FragColor = half4(0.75); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#include <metal_stdlib>\n"
         "#include <simd/simd.h>\n"
         "using namespace metal;\n"
         "struct Inputs {\n"
         "};\n"
         "struct Outputs {\n"
         "    float4 sk_FragColor [[color(0)]];\n"
         "};\n"
         "fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {\n"
         "    Outputs _outputStruct;\n"
         "    thread Outputs* _out = &_outputStruct;\n"
         "    _out->sk_FragColor = float4(0.75);\n"
         "    return *_out;\n"
         "}\n");
}

DEF_TEST(SkSLMetalMatrices, r) {
    test(r,
         "void main() {"
         "float2x2 m1 = float2x2(float4(1, 2, 3, 4));"
         "float2x2 m2 = float2x2(float4(0));"
         "float2x2 m3 = float2x2(m1);"
         "float2x2 m4 = float2x2(1);"
         "float2x2 m5 = float2x2(m1[0][0]);"
         "float2x2 m6 = float2x2(1, 2, 3, 4);"
         "float2x2 m7 = float2x2(5, 6, 7, 8);"
         "float3x3 m8 = float3x3(1);"
         "float3x3 m9 = float3x3(2);"
         "float4x4 m10 = float4x4(1);"
         "float4x4 m11 = float4x4(2);"
         "sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0] + m4[0][0] + m5[0][0] + "
                "m6[0][0] + m7[0][0] + m8[0][0] + m9[0][0] + m10[0][0] + m11[0][0]));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#include <metal_stdlib>\n"
         "#include <simd/simd.h>\n"
         "using namespace metal;\n"
         "struct Inputs {\n"
         "};\n"
         "struct Outputs {\n"
         "    float4 sk_FragColor [[color(0)]];\n"
         "};\n"
         "float2x2 float2x2_from_float(float x) {\n"
         "    return float2x2(float2(x, 0), float2(0, x));\n"
         "}\n"
         "float2x2 float2x2_from_float4(float4 v) {\n"
         "    return float2x2(float2(v[0], v[1]), float2(v[2], v[3]));\n"
         "}\n"
         "float2x2 float2x2_from_float(float x) {\n"
         "    return float2x2(float2(x, 0), float2(0, x));\n"
         "}\n"
         "float3x3 float3x3_from_float(float x) {\n"
         "    return float3x3(float3(x, 0, 0), float3(0, x, 0), float3(0, 0, x));\n"
         "}\n"
         "float4x4 float4x4_from_float(float x) {\n"
         "    return float4x4(float4(x, 0, 0, 0), float4(0, x, 0, 0), float4(0, 0, x, 0), float4(0, 0, 0, x));\n"
         "}\n"
         "fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {\n"
         "    Outputs _outputStruct;\n"
         "    thread Outputs* _out = &_outputStruct;\n"
         "    float2x2 m5 = float2x2_from_float(float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]);\n"
         "    _out->sk_FragColor = float4((((((((((float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0] + float2x2_from_float4(float4(0.0))[0][0]) + float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]) + float2x2_from_float(1.0)[0][0]) + m5[0][0]) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0]) + float2x2(float2(5.0, 6.0), float2(7.0, 8.0))[0][0]) + float3x3_from_float(1.0)[0][0]) + float3x3_from_float(2.0)[0][0]) + float4x4_from_float(1.0)[0][0]) + float4x4_from_float(2.0)[0][0]);\n"
         "    return *_out;\n"
         "}\n");
}

DEF_TEST(SkSLMetalConstantSwizzle, r) {
    test(r,
         "void main() {"
         "sk_FragColor = half4(0.5).rgb1;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#include <metal_stdlib>\n"
         "#include <simd/simd.h>\n"
         "using namespace metal;\n"
         "struct Inputs {\n"
         "};\n"
         "struct Outputs {\n"
         "    float4 sk_FragColor [[color(0)]];\n"
         "};\n"
         "fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {\n"
         "    Outputs _outputStruct;\n"
         "    thread Outputs* _out = &_outputStruct;\n"
         "    _out->sk_FragColor = float4(float4(0.5).xyz, 1);\n"
         "    return *_out;\n"
         "}\n");
}
