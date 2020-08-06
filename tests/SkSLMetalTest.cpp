/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

static void test(skiatest::Reporter* r, const SkSL::Program::Settings& settings, const char* src,
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

static void test(skiatest::Reporter* r, const GrShaderCaps& caps, const char* src,
                 const char* expected, SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, settings, src, expected, &inputs, kind);
}

DEF_TEST(SkSLMetalHelloWorld, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
         "void main() { sk_FragColor = half4(0.75); }",
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
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
  float2x2 m1 = float2x2(float2(1, 2), float2(3, 4));
  float2x2 m2 = m1;
  float2x2 m3 = float2x2(m1);
  sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0]));
})__SkSL__",
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
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
  float2x2 m1 = float2x2(float4(1, 2, 3, 4));
  float2x2 m2 = m1;
  float2x2 m3 = float2x2(m1);
  sk_FragColor = half4(half(m1[0][0] + m2[0][0] + m3[0][0]));
})__SkSL__",
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

DEF_TEST(SkSLMetalCastMat2x2ToMat3x3, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
float3x3 a = float3x3(1);
float3x3 b = float3x3(float2x2(1));
sk_FragColor.x = (a[0] == b[0]) ? 0 : 1;
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float3x3 float3x3_from_float2x2(float2x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(0.0, 0.0, 1.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(all(float3x3(1.0)[0] == float3x3_from_float2x2(float2x2(1.0))[0]) ? 0 : 1);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalCastMat2x3ToMat4x4, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
float4x4 a = float4x4(6);
float4x4 b = float4x4(float2x3(7));
sk_FragColor.x = (a[1] == b[1]) ? 0 : 1;
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4x4 float4x4_from_float2x3(float2x3 x0) {
    return float4x4(float4(x0[0].xyz, 0.0), float4(x0[1].xyz, 0.0), float4(0.0, 0.0, 1.0, 0.0), float4(0.0, 0.0, 0.0, 1.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(all(float4x4(6.0)[1] == float4x4_from_float2x3(float2x3(7.0))[1]) ? 0 : 1);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalCastMat4x4ToMat3x4, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
float3x4 a = float3x4(1);
float3x4 b = float3x4(float4x4(1));
sk_FragColor.x = (a[0] == b[0]) ? 0 : 1;
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float3x4 float3x4_from_float4x4(float4x4 x0) {
    return float3x4(float4(x0[0].xyzw), float4(x0[1].xyzw), float4(x0[2].xyzw));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(all(float3x4(1.0)[0] == float3x4_from_float4x4(float4x4(1.0))[0]) ? 0 : 1);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalCastMat4x4ToMat4x3, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
void main() {
float4x3 a = float4x3(1);
float4x3 b = float4x3(float4x4(1));
sk_FragColor.x = (a[0] == b[0]) ? 0 : 1;
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4x3 float4x3_from_float4x4(float4x4 x0) {
    return float4x3(float3(x0[0].xyz), float3(x0[1].xyz), float3(x0[2].xyz), float3(x0[3].xyz));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(all(float4x3(1.0)[0] == float4x3_from_float4x4(float4x4(1.0))[0]) ? 0 : 1);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalMatrices, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
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
    test(r, *SkSL::ShaderCapsFactory::Default(),
         "void main() {"
         "sk_FragColor = half4(0.5).rgb1;"
         "}",
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

DEF_TEST(SkSLMetalNumericGlobals, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
half attr1;
int attr2 = 123;
float attr3;
half4 attr4 = half4(4, 5, 6, 7);
void main()
{
    sk_FragColor = half4(attr1, attr2, half(attr3), attr4.x);
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float attr1;
    int attr2;
    float attr3;
    float4 attr4;
};



fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{{}, 123, {}, float4(4.0, 5.0, 6.0, 7.0)};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_globals->attr1, float(_globals->attr2), _globals->attr3, _globals->attr4.x);
    return *_out;
}
)__MSL__");
}

DEF_TEST(SkSLMetalSamplerGlobals, r) {
    test(r, *SkSL::ShaderCapsFactory::Default(),
R"__SkSL__(
layout(binding=1) uniform sampler2D texA;
layout(binding=0) uniform sampler2D texB;
void main()
{
    sk_FragColor = sample(texA, half2(0)) * sample(texB, half2(0));
}
)__SkSL__",
R"__MSL__(#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<float> texA;
    sampler texASmplr;
    texture2d<float> texB;
    sampler texBSmplr;
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<float> texA[[texture(1)]], sampler texASmplr[[sampler(1)]], texture2d<float> texB[[texture(0)]], sampler texBSmplr[[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{texA, texASmplr, texB, texBSmplr};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = _globals->texA.sample(_globals->texASmplr, float2(0.0)) * _globals->texB.sample(_globals->texBSmplr, float2(0.0));
    return *_out;
}
)__MSL__");
}
