/*
 * Copyright 2020 Google LLC
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

static void test(skiatest::Reporter* r, const SkSL::Program::Settings& settings, const char* src,
                 SkSL::Program::Inputs* inputs,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        *inputs = program->fInputs;
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        REPORTER_ASSERT(r, output != "");
        //SkDebugf("GLSL output:\n\n%s", output.c_str());
    }
}

static void test(skiatest::Reporter* r, const GrShaderCaps& caps, const char* src,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, settings, src, &inputs, kind);
}

DEF_TEST(SkSLGLSLTestbed, r) {
    // Add in your SkSL here.
printf("------------ STARTING TEST\n");
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
uniform half urange_Stage1;
uniform half4 uleftBorderColor_Stage1_c0_c0;
uniform half4 urightBorderColor_Stage1_c0_c0;
uniform float3x3 umatrix_Stage1_c0_c0_c0;
uniform half2 ufocalParams_Stage1_c0_c0_c0_c0;
uniform float4 uscale0_1_Stage1_c0_c0_c1;
uniform float4 uscale2_3_Stage1_c0_c0_c1;
uniform float4 uscale4_5_Stage1_c0_c0_c1;
uniform float4 uscale6_7_Stage1_c0_c0_c1;
uniform float4 ubias0_1_Stage1_c0_c0_c1;
uniform float4 ubias2_3_Stage1_c0_c0_c1;
uniform float4 ubias4_5_Stage1_c0_c0_c1;
uniform float4 ubias6_7_Stage1_c0_c0_c1;
uniform half4 uthresholds1_7_Stage1_c0_c0_c1;
uniform half4 uthresholds9_13_Stage1_c0_c0_c1;
flat in half4 vcolor_Stage0;
in float vcoverage_Stage0;
flat in float4 vgeomSubset_Stage0;
in float2 vTransformedCoords_0_Stage0;
out half4 sk_FragColor;
half4 TwoPointConicalGradientLayout_Stage1_c0_c0_c0_c0(half4 _input)
{
    half4 _output;
    float t = -1.0;
    half v = 1.0;
    @switch (2)
    {
        case 1:
        {
            half r0_2 = ufocalParams_Stage1_c0_c0_c0_c0.y;
            t = float(r0_2) - vTransformedCoords_0_Stage0.y * vTransformedCoords_0_Stage0.y;
            if (t >= 0.0)
            {
                t = vTransformedCoords_0_Stage0.x + sqrt(t);
            }
            else
            {
                v = -1.0;
            }
        }
        break;
        case 0:
        {
            half r0 = ufocalParams_Stage1_c0_c0_c0_c0.x;
            @if (true)
            {
                t = length(vTransformedCoords_0_Stage0) - float(r0);
            }
            else
            {
                t = -length(vTransformedCoords_0_Stage0) - float(r0);
            }
        }
        break;
        case 2:
        {
            half invR1 = ufocalParams_Stage1_c0_c0_c0_c0.x;
            half fx = ufocalParams_Stage1_c0_c0_c0_c0.y;
            float x_t = -1.0;
            @if (false)
            {
                x_t = dot(vTransformedCoords_0_Stage0, vTransformedCoords_0_Stage0) / vTransformedCoords_0_Stage0.x;
            }
            else if (true)
            {
                x_t = length(vTransformedCoords_0_Stage0) - vTransformedCoords_0_Stage0.x * float(invR1);
            }
            else
            {
                float temp = vTransformedCoords_0_Stage0.x * vTransformedCoords_0_Stage0.x - vTransformedCoords_0_Stage0.y * vTransformedCoords_0_Stage0.y;
                if (temp >= 0.0)
                {
                    @if (false || !true)
                    {
                        x_t = -sqrt(temp) - vTransformedCoords_0_Stage0.x * float(invR1);
                    }
                    else
                    {
                        x_t = sqrt(temp) - vTransformedCoords_0_Stage0.x * float(invR1);
                    }
                }
            }
            @if (!true)
            {
                if (x_t <= 0.0)
                {
                    v = -1.0;
                }
            }
            @if (true)
            {
                @if (false)
                {
                    t = x_t;
                }
                else
                {
                    t = x_t + float(fx);
                }
            }
            else
            {
                @if (false)
                {
                    t = -x_t;
                }
                else
                {
                    t = -x_t + float(fx);
                }
            }
            @if (false)
            {
                t = 1.0 - t;
            }
        }
        break;
    }
    _output = half4(half(t), v, 0.0, 0.0);
    return _output;
}
half4 MatrixEffect_Stage1_c0_c0_c0(half4 _input)
{
    half4 _output;
    _output = TwoPointConicalGradientLayout_Stage1_c0_c0_c0_c0(_input);
    return _output;
}
half4 UnrolledBinaryGradientColorizer_Stage1_c0_c0_c1(half4 _input, float2 _coords)
{
    half4 _output;
    half t = half(_coords.x);
    float4 scale, bias;
    if (4 <= 4 || t < uthresholds1_7_Stage1_c0_c0_c1.w)
    {
        if (4 <= 2 || t < uthresholds1_7_Stage1_c0_c0_c1.y)
        {
            if (4 <= 1 || t < uthresholds1_7_Stage1_c0_c0_c1.x)
            {
                scale = uscale0_1_Stage1_c0_c0_c1;
                bias = ubias0_1_Stage1_c0_c0_c1;
            }
            else
            {
                scale = uscale2_3_Stage1_c0_c0_c1;
                bias = ubias2_3_Stage1_c0_c0_c1;
            }
        }
        else
        {
            if (4 <= 3 || t < uthresholds1_7_Stage1_c0_c0_c1.z)
            {
                scale = uscale4_5_Stage1_c0_c0_c1;
                bias = ubias4_5_Stage1_c0_c0_c1;
            }
            else
            {
                scale = uscale6_7_Stage1_c0_c0_c1;
                bias = ubias6_7_Stage1_c0_c0_c1;
            }
        }
    }
    else
    {
        if (4 <= 6 || t < uthresholds9_13_Stage1_c0_c0_c1.y)
        {
            if (4 <= 5 || t < uthresholds9_13_Stage1_c0_c0_c1.x)
            {
                scale = float4(0);
                bias = float4(0);
            }
            else
            {
                scale = float4(0);
                bias = float4(0);
            }
        }
        else
        {
            if (4 <= 7 || t < uthresholds9_13_Stage1_c0_c0_c1.z)
            {
                scale = float4(0);
                bias = float4(0);
            }
            else
            {
                scale = float4(0);
                bias = float4(0);
            }
        }
    }
    _output = half4(float(t) * scale + bias);
    return _output;
}
half4 ClampedGradientEffect_Stage1_c0_c0(half4 _input)
{
    half4 _output;
    half4 t = MatrixEffect_Stage1_c0_c0_c0(_input);
    if (!false && t.y < 0.0)
    {
        _output = half4(0.0);
    }
    else if (t.x < 0.0)
    {
        _output = uleftBorderColor_Stage1_c0_c0;
    }
    else if (t.x > 1.0)
    {
        _output = urightBorderColor_Stage1_c0_c0;
    }
    else
    {
        _output = UnrolledBinaryGradientColorizer_Stage1_c0_c0_c1(_input, float2(half2(t.x, 0.0)));
    }
    @if (false)
    {
        _output.xyz *= _output.w;
    }
    return _output;
}
half4 OverrideInputFragmentProcessor_Stage1_c0(half4 _input)
{
    half4 _output;
    half4 constColor;
    @if (false)
    {
        constColor = half4(0);
    }
    else
    {
        constColor = half4(1.000000, 1.000000, 1.000000, 1.000000);
    }
    _output = ClampedGradientEffect_Stage1_c0_c0(constColor);
    return _output;
}
void main()
{
    half4 outputColor_Stage0;
    half4 outputCoverage_Stage0;
    {
        // Stage 0, QuadPerEdgeAAGeometryProcessor
        outputColor_Stage0 = vcolor_Stage0;
        float coverage = vcoverage_Stage0 * sk_FragCoord.w;
        float4 geoSubset;
        geoSubset = vgeomSubset_Stage0;
        if (coverage < 0.5)
        {
            float4 dists4 = clamp(float4(1, 1, -1, -1) * (sk_FragCoord.xyxy - geoSubset), 0, 1);
            float2 dists2 = dists4.xy * dists4.zw;
            coverage = min(coverage, dists2.x * dists2.y);
        }
        outputCoverage_Stage0 = half4(half(coverage));
    }
    half4 output_Stage1;
    {
        // Stage 1, DitherEffect
        half4 color = OverrideInputFragmentProcessor_Stage1_c0(outputColor_Stage0);
        half value;
        @if (sk_Caps.integerSupport)
        {
            uint x = uint(sk_FragCoord.x);
            uint y = uint(sk_FragCoord.y) ^ x;
            uint m = (((((y & 1) << 5 | (x & 1) << 4) | (y & 2) << 2) | (x & 2) << 1) | (y & 4) >> 1) | (x & 4) >> 2;
            value = half(m) / 64.0 - 0.4921875;
        }
        else
        {
            half4 bits = mod(half4(sk_FragCoord.yxyx), half4(2.0, 2.0, 4.0, 4.0));
            bits.zw = step(2.0, bits.zw);
            bits.xz = abs(bits.xz - bits.yw);
            value = dot(bits, half4(0.5, 0.25, 0.125, 0.0625)) - 0.46875;
        }
        output_Stage1 = half4(clamp(color.xyz + value * urange_Stage1, 0.0, color.w), color.w);
    }
    {
        // Xfer Processor: Porter Duff
        sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
}
         )__SkSL__");
printf("************** ENDING TEST\n");
}
