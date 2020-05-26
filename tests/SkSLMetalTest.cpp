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

DEF_TEST(SkSLMetal_Crbug10280, r) {
    test(r,
R"__SkSL__(
in float4 abcd;
in float4 efgh;
void main()
{
  float4x2 P = float4x2(abcd, efgh);
  sk_Position = float4(P[0].x, P[0].y, P[1].x, P[1].y);
}
)__SkSL__",
         *SkSL::ShaderCapsFactory::Default(),
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 abcd;
    float4 efgh;
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize;
};


vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4x2 P = float4x2(_in.abcd, _in.efgh);
    _out->sk_Position = float4(P[0].x, P[0].y, P[1].x, P[1].y);
    _out->sk_Position.y = -_out->sk_Position.y;
    return *_out;
}
)__MSL__",
         SkSL::Program::kVertex_Kind);
}

DEF_TEST(SkSLMetalScalarMerge, r) {
    test(r,
R"__SkSL__(
in float2 ab;
in float2 cd;
in float e;
in float f;
in float g;
in float h;
in float3 ijk;
void main()
{
  float4x4 P = float4x4(float4(ab.xy, ab.x, cd.y), float4(e, f, g, h), float4(ijk.yyyy), float4(ab.x, ijk.zz, ab.y));
  float3x3 Q = float3x3(P[0].xyz, float3(P[1].x, P[1].y, P[1].z), float4(P[2].wzyx).wzy);
  sk_Position = float4(P[0].x, P[1].y, Q[0].x, Q[1].y);
}
)__SkSL__",
         *SkSL::ShaderCapsFactory::Default(),
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 ab;
    float2 cd;
    float e;
    float f;
    float g;
    float h;
    float3 ijk;
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize;
};







vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4x4 P = float4x4(float4(_in.ab, _in.ab.x, _in.cd.y), float4(_in.e, _in.f, _in.g, _in.h), _in.ijk.yyyy, float4(_in.ab.x, _in.ijk.zz, _in.ab.y));
    float3x3 Q = float3x3(P[0].xyz, float3(P[1].x, P[1].y, P[1].z), P[2].xyz);
    _out->sk_Position = float4(P[0].x, P[1].y, Q[0].x, Q[1].y);
    _out->sk_Position.y = -_out->sk_Position.y;
    return *_out;
}
)__MSL__",
         SkSL::Program::kVertex_Kind);
}
