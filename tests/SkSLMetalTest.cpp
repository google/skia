/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

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

DEF_TEST(SkSLMetal2x2MatrixCopyFromFloat2x2, r) {
    test(r, R"__SkSL__(
void main() {
  float2x2 m1 = float2x2(float2(1, 2), float2(3, 4));
  float2x2 m2 = m1;
  float2x2 m3 = float2x2(m1);
  sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0]));
})__SkSL__",
             *SkSL::ShaderCapsFactory::Default(),
    R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4((float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0] + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0]) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0]);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetal2x2MatrixCopyFromConstantPropagatedFloat4, r) {
    test(r, R"__SkSL__(
void main() {
  float2x2 m1 = float2x2(float4(1, 2, 3, 4));
  float2x2 m2 = m1;
  float2x2 m3 = float2x2(m1);
  sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0]));
})__SkSL__",
             *SkSL::ShaderCapsFactory::Default(),
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0[0], x0[1]), float2(x0[2], x0[3]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4((float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0] + float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]) + float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalMatrices, r) {
    test(r, R"__SkSL__(
void main() {
  float2x2 m1 = float2x2(float4(1, 2, 3, 4));
  float2x2 m2 = float2x2(float4(0));
  float2x2 m3 = float2x2(m1);
  float2x2 m4 = float2x2(1);
  float2x2 m5 = float2x2(m1[0][0]);
  float2x2 m6 = float2x2(1, 2, 3, 4);
  float2x2 m7 = float2x2(5, float3(6, 7, 8));
  float3x2 m8 = float3x2(float2(1, 2), 3, float3(4, 5, 6));
  float3x3 m9 = float3x3(1);
  float4x4 m10 = float4x4(1);
  float4x4 m11 = float4x4(2);
  sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0] + m4[0][0] + m5[0][0] +
                            m6[0][0] + m7[0][0] + m8[0][0] + m9[0][0] + m10[0][0] + m11[0][0]));
})__SkSL__",
         *SkSL::ShaderCapsFactory::Default(),
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0[0], x0[1]), float2(x0[2], x0[3]));
}
float2x2 float2x2_from_float_float3(float x0, float3 x1) {
    return float2x2(float2(x0, x1[0]), float2(x1[1], x1[2]));
}
float3x2 float3x2_from_float2_float_float3(float2 x0, float x1, float3 x2) {
    return float3x2(float2(x0[0], x0[1]), float2(x1, x2[0]), float2(x2[1], x2[2]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float2x2 m5 = float2x2(float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]);
    _out->sk_FragColor = float4((((((((((float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0] + float2x2_from_float4(float4(0.0))[0][0]) + float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]) + float2x2(1.0)[0][0]) + m5[0][0]) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0]) + float2x2_from_float_float3(5.0, float3(6.0, 7.0, 8.0))[0][0]) + float3x2_from_float2_float_float3(float2(1.0, 2.0), 3.0, float3(4.0, 5.0, 6.0))[0][0]) + float3x3(1.0)[0][0]) + float4x4(1.0)[0][0]) + float4x4(2.0)[0][0]);
    return *_out;
}
)__MSL__");
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
