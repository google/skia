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
                                                                      SkSL::ProgramKind::kFragment,
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
        SkSL::ParsedModule module = fCompiler.moduleForProgramKind(SkSL::ProgramKind::kFragment);
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

// This fragment shader is from the third tile on the top row of GM_gradients_2pt_conical_outside.
COMPILER_BENCH(large, R"(
layout(set=0, binding=0) uniform half urange_Stage1_c0;
layout(set=0, binding=0) uniform half4 uleftBorderColor_Stage1_c0_c0_c0;
layout(set=0, binding=0) uniform half4 urightBorderColor_Stage1_c0_c0_c0;
layout(set=0, binding=0) uniform float3x3 umatrix_Stage1_c0_c0_c0_c0;
layout(set=0, binding=0) uniform half2 ufocalParams_Stage1_c0_c0_c0_c0_c0;
layout(set=0, binding=0) uniform float4 uscale0_1_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale2_3_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale4_5_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 uscale6_7_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias0_1_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias2_3_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias4_5_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform float4 ubias6_7_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform half4 uthresholds1_7_Stage1_c0_c0_c0_c1;
layout(set=0, binding=0) uniform half4 uthresholds9_13_Stage1_c0_c0_c0_c1;
flat in half4 vcolor_Stage0;
noperspective in float2 vTransformedCoords_0_Stage0;
out half4 sk_FragColor;
half4 TwoPointConicalGradientLayout_Stage1_c0_c0_c0_c0_c0(half4 _input)
{
    float t = -1.0;
    half v = 1.0;
    @switch (2)
    {
        case 1:
        {
            half r0_2 = ufocalParams_Stage1_c0_c0_c0_c0_c0.y;
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
            half r0 = ufocalParams_Stage1_c0_c0_c0_c0_c0.x;
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
            half invR1 = ufocalParams_Stage1_c0_c0_c0_c0_c0.x;
            half fx = ufocalParams_Stage1_c0_c0_c0_c0_c0.y;
            float x_t = -1.0;
            @if (false)
            {
                x_t = dot(vTransformedCoords_0_Stage0, vTransformedCoords_0_Stage0) / vTransformedCoords_0_Stage0.x;
            }
            else if (false)
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
            @if (!false)
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
    return half4(half(t), v, 0.0, 0.0);
}
half4 MatrixEffect_Stage1_c0_c0_c0_c0(half4 _input)
{
    return TwoPointConicalGradientLayout_Stage1_c0_c0_c0_c0_c0(_input);
}
half4 UnrolledBinaryGradientColorizer_Stage1_c0_c0_c0_c1(half4 _input, float2 _coords)
{
    half t = half(_coords.x);
    float4 scale;
    float4 bias;
    if (4 <= 4 || t < uthresholds1_7_Stage1_c0_c0_c0_c1.w)
    {
        if (4 <= 2 || t < uthresholds1_7_Stage1_c0_c0_c0_c1.y)
        {
            if (4 <= 1 || t < uthresholds1_7_Stage1_c0_c0_c0_c1.x)
            {
                scale = uscale0_1_Stage1_c0_c0_c0_c1;
                bias = ubias0_1_Stage1_c0_c0_c0_c1;
            }
            else
            {
                scale = uscale2_3_Stage1_c0_c0_c0_c1;
                bias = ubias2_3_Stage1_c0_c0_c0_c1;
            }
        }
        else
        {
            if (4 <= 3 || t < uthresholds1_7_Stage1_c0_c0_c0_c1.z)
            {
                scale = uscale4_5_Stage1_c0_c0_c0_c1;
                bias = ubias4_5_Stage1_c0_c0_c0_c1;
            }
            else
            {
                scale = uscale6_7_Stage1_c0_c0_c0_c1;
                bias = ubias6_7_Stage1_c0_c0_c0_c1;
            }
        }
    }
    else
    {
        if (4 <= 6 || t < uthresholds9_13_Stage1_c0_c0_c0_c1.y)
        {
            if (4 <= 5 || t < uthresholds9_13_Stage1_c0_c0_c0_c1.x)
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
            if (4 <= 7 || t < uthresholds9_13_Stage1_c0_c0_c0_c1.z)
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
    return half4(float(t) * scale + bias);
}
half4 ClampedGradientEffect_Stage1_c0_c0_c0(half4 _input)
{
    half4 t = MatrixEffect_Stage1_c0_c0_c0_c0(_input);
    half4 outColor;
    if (!false && t.y < 0.0)
    {
        outColor = half4(0.0);
    }
    else if (t.x < 0.0)
    {
        outColor = uleftBorderColor_Stage1_c0_c0_c0;
    }
    else if (t.x > 1.0)
    {
        outColor = urightBorderColor_Stage1_c0_c0_c0;
    }
    else
    {
        outColor = UnrolledBinaryGradientColorizer_Stage1_c0_c0_c0_c1(_input, float2(half2(t.x, 0.0)));
    }
    @if (false)
    {
        outColor.xyz *= outColor.w;
    }
    return outColor;
}
half4 OverrideInputFragmentProcessor_Stage1_c0_c0(half4 _input)
{
    return ClampedGradientEffect_Stage1_c0_c0_c0(false ? half4(0) : half4(1.000000, 1.000000, 1.000000, 1.000000));
}
half4 DitherEffect_Stage1_c0(half4 _input)
{
    half4 color = OverrideInputFragmentProcessor_Stage1_c0_c0(_input);
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
    return half4(clamp(color.xyz + value * urange_Stage1_c0, 0.0, color.w), color.w);
}
void main()
{
    // Stage 0, QuadPerEdgeAAGeometryProcessor
    half4 outputColor_Stage0;
    outputColor_Stage0 = vcolor_Stage0;
    const half4 outputCoverage_Stage0 = half4(1);
    half4 output_Stage1;
    output_Stage1 = DitherEffect_Stage1_c0(outputColor_Stage0);
    {
        // Xfer Processor: Porter Duff
        sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
}
)");

// This fragment shader is taken from GM_BlurDrawImage.
COMPILER_BENCH(medium, R"(
layout(set=0, binding=0) uniform float3x3 umatrix_Stage1_c0_c0_c0;
layout(set=0, binding=0) uniform half4 urectH_Stage2_c1;
layout(set=0, binding=0) uniform float3x3 umatrix_Stage2_c1_c0;
layout(set=0, binding=0) uniform sampler2D uTextureSampler_0_Stage1;
layout(set=0, binding=0) uniform sampler2D uTextureSampler_0_Stage2;
flat in half4 vcolor_Stage0;
noperspective in float2 vTransformedCoords_0_Stage0;
out half4 sk_FragColor;
half4 TextureEffect_Stage1_c0_c0_c0_c0(half4 _input)
{
    return sample(uTextureSampler_0_Stage1, vTransformedCoords_0_Stage0);
}
half4 MatrixEffect_Stage1_c0_c0_c0(half4 _input)
{
    return TextureEffect_Stage1_c0_c0_c0_c0(_input);
}
half4 Blend_Stage1_c0_c0(half4 _input)
{
    // Blend mode: SrcIn (Compose-One behavior)
    return blend_src_in(MatrixEffect_Stage1_c0_c0_c0(half4(1)), _input);
}
half4 OverrideInputFragmentProcessor_Stage1_c0(half4 _input)
{
    return Blend_Stage1_c0_c0(false ? half4(0) : half4(1.000000, 1.000000, 1.000000, 1.000000));
}
half4 TextureEffect_Stage2_c1_c0_c0(half4 _input, float2 _coords)
{
    return sample(uTextureSampler_0_Stage2, _coords).000r;
}
half4 MatrixEffect_Stage2_c1_c0(half4 _input, float2 _coords)
{
    return TextureEffect_Stage2_c1_c0_c0(_input, ((umatrix_Stage2_c1_c0) * _coords.xy1).xy);
}
half4 RectBlurEffect_Stage2_c1(half4 _input)
{
    /* key */ const bool highPrecision = false;
    half xCoverage;
    half yCoverage;
    float2 pos = sk_FragCoord.xy;
    @if (false)
    {
        pos = (float3x3(1) * float3(pos, 1.0)).xy;
    }
    @if (true)
    {
        half2 xy;
        @if (highPrecision)
        {
            xy = max(half2(float4(0).xy - pos), half2(pos - float4(0).zw));
        }
        else
        {
            xy = max(half2(float2(urectH_Stage2_c1.xy) - pos), half2(pos - float2(urectH_Stage2_c1.zw)));
        }
        xCoverage = MatrixEffect_Stage2_c1_c0(_input, float2(half2(xy.x, 0.5))).w;
        yCoverage = MatrixEffect_Stage2_c1_c0(_input, float2(half2(xy.y, 0.5))).w;
    }
    else
    {
        half4 rect;
        @if (highPrecision)
        {
            rect.xy = half2(float4(0).xy - pos);
            rect.zw = half2(pos - float4(0).zw);
        }
        else
        {
            rect.xy = half2(float2(urectH_Stage2_c1.xy) - pos);
            rect.zw = half2(pos - float2(urectH_Stage2_c1.zw));
        }
        xCoverage = (1.0 - MatrixEffect_Stage2_c1_c0(_input, float2(half2(rect.x, 0.5))).w) - MatrixEffect_Stage2_c1_c0(_input, float2(half2(rect.z, 0.5))).w;
        yCoverage = (1.0 - MatrixEffect_Stage2_c1_c0(_input, float2(half2(rect.y, 0.5))).w) - MatrixEffect_Stage2_c1_c0(_input, float2(half2(rect.w, 0.5))).w;
    }
    return (_input * xCoverage) * yCoverage;
}
void main()
{
    // Stage 0, QuadPerEdgeAAGeometryProcessor
    half4 outputColor_Stage0;
    outputColor_Stage0 = vcolor_Stage0;
    const half4 outputCoverage_Stage0 = half4(1);
    half4 output_Stage1;
    output_Stage1 = OverrideInputFragmentProcessor_Stage1_c0(outputColor_Stage0);
    half4 output_Stage2;
    output_Stage2 = RectBlurEffect_Stage2_c1(outputCoverage_Stage0);
    {
        // Xfer Processor: Porter Duff
        sk_FragColor = output_Stage1 * output_Stage2;
    }
}
)");

// This is the fragment shader used to blit the Viewer window when running the software rasterizer.
COMPILER_BENCH(small, R"(
layout(set=0, binding=0) uniform float3x3 umatrix_Stage1_c0_c0;
layout(set=0, binding=0) uniform sampler2D uTextureSampler_0_Stage1;
noperspective in float2 vTransformedCoords_0_Stage0;
out half4 sk_FragColor;
half4 TextureEffect_Stage1_c0_c0_c0(half4 _input)
{
    return sample(uTextureSampler_0_Stage1, vTransformedCoords_0_Stage0);
}
half4 MatrixEffect_Stage1_c0_c0(half4 _input)
{
    return TextureEffect_Stage1_c0_c0_c0(_input);
}
half4 Blend_Stage1_c0(half4 _input)
{
    // Blend mode: Modulate (Compose-One behavior)
    return blend_modulate(MatrixEffect_Stage1_c0_c0(half4(1)), _input);
}
void main()
{
    // Stage 0, QuadPerEdgeAAGeometryProcessor
    half4 outputColor_Stage0 = half4(1);
    const half4 outputCoverage_Stage0 = half4(1);
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
        compiler.moduleForProgramKind(SkSL::ProgramKind::kVertex);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kFragment);
        int after = heap_bytes_used();
        bench("sksl_compiler_gpu", after - before);
    }

    // Heap used by a compiler with the runtime effect module loaded
    {
        int before = heap_bytes_used();
        GrShaderCaps caps(GrContextOptions{});
        SkSL::Compiler compiler(&caps);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeEffect);
        int after = heap_bytes_used();
        bench("sksl_compiler_runtimeeffect", after - before);
    }
}

#else

void RunSkSLMemoryBenchmarks(NanoJSONResultsWriter*) {}

#endif
