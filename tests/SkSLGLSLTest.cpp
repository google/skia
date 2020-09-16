/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

// Note that the optimizer will aggressively kill dead code and substitute constants in place of
// variables, so we have to jump through a few hoops to ensure that the code in these tests has the
// necessary side-effects to remain live. In some cases we rely on the optimizer not (yet) being
// smart enough to optimize around certain constructs; as the optimizer gets smarter it will
// undoubtedly end up breaking some of these tests. That is a good thing, as long as the new code is
// equivalent!

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
    if (program) {
        *inputs = program->fInputs;
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        if (program) {
            SkSL::String skExpected(expected);
            if (output != skExpected) {
                SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                         expected, output.c_str());
            }
            REPORTER_ASSERT(r, output == skExpected);
        }
    }
}

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 const char* expected, SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, src, settings, expected, &inputs, kind);
}

DEF_TEST(SkSLVersion, r) {
    test(r,
         "in float test; void main() { sk_FragColor.r = half(test); }",
         *SkSL::ShaderCapsFactory::Version450Core(),
         "#version 450 core\n"
         "out vec4 sk_FragColor;\n"
         "in float test;\n"
         "void main() {\n"
         "    sk_FragColor.x = test;\n"
         "}\n");
    test(r,
         "in float test; void main() { sk_FragColor.r = half(test); }",
         *SkSL::ShaderCapsFactory::Version110(),
         "#version 110\n"
         "varying float test;\n"
         "void main() {\n"
         "    gl_FragColor.x = test;\n"
         "}\n");
}

DEF_TEST(SkSLUsesPrecisionModifiers, r) {
    test(r,
         "void main() { half x = 0.75; float y = 1; x++; y++;"
         "sk_FragColor.rg = half2(x, half(y)); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    float y = 1.0;\n"
         "    x++;\n"
         "    y++;\n"
         "    sk_FragColor.xy = vec2(x, y);\n"
         "}\n");
    test(r,
         "void main() { half x = 0.75; float y = 1; x++; y++;"
         "sk_FragColor.rg = half2(x, half(y)); }",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    mediump float x = 0.75;\n"
         "    highp float y = 1.0;\n"
         "    x++;\n"
         "    y++;\n"
         "    sk_FragColor.xy = vec2(x, y);\n"
         "}\n");
}

DEF_TEST(SkSLMinAbs, r) {
    test(r,
         "void main() {"
         "half x = -5;"
         "sk_FragColor.r = min(abs(x), 6);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = min(abs(-5.0), 6.0);\n"
         "}\n");

    test(r,
         "void main() {"
         "half x = -5.0;"
         "sk_FragColor.r = min(abs(x), 6.0);"
         "}",
         *SkSL::ShaderCapsFactory::CannotUseMinAndAbsTogether(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float minAbsHackVar0;\n"
         "    float minAbsHackVar1;\n"
         "    sk_FragColor.x = ((minAbsHackVar0 = abs(-5.0)) < (minAbsHackVar1 = 6.0) ? "
                                                               "minAbsHackVar0 : minAbsHackVar1);\n"
         "}\n");
}

DEF_TEST(SkSLFractNegative, r) {
    static constexpr char input[] =
        "void main() {"
        "float x = -42.0;"
        "sk_FragColor.r = half(fract(x));"
        "}";
    static constexpr char output_default[] =
        "#version 400\n"
        "out vec4 sk_FragColor;\n"
        "void main() {\n"
        "    sk_FragColor.x = fract(-42.0);\n"
        "}\n";
    static constexpr char output_workaround[] =
        "#version 400\n"
        "out vec4 sk_FragColor;\n"
        "void main() {\n"
        "    sk_FragColor.x = (0.5 - sign(-42.0) * (0.5 - fract(abs(-42.0))));\n"
        "}\n";

    test(r, input, *SkSL::ShaderCapsFactory::Default(), output_default);
    test(r, input, *SkSL::ShaderCapsFactory::CannotUseFractForNegativeValues(), output_workaround);
}

DEF_TEST(SkSLNegatedAtan, r) {
    test(r,
         "void main() { float2 x = float2(sqrt(2)); sk_FragColor.r = half(atan(x.x, -x.y)); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(sqrt(2.0));\n"
         "    sk_FragColor.x = atan(x.x, -x.y);\n"
         "}\n");
    test(r,
         "void main() { float2 x = float2(sqrt(2)); sk_FragColor.r = half(atan(x.x, -x.y)); }",
         *SkSL::ShaderCapsFactory::MustForceNegatedAtanParamToFloat(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(sqrt(2.0));\n"
         "    sk_FragColor.x = atan(x.x, -1.0 * x.y);\n"
         "}\n");
}

DEF_TEST(SkSLDerivatives, r) {
    test(r,
         "void main() { sk_FragColor.r = half(dFdx(1)); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = dFdx(1.0);\n"
         "}\n");
    test(r,
         "void main() { sk_FragColor.r = 1; }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 1.0;\n"
         "}\n");
    test(r,
         "void main() { sk_FragColor.r = half(dFdx(1)); }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "#extension GL_OES_standard_derivatives : require\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = dFdx(1.0);\n"
         "}\n");

    SkSL::Program::Settings settings;
    settings.fFlipY = false;
    auto caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    SkSL::Program::Inputs inputs;
    test(r,
         "void main() { sk_FragColor.r = half(dFdx(1)), sk_FragColor.g = half(dFdy(1)); }",
         settings,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    (sk_FragColor.x = dFdx(1.0) , sk_FragColor.y = dFdy(1.0));\n"
         "}\n",
         &inputs);
    settings.fFlipY = true;
    test(r,
         "void main() { sk_FragColor.r = half(dFdx(1)), sk_FragColor.g = half(dFdy(1)); }",
         settings,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    (sk_FragColor.x = dFdx(1.0) , sk_FragColor.y = -dFdy(1.0));\n"
         "}\n",
         &inputs);
}

DEF_TEST(SkSLTexture, r) {
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "float4 a = sample(one, 0);"
         "float4 b = sample(two, float2(0));"
         "float4 c = sample(one, float2(0));"
         "float4 d = sample(two, float3(0));"
         "sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture(one, 0.0);\n"
         "    vec4 b = texture(two, vec2(0.0));\n"
         "    vec4 c = textureProj(one, vec2(0.0));\n"
         "    vec4 d = textureProj(two, vec3(0.0));\n"
         "    sk_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
         "}\n");
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "float4 a = sample(one, 0);"
         "float4 b = sample(two, float2(0));"
         "float4 c = sample(one, float2(0));"
         "float4 d = sample(two, float3(0));"
         "sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));"
         "}",
         *SkSL::ShaderCapsFactory::Version110(),
         "#version 110\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture1D(one, 0.0);\n"
         "    vec4 b = texture2D(two, vec2(0.0));\n"
         "    vec4 c = texture1DProj(one, vec2(0.0));\n"
         "    vec4 d = texture2DProj(two, vec3(0.0));\n"
         "    gl_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
         "}\n");
}

DEF_TEST(SkSLSharpen, r) {
    SkSL::Program::Settings settings;
    settings.fSharpenTextures = true;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    SkSL::Program::Inputs inputs;
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "float4 a = sample(one, 0);"
         "float4 b = sample(two, float2(0));"
         "float4 c = sample(one, float2(0));"
         "float4 d = sample(two, float3(0));"
         "sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));"
         "}",
         settings,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture(one, 0.0, -0.5);\n"
         "    vec4 b = texture(two, vec2(0.0), -0.5);\n"
         "    vec4 c = textureProj(one, vec2(0.0), -0.5);\n"
         "    vec4 d = textureProj(two, vec3(0.0), -0.5);\n"
         "    sk_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
         "}\n",
         &inputs);

    caps = SkSL::ShaderCapsFactory::Version110();
    settings.fCaps = caps.get();
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "float4 a = sample(one, 0);"
         "float4 b = sample(two, float2(0));"
         "float4 c = sample(one, float2(0));"
         "float4 d = sample(two, float3(0));"
         "sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));"
         "}",
         settings,
         "#version 110\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture1D(one, 0.0, -0.5);\n"
         "    vec4 b = texture2D(two, vec2(0.0), -0.5);\n"
         "    vec4 c = texture1DProj(one, vec2(0.0), -0.5);\n"
         "    vec4 d = texture2DProj(two, vec3(0.0), -0.5);\n"
         "    gl_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
         "}\n",
         &inputs);
}

DEF_TEST(SkSLFragCoord, r) {
    SkSL::Program::Settings settings;
    settings.fFlipY = true;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::FragCoordsOld();
    settings.fCaps = caps.get();
    SkSL::Program::Inputs inputs;
    test(r,
         "void main() { sk_FragColor.xy = half2(sk_FragCoord.xy); }",
         settings,
         "#version 110\n"
         "#extension GL_ARB_fragment_coord_conventions : require\n"
         "layout(origin_upper_left) in vec4 gl_FragCoord;\n"
         "void main() {\n"
         "    gl_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);

    caps = SkSL::ShaderCapsFactory::FragCoordsNew();
    settings.fCaps = caps.get();
    test(r,
         "void main() { sk_FragColor.xy = half2(sk_FragCoord.xy); }",
         settings,
         "#version 400\n"
         "layout(origin_upper_left) in vec4 gl_FragCoord;\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);

    caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    test(r,
         "void main() { sk_FragColor.xy = half2(sk_FragCoord.xy); }",
         settings,
         "#version 400\n"
         "uniform float u_skRTHeight;\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTHeight - gl_FragCoord.y, "
                 "gl_FragCoord.z, gl_FragCoord.w);\n"
         "    sk_FragColor.xy = sk_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, inputs.fRTHeight);

    settings.fFlipY = false;
    test(r,
         "void main() { sk_FragColor.xy = half2(sk_FragCoord.xy); }",
         settings,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);

    test(r,
         "in float4 pos; void main() { sk_Position = pos; }",
         *SkSL::ShaderCapsFactory::CannotUseFragCoord(),
         "#version 400\n"
         "out vec4 sk_FragCoord_Workaround;\n"
         "in vec4 pos;\n"
         "void main() {\n"
         "    sk_FragCoord_Workaround = (gl_Position = pos);\n"
         "}\n",
         SkSL::Program::kVertex_Kind);

    test(r,
         "uniform float4 sk_RTAdjust; in float4 pos; void main() { sk_Position = pos; }",
         *SkSL::ShaderCapsFactory::CannotUseFragCoord(),
         "#version 400\n"
         "out vec4 sk_FragCoord_Workaround;\n"
         "uniform vec4 sk_RTAdjust;\n"
         "in vec4 pos;\n"
         "void main() {\n"
         "    sk_FragCoord_Workaround = (gl_Position = pos);\n"
         "    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw,"
                                " 0.0, gl_Position.w);\n"
         "}\n",
         SkSL::Program::kVertex_Kind);

    test(r,
         "void main() { sk_FragColor.xy = half2(sk_FragCoord.xy); }",
         *SkSL::ShaderCapsFactory::CannotUseFragCoord(),
         "#version 400\n"
         "in vec4 sk_FragCoord_Workaround;\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float sk_FragCoord_InvW = 1. / sk_FragCoord_Workaround.w;\n"
         "    vec4 sk_FragCoord_Resolved = vec4(sk_FragCoord_Workaround.xyz * "
              "sk_FragCoord_InvW, sk_FragCoord_InvW);\n"
         "    sk_FragCoord_Resolved.xy = floor(sk_FragCoord_Resolved.xy) + vec2(.5);\n"
         "    sk_FragColor.xy = sk_FragCoord_Resolved.xy;\n"
         "}\n");
}

DEF_TEST(SkSLGeometry, r) {
    test(r,
         "layout(points) in;"
         "layout(invocations = 2) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void main() {"
         "sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "sk_Position = sk_in[0].sk_Position + float4(0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "EndPrimitive();"
         "}",
         *SkSL::ShaderCapsFactory::GeometryShaderSupport(),
         "#version 400\n"
         "layout (points) in ;\n"
         "layout (invocations = 2) in ;\n"
         "layout (line_strip, max_vertices = 2) out ;\n"
         "void main() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    EndPrimitive();\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
}

DEF_TEST(SkSLGeometryShaders, r) {
    test(r,
         "layout(points) in;"
         "layout(invocations = 2) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void test() {"
         "sk_Position = sk_in[0].sk_Position + float4(0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "}"
         "void main() {"
         "test();"
         "sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "}",
         *SkSL::ShaderCapsFactory::NoGSInvocationsSupport(),
         R"__GLSL__(#version 400
int sk_InvocationID;
layout (points) in ;
layout (line_strip, max_vertices = 4) out ;
void _invoke() {
    {
        gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(sk_InvocationID));
        EmitVertex();
    }


    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(sk_InvocationID));
    EmitVertex();
}
void main() {
    for (sk_InvocationID = 0;sk_InvocationID < 2; sk_InvocationID++) {
        _invoke();
        EndPrimitive();
    }
}
)__GLSL__",
         SkSL::Program::kGeometry_Kind);
    test(r,
         "layout(points, invocations = 2) in;"
         "layout(invocations = 3) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void main() {"
         "sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "EndPrimitive();"
         "}",
         *SkSL::ShaderCapsFactory::GSInvocationsExtensionString(),
         "#version 400\n"
         "#extension GL_ARB_gpu_shader5 : require\n"
         "layout (points, invocations = 2) in ;\n"
         "layout (invocations = 3) in ;\n"
         "layout (line_strip, max_vertices = 2) out ;\n"
         "void main() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    EndPrimitive();\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
    test(r,
         "layout(points, invocations = 2) in;"
         "layout(invocations = 3) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void main() {"
         "sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "EndPrimitive();"
         "}",
         *SkSL::ShaderCapsFactory::GeometryShaderExtensionString(),
         "#version 310es\n"
         "#extension GL_EXT_geometry_shader : require\n"
         "layout (points, invocations = 2) in ;\n"
         "layout (invocations = 3) in ;\n"
         "layout (line_strip, max_vertices = 2) out ;\n"
         "void main() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    EndPrimitive();\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
}

DEF_TEST(SkSLTypePrecision, r) {
    test(r,
         "float f = 1;"
         "half h = 2;"
         "float d = 3;"
         "float2 f2 = float2(1, 2);"
         "half3 h3 = half3(1, 2, 3);"
         "float4 d4 = float4(1, 2, 3, 4);"
         "float2x2 f22 = float2x2(1, 2, 3, 4);"
         "half2x4 h24 = half2x4(1, 2, 3, 4, 5, 6, 7, 8);"
         "float4x2 d42 = float4x2(1, 2, 3, 4, 5, 6, 7, 8);"
         "void main() {"
         "sk_FragColor.r = half(f + h + d + f2.x + h3.x + d4.x + f22[0][0] + h24[0][0] + "
                               "d42[0][0]);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "float f = 1.0;\n"
         "float h = 2.0;\n"
         "float d = 3.0;\n"
         "vec2 f2 = vec2(1.0, 2.0);\n"
         "vec3 h3 = vec3(1.0, 2.0, 3.0);\n"
         "vec4 d4 = vec4(1.0, 2.0, 3.0, 4.0);\n"
         "mat2 f22 = mat2(1.0, 2.0, 3.0, 4.0);\n"
         "mat2x4 h24 = mat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);\n"
         "mat4x2 d42 = mat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);\n"
         "void main() {\n"
         "    sk_FragColor.x = (((((((f + h) + d) + f2.x) + h3.x) + d4.x) + "
                               "f22[0][0]) + h24[0][0]) + d42[0][0];\n"
         "}\n");
    test(r,
         "float f = 1;"
         "half h = 2;"
         "float2 f2 = float2(1, 2);"
         "half3 h3 = half3(1, 2, 3);"
         "float2x2 f22 = float2x2(1, 2, 3, 4);"
         "half2x4 h24 = half2x4(1, 2, 3, 4, 5, 6, 7, 8);"
         "void main() {"
         "sk_FragColor.r = half(f + h + f2.x + h3.x + f22[0][0] + h24[0][0]);"
         "}",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "highp float f = 1.0;\n"
         "mediump float h = 2.0;\n"
         "highp vec2 f2 = vec2(1.0, 2.0);\n"
         "mediump vec3 h3 = vec3(1.0, 2.0, 3.0);\n"
         "highp mat2 f22 = mat2(1.0, 2.0, 3.0, 4.0);\n"
         "mediump mat2x4 h24 = mat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);\n"
         "void main() {\n"
         "    sk_FragColor.x = ((((f + h) + f2.x) + h3.x) + f22[0][0]) + h24[0][0];\n"
         "}\n");
}

DEF_TEST(SkSLForceHighPrecision, r) {
    test(r,
         "void main() {\n half x = half(sqrt(1));\n half4 y = half4(x);\n sk_FragColor = y;\n }",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    mediump float x = sqrt(1.0);\n"
         "    mediump vec4 y = vec4(x);\n"
         "    sk_FragColor = y;\n"
         "}\n");
    SkSL::Program::Settings settings;
    settings.fForceHighPrecision = true;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::UsesPrecisionModifiers();
    settings.fCaps = caps.get();
    SkSL::Program::Inputs inputs;
    test(r,
         "void main() { half x = half(sqrt(1)); half4 y = half4(x); sk_FragColor = y; }",
         settings,
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    highp float x = sqrt(1.0);\n"
         "    highp vec4 y = vec4(x);\n"
         "    sk_FragColor = y;\n"
         "}\n",
         &inputs);
}

DEF_TEST(SkSLNormalization, r) {
    test(r,
         "uniform float4 sk_RTAdjust; void main() { sk_Position = half4(1); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "uniform vec4 sk_RTAdjust;\n"
         "void main() {\n"
         "    gl_Position = vec4(1.0);\n"
         "    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * "
                                "sk_RTAdjust.yw, 0.0, gl_Position.w);\n"
         "}\n",
         SkSL::Program::kVertex_Kind);
    test(r,
         "uniform float4 sk_RTAdjust;"
         "layout(points) in;"
         "layout(invocations = 2) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void main() {"
         "sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "sk_Position = sk_in[0].sk_Position + float4(0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "EndPrimitive();"
         "}",
         *SkSL::ShaderCapsFactory::GeometryShaderSupport(),
         "#version 400\n"
         "uniform vec4 sk_RTAdjust;\n"
         "layout (points) in ;\n"
         "layout (invocations = 2) in ;\n"
         "layout (line_strip, max_vertices = 2) out ;\n"
         "void main() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    {\n"
         "        gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * "
                                    "sk_RTAdjust.yw, 0.0, gl_Position.w);\n"
         "        EmitVertex();\n"
         "    }\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    {\n"
         "        gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * "
                                    "sk_RTAdjust.yw, 0.0, gl_Position.w);\n"
         "        EmitVertex();\n"
         "    }\n"
         "    EndPrimitive();\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
}

DEF_TEST(SkSLIncompleteShortIntPrecision, r) {
    test(r,
         "uniform sampler2D tex;"
         "in float2 texcoord;"
         "in short2 offset;"
         "void main() {"
         "    short scalar = offset.y;"
         "    sk_FragColor = sample(tex, texcoord + float2(offset * scalar));"
         "}",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "uniform sampler2D tex;\n"
         "in highp vec2 texcoord;\n"
         "in mediump ivec2 offset;\n"
         "void main() {\n"
         "    mediump int scalar = offset.y;\n"
         "    sk_FragColor = texture(tex, texcoord + vec2(offset * scalar));\n"
         "}\n",
         SkSL::Program::kFragment_Kind);
    test(r,
         "uniform sampler2D tex;"
         "in float2 texcoord;"
         "in short2 offset;"
         "void main() {"
         "    short scalar = offset.y;"
         "    sk_FragColor = sample(tex, texcoord + float2(offset * scalar));"
         "}",
         *SkSL::ShaderCapsFactory::IncompleteShortIntPrecision(),
         "#version 310es\n"
         "precision mediump float;\n"
         "precision mediump sampler2D;\n"
         "out mediump vec4 sk_FragColor;\n"
         "uniform sampler2D tex;\n"
         "in highp vec2 texcoord;\n"
         "in highp ivec2 offset;\n"
         "void main() {\n"
         "    highp int scalar = offset.y;\n"
         "    sk_FragColor = texture(tex, texcoord + vec2(offset * scalar));\n"
         "}\n",
         SkSL::Program::kFragment_Kind);
}

DEF_TEST(SkSLWorkaroundAddAndTrueToLoopCondition, r) {
    test(r,
         "void main() {"
         "    int c = 0;"
         "    for (int i = 0; i < 4 || c < 10; ++i) {"
         "        c += 1;"
         "    }"
         "}",
         *SkSL::ShaderCapsFactory::AddAndTrueToLoopCondition(),
         "#version 400\n"
         "void main() {\n"
         "    int c = 0;\n"
         "    for (int i = 0;(i < 4 || c < 10) && true; ++i) {\n"
         "        c += 1;\n"
         "    }\n"
         "}\n",
         SkSL::Program::kFragment_Kind
         );
}

DEF_TEST(SkSLWorkaroundUnfoldShortCircuitAsTernary, r) {
    test(r,
         "uniform bool x;"
         "uniform bool y;"
         "uniform int i;"
         "uniform int j;"
         "void main() {"
         "    bool andXY = x && y;"
         "    bool orXY = x || y;"
         "    bool combo = (x && y) || (x || y);"
         "    bool prec = (i + j == 3) && y;"
         "    while (andXY && orXY && combo && prec) {"
         "        sk_FragColor = half4(0);"
         "        break;"
         "    }"
         "}",
         *SkSL::ShaderCapsFactory::UnfoldShortCircuitAsTernary(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform bool x;\n"
         "uniform bool y;\n"
         "uniform int i;\n"
         "uniform int j;\n"
         "void main() {\n"
         "    bool andXY = x ? y : false;\n"
         "    bool orXY = x ? true : y;\n"
         "    bool combo = (x ? y : false) ? true : (x ? true : y);\n"
         "    bool prec = i + j == 3 ? y : false;\n"
         "    while (((andXY ? orXY : false) ? combo : false) ? prec : false) {\n"
         "        sk_FragColor = vec4(0.0);\n"
         "        break;\n"
         "    }\n"
         "}\n",
         SkSL::Program::kFragment_Kind
         );
}

DEF_TEST(SkSLWorkaroundEmulateAbsIntFunction, r) {
    test(r,
         "uniform int i;"
         "uniform float f;"
         "void main() {"
         "    float output = abs(f) + abs(i);"
         "    sk_FragColor = half4(half(output));"
         "}",
         *SkSL::ShaderCapsFactory::EmulateAbsIntFunction(),
         "#version 400\n"
         "int _absemulation(int x) {\n"
         "    return x * sign(x);\n"
         "}\n"
         "out vec4 sk_FragColor;\n"
         "uniform int i;\n"
         "uniform float f;\n"
         "void main() {\n"
         "    float output = abs(f) + float(_absemulation(i));\n"
         "    sk_FragColor = vec4(output);\n"
         "}\n",
         SkSL::Program::kFragment_Kind
         );
}

DEF_TEST(SkSLWorkaroundRewriteDoWhileLoops, r) {
    test(r,
         "void main() {"
         "    int i = 0;"
         "    do {"
         "      ++i;"
         "      do {"
         "        i++;"
         "      } while (true);"
         "    } while (i < 10);"
         "    sk_FragColor = half4(i);"
         "}",
         *SkSL::ShaderCapsFactory::RewriteDoWhileLoops(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    int i = 0;\n"
         "    bool _tmpLoopSeenOnce0 = false;\n"
         "    while (true) {\n"
         "        if (_tmpLoopSeenOnce0) {\n"
         "            if (!(i < 10)) {\n"
         "                break;\n"
         "            }\n"
         "        }\n"
         "        _tmpLoopSeenOnce0 = true;\n"
         "        {\n"
         "            ++i;\n"
         "            bool _tmpLoopSeenOnce1 = false;\n"
         "            while (true) {\n"
         "                if (_tmpLoopSeenOnce1) {\n"
         "                    if (!true) {\n"
         "                        break;\n"
         "                    }\n"
         "                }\n"
         "                _tmpLoopSeenOnce1 = true;\n"
         "                {\n"
         "                    i++;\n"
         "                }\n"
         "            }\n"
         "        }\n"
         "    }\n"
         "    sk_FragColor = vec4(float(i));\n"
         "}\n",
         SkSL::Program::kFragment_Kind
         );
}

DEF_TEST(SkSLWorkaroundRemovePowWithConstantExponent, r) {
    test(r,
         "uniform float x;"
         "uniform float y;"
         "void main() {"
         "    float z = pow(x + 1.0, y + 2.0);"
         "    sk_FragColor = half4(half(z));"
         "}",
         *SkSL::ShaderCapsFactory::RemovePowWithConstantExponent(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform float x;\n"
         "uniform float y;\n"
         "void main() {\n"
         "    float z = exp2((y + 2.0) * log2(x + 1.0));\n"
         "    sk_FragColor = vec4(z);\n"
         "}\n",
         SkSL::Program::kFragment_Kind
         );
}
