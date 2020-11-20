/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "bench/ResultsWriter.h"
#include "bench/SkSLBench.h"
#include "include/core/SkCanvas.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/mock/GrMockCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLParser.h"

class SkSLCompilerStartupBench : public Benchmark {
protected:
    const char* onGetName() override {
        return "sksl_compiler_startup";
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        GrShaderCaps caps(GrContextOptions{});
        for (int i = 0; i < loops; i++) {
            SkSL::Compiler compiler(&caps);
        }
    }
};

DEF_BENCH(return new SkSLCompilerStartupBench();)

enum class Output {
    kNone,
    kGLSL,
    kMetal,
    kSPIRV
};

class SkSLCompileBench : public Benchmark {
public:
    static const char* output_string(Output output) {
        switch (output) {
            case Output::kNone: return "";
            case Output::kGLSL: return "glsl_";
            case Output::kMetal: return "metal_";
            case Output::kSPIRV: return "spirv_";
        }
        SkUNREACHABLE;
    }

    SkSLCompileBench(SkSL::String name, const char* src, bool optimize, Output output)
        : fName(SkSL::String("sksl_") + (optimize ? "" : "unoptimized_") + output_string(output) +
                name)
        , fSrc(src)
        , fCaps(GrContextOptions(), GrMockOptions())
        , fCompiler(fCaps.shaderCaps())
        , fOutput(output) {
            fSettings.fOptimize = optimize;
            // The test programs we compile don't follow Vulkan rules and thus produce invalid
            // SPIR-V. This is harmless, so long as we don't try to validate them.
            fSettings.fValidateSPIRV = false;
        }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            std::unique_ptr<SkSL::Program> program = fCompiler.convertProgram(
                                                                      SkSL::Program::kFragment_Kind,
                                                                      fSrc,
                                                                      fSettings);
            if (fCompiler.errorCount()) {
                SK_ABORT("shader compilation failed: %s\n", fCompiler.errorText().c_str());
            }
            SkSL::String result;
            switch (fOutput) {
                case Output::kNone: break;
                case Output::kGLSL:  SkAssertResult(fCompiler.toGLSL(*program,  &result)); break;
                case Output::kMetal: SkAssertResult(fCompiler.toMetal(*program, &result)); break;
                case Output::kSPIRV: SkAssertResult(fCompiler.toSPIRV(*program, &result)); break;
            }
        }
    }

private:
    SkSL::String fName;
    SkSL::String fSrc;
    GrMockCaps fCaps;
    SkSL::Compiler fCompiler;
    SkSL::Program::Settings fSettings;
    Output fOutput;

    using INHERITED = Benchmark;
};

class SkSLParseBench : public Benchmark {
public:
    SkSLParseBench(SkSL::String name, const char* src)
        : fName("sksl_parse_" + name)
        , fSrc(src)
        , fCaps(GrContextOptions())
        , fCompiler(&fCaps) {}

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        SkSL::ParsedModule module = fCompiler.moduleForProgramKind(
                                                               SkSL::Program::Kind::kFragment_Kind);
        fCompiler.irGenerator().setSymbolTable(module.fSymbols);
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            fCompiler.irGenerator().pushSymbolTable();
            SkSL::Parser parser(fSrc.c_str(), fSrc.length(), *fCompiler.irGenerator().symbolTable(),
                                fCompiler);
            parser.compilationUnit();
            fCompiler.irGenerator().popSymbolTable();
            if (fCompiler.errorCount()) {
                SK_ABORT("shader compilation failed: %s\n", fCompiler.errorText().c_str());
            }
        }
    }

private:
    SkSL::String fName;
    SkSL::String fSrc;
    GrShaderCaps fCaps;
    SkSL::Compiler fCompiler;
    SkSL::Program::Settings fSettings;

    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

#define COMPILER_BENCH(name, text)                                                               \
static constexpr char name ## _SRC[] = text;                                                     \
DEF_BENCH(return new SkSLParseBench(#name, name ## _SRC);)                                       \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/false, Output::kNone);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kNone);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kGLSL);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kMetal);) \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSPIRV);)

// Metal requires a layout set and binding for all of its uniforms. We just care that these shaders
// compile, not that they actually work, so we just fill them with zeroes.
COMPILER_BENCH(large, R"(
layout(set=0, binding=0) uniform half urange_Stage1;
layout(set=0, binding=0) uniform half4 uleftBorderColor_Stage1_c0_c0;
layout(set=0, binding=0) uniform half4 urightBorderColor_Stage1_c0_c0;
layout(set=0, binding=0) uniform float3x3 umatrix_Stage1_c0_c0_c0;
layout(set=0, binding=0) uniform half2 ufocalParams_Stage1_c0_c0_c0_c0;
layout(set=0, binding=0) uniform float4 uscale0_1_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale2_3_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale4_5_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale6_7_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias0_1_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias2_3_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias4_5_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias6_7_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform half4 uthresholds1_7_Stage1_c0_c0_c1;
layout(set=0, binding=0) uniform half4 uthresholds9_13_Stage1_c0_c0_c1;
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
)");

COMPILER_BENCH(medium, R"(
    layout(set=0, binding=0) uniform half2 uDstTextureUpperLeft_Stage1;
    layout(set=0, binding=0) uniform half2 uDstTextureCoordScale_Stage1;
    layout(set=0, binding=0) uniform sampler2D uDstTextureSampler_Stage1;
    noperspective in half4 vQuadEdge_Stage0;
    noperspective in half4 vinColor_Stage0;
    out half4 sk_FragColor;
    half luminance_Stage1(half3 color) {
        return dot(half3(0.3, 0.59, 0.11), color);
    }

    half3 set_luminance_Stage1(half3 hueSat, half alpha, half3 lumColor) {
        half diff = luminance_Stage1(lumColor - hueSat);
        half3 outColor = hueSat + diff;
        half outLum = luminance_Stage1(outColor);
        half minComp = min(min(outColor.r, outColor.g), outColor.b);
        half maxComp = max(max(outColor.r, outColor.g), outColor.b);
        if (minComp < 0.0 && outLum != minComp) {
            outColor = outLum + ((outColor - half3(outLum, outLum, outLum)) * outLum) /
                       (outLum - minComp);
        }
        if (maxComp > alpha && maxComp != outLum) {
            outColor = outLum +((outColor - half3(outLum, outLum, outLum)) * (alpha - outLum)) /
                       (maxComp - outLum);
        }
        return outColor;
    }

    void main() {
        half4 outputColor_Stage0;
        half4 outputCoverage_Stage0;
        { // Stage 0, QuadEdge
            outputColor_Stage0 = vinColor_Stage0;
            half edgeAlpha;
            half2 duvdx = half2(dFdx(vQuadEdge_Stage0.xy));
            half2 duvdy = half2(dFdy(vQuadEdge_Stage0.xy));
            if (vQuadEdge_Stage0.z > 0.0 && vQuadEdge_Stage0.w > 0.0) {
                edgeAlpha = min(min(vQuadEdge_Stage0.z, vQuadEdge_Stage0.w) + 0.5, 1.0);
            } else {
                half2 gF = half2(2.0 * vQuadEdge_Stage0.x * duvdx.x - duvdx.y,
                                 2.0 * vQuadEdge_Stage0.x * duvdy.x - duvdy.y);
                edgeAlpha = (vQuadEdge_Stage0.x*vQuadEdge_Stage0.x - vQuadEdge_Stage0.y);
                edgeAlpha = saturate(0.5 - edgeAlpha / length(gF));
            }
            outputCoverage_Stage0 = half4(edgeAlpha);
        }
        { // Xfer Processor: Custom Xfermode
            if (all(lessThanEqual(outputCoverage_Stage0.rgb, half3(0)))) {
                discard;
            }
            // Read color from copy of the destination.
            half2 _dstTexCoord = (half2(sk_FragCoord.xy) - uDstTextureUpperLeft_Stage1) *
                                  uDstTextureCoordScale_Stage1;
            _dstTexCoord.y = 1.0 - _dstTexCoord.y;
            half4 _dstColor = sample(uDstTextureSampler_Stage1, _dstTexCoord);
            sk_FragColor.a = outputColor_Stage0.a + (1.0 - outputColor_Stage0.a) * _dstColor.a;
            half4 srcDstAlpha = outputColor_Stage0 * _dstColor.a;
            sk_FragColor.rgb = set_luminance_Stage1(_dstColor.rgb * outputColor_Stage0.a,
                                                    srcDstAlpha.a, srcDstAlpha.rgb);
            sk_FragColor.rgb += (1.0 - outputColor_Stage0.a) * _dstColor.rgb + (1.0 - _dstColor.a) *
                                outputColor_Stage0.rgb;
            sk_FragColor = outputCoverage_Stage0 * sk_FragColor +
                           (half4(1.0) - outputCoverage_Stage0) * _dstColor;
        }
    }
)");

COMPILER_BENCH(small, R"(
    layout(set=0, binding=0) uniform float3x3 umatrix_Stage1_c0_c0;
    layout(set=0, binding=0) uniform sampler2D uTextureSampler_0_Stage1;
    noperspective in float2 vTransformedCoords_0_Stage0;
    out half4 sk_FragColor;
    half4 TextureEffect_Stage1_c0_c0_c0(half4 _input)
    {
        half4 _output;
        return sample(uTextureSampler_0_Stage1, vTransformedCoords_0_Stage0);
    }
    half4 MatrixEffect_Stage1_c0_c0(half4 _input)
    {
        half4 _output;
        return TextureEffect_Stage1_c0_c0_c0(_input);
    }
    inline half4 Blend_Stage1_c0(half4 _input)
    {
        half4 _output;
        // Blend mode: Modulate (Compose-One behavior)
        return blend_modulate(MatrixEffect_Stage1_c0_c0(half4(1)), _input);
    }
    void main()
    {
        half4 outputColor_Stage0;
        half4 outputCoverage_Stage0;
        {
            // Stage 0, QuadPerEdgeAAGeometryProcessor
            outputColor_Stage0 = half4(1);
            outputCoverage_Stage0 = half4(1);
        }
        half4 output_Stage1;
        output_Stage1 = Blend_Stage1_c0(outputColor_Stage0);
        {
            // Xfer Processor: Porter Duff
            sk_FragColor = output_Stage1 * outputCoverage_Stage0;
        }
    }
)");

COMPILER_BENCH(tiny, "void main() { sk_FragColor = half4(1); }");

#if defined(SK_BUILD_FOR_UNIX)

#include <malloc.h>

// These benchmarks aren't timed, they produce memory usage statistics. They run standalone, and
// directly add their results to the nanobench log.
void RunSkSLMemoryBenchmarks(NanoJSONResultsWriter* log) {
    auto heap_bytes_used = []() { return mallinfo().uordblks; };
    auto bench = [log](const char* name, int bytes) {
        log->beginObject(name);          // test
        log->beginObject("meta");        //   config
        log->appendS32("bytes", bytes);  //     sub_result
        log->endObject();                //   config
        log->endObject();                // test
    };

    // Heap used by a default compiler (with no modules loaded)
    {
        int before = heap_bytes_used();
        GrShaderCaps caps(GrContextOptions{});
        SkSL::Compiler compiler(&caps);
        int after = heap_bytes_used();
        bench("sksl_compiler_baseline", after - before);
    }

    // Heap used by a compiler with the two main GPU modules (fragment + vertex) loaded
    {
        int before = heap_bytes_used();
        GrShaderCaps caps(GrContextOptions{});
        SkSL::Compiler compiler(&caps);
        compiler.moduleForProgramKind(SkSL::Program::kVertex_Kind);
        compiler.moduleForProgramKind(SkSL::Program::kFragment_Kind);
        int after = heap_bytes_used();
        bench("sksl_compiler_gpu", after - before);
    }

    // Heap used by a compiler with the runtime effect module loaded
    {
        int before = heap_bytes_used();
        GrShaderCaps caps(GrContextOptions{});
        SkSL::Compiler compiler(&caps);
        compiler.moduleForProgramKind(SkSL::Program::kPipelineStage_Kind);
        int after = heap_bytes_used();
        bench("sksl_compiler_runtimeeffect", after - before);
    }
}

#else

void RunSkSLMemoryBenchmarks(NanoJSONResultsWriter*) {}

#endif
