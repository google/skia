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
#include "src/sksl/SkSLDSLParser.h"

class SkSLCompilerStartupBench : public Benchmark {
protected:
    const char* onGetName() override {
        return "sksl_compiler_startup";
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        GrShaderCaps caps;
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
            fSettings.fDSLMangling = false;
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
            std::unique_ptr<SkSL::Program> program = SkSL::DSLParser(&fCompiler,
                                                                     fSettings,
                                                                     SkSL::ProgramKind::kFragment,
                                                                     fSrc).program();
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

///////////////////////////////////////////////////////////////////////////////

#define COMPILER_BENCH(name, text)                                                               \
static constexpr char name ## _SRC[] = text;                                                     \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/false, Output::kNone);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kNone);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kGLSL);)  \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kMetal);) \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSPIRV);)

// This fragment shader is from the third tile on the top row of GM_gradients_2pt_conical_outside.
COMPILER_BENCH(large, R"(
uniform float3x3 umatrix_S1_c0;
uniform half4 uthresholds_S1_c1_c0_c0[1];
uniform float4 uscale_S1_c1_c0_c0[4];
uniform float4 ubias_S1_c1_c0_c0[4];
uniform half uinvR1_S1_c1_c0_c1_c0;
uniform half ufx_S1_c1_c0_c1_c0;
uniform float3x3 umatrix_S1_c1_c0_c1;
uniform half4 uleftBorderColor_S1_c1_c0;
uniform half4 urightBorderColor_S1_c1_c0;
uniform half urange_S1;
uniform sampler2D uTextureSampler_0_S1;
flat in half4 vcolor_S0;
noperspective in float2 vTransformedCoords_8_S0;
out half4 sk_FragColor;
half4 TextureEffect_S1_c0_c0(half4 _input, float2 _coords)
{
	return sample(uTextureSampler_0_S1, _coords).000r;
}
half4 MatrixEffect_S1_c0(half4 _input, float2 _coords)
{
	return TextureEffect_S1_c0_c0(_input, float3x2(umatrix_S1_c0) * _coords.xy1);
}
half4 LoopingBinaryColorizer_S1_c1_c0_c0(half4 _input, float2 _coords)
{
	half4 _tmp_0_inColor = _input;
	float2 _tmp_1_coords = _coords;
	half t = half(_tmp_1_coords.x);
	;
	;
	int chunk = 0;
	;
	int pos;
	if (t < uthresholds_S1_c1_c0_c0[chunk].y)
	{
		pos = int(t < uthresholds_S1_c1_c0_c0[chunk].x ? 0 : 1);
	}
	else
	{
		pos = int(t < uthresholds_S1_c1_c0_c0[chunk].z ? 2 : 3);
	}
	;
	return half4(half4(float(t) * uscale_S1_c1_c0_c0[pos] + ubias_S1_c1_c0_c0[pos]));
}
half4 TwoPointConicalFocalLayout_S1_c1_c0_c1_c0(half4 _input)
{
	half4 _tmp_2_inColor = _input;
	float2 _tmp_3_coords = vTransformedCoords_8_S0;
	float t = -1.0;
	half v = 1.0;
	float x_t = -1.0;
	if (bool(int(0)))
	{
		x_t = dot(_tmp_3_coords, _tmp_3_coords) / _tmp_3_coords.x;
	}
	else if (bool(int(0)))
	{
		x_t = length(_tmp_3_coords) - _tmp_3_coords.x * float(uinvR1_S1_c1_c0_c1_c0);
	}
	else
	{
		float temp = _tmp_3_coords.x * _tmp_3_coords.x - _tmp_3_coords.y * _tmp_3_coords.y;
		if (temp >= 0.0)
		{
			if (bool(int(0)) || !bool(int(1)))
			{
				x_t = -sqrt(temp) - _tmp_3_coords.x * float(uinvR1_S1_c1_c0_c1_c0);
			}
			else
			{
				x_t = sqrt(temp) - _tmp_3_coords.x * float(uinvR1_S1_c1_c0_c1_c0);
			}
		}
	}
	if (!bool(int(0)))
	{
		if (x_t <= 0.0)
		{
			v = -1.0;
		}
	}
	if (bool(int(1)))
	{
		if (bool(int(0)))
		{
			t = x_t;
		}
		else
		{
			t = x_t + float(ufx_S1_c1_c0_c1_c0);
		}
	}
	else
	{
		if (bool(int(0)))
		{
			t = -x_t;
		}
		else
		{
			t = -x_t + float(ufx_S1_c1_c0_c1_c0);
		}
	}
	if (bool(int(0)))
	{
		t = 1.0 - t;
	}
	return half4(half4(half(t), v, 0.0, 0.0));
}
half4 MatrixEffect_S1_c1_c0_c1(half4 _input)
{
	return TwoPointConicalFocalLayout_S1_c1_c0_c1_c0(_input);
}
half4 ClampedGradient_S1_c1_c0(half4 _input)
{
	half4 _tmp_4_inColor = _input;
	half4 t = MatrixEffect_S1_c1_c0_c1(_tmp_4_inColor);
	half4 outColor;
	if (!bool(int(0)) && t.y < 0.0)
	{
		outColor = half4(0.0);
	}
	else if (t.x < 0.0)
	{
		outColor = uleftBorderColor_S1_c1_c0;
	}
	else if (t.x > 1.0)
	{
		outColor = urightBorderColor_S1_c1_c0;
	}
	else
	{
		outColor = LoopingBinaryColorizer_S1_c1_c0_c0(_tmp_4_inColor, float2(half2(t.x, 0.0)));
	}
	if (bool(int(0)))
	{
		outColor.xyz *= outColor.w;
	}
	return half4(outColor);
}
half4 DisableCoverageAsAlpha_S1_c1(half4 _input)
{
	_input = ClampedGradient_S1_c1_c0(_input);
	half4 _tmp_5_inColor = _input;
	return half4(_input);
}
half4 Dither_S1(half4 _input)
{
	_input = DisableCoverageAsAlpha_S1_c1(_input);
	half4 _tmp_6_inColor = _input;
	half value = MatrixEffect_S1_c0(_tmp_6_inColor, sk_FragCoord.xy).w - 0.5;
	return half4(half4(clamp(_input.xyz + value * urange_S1, 0.0, _input.w), _input.w));
}
void main()
{
	// Stage 0, QuadPerEdgeAAGeometryProcessor
	half4 outputColor_S0;
	outputColor_S0 = vcolor_S0;
	const half4 outputCoverage_S0 = half4(1);
	half4 output_S1;
	output_S1 = Dither_S1(outputColor_S0);
	{
		// Xfer Processor: Porter Duff
		sk_FragColor = output_S1 * outputCoverage_S0;
	}
}
)");

// This fragment shader is taken from GM_BlurDrawImage.
COMPILER_BENCH(medium, R"(
uniform float3x3 umatrix_S1_c0;
uniform float3x3 umatrix_S2_c0_c0;
uniform float4 urect_S2_c0;
uniform sampler2D uTextureSampler_0_S1;
uniform sampler2D uTextureSampler_0_S2;
flat in half4 vcolor_S0;
noperspective in float2 vTransformedCoords_3_S0;
out half4 sk_FragColor;
half4 TextureEffect_S1_c0_c0(half4 _input)
{
	return sample(uTextureSampler_0_S1, vTransformedCoords_3_S0);
}
half4 MatrixEffect_S1_c0(half4 _input)
{
	return TextureEffect_S1_c0_c0(_input);
}
half4 DisableCoverageAsAlpha_S1(half4 _input)
{
	_input = MatrixEffect_S1_c0(_input);
	half4 _tmp_0_inColor = _input;
	return half4(_input);
}
half4 TextureEffect_S2_c0_c0_c0(half4 _input, float2 _coords)
{
	return sample(uTextureSampler_0_S2, _coords).000r;
}
half4 MatrixEffect_S2_c0_c0(half4 _input, float2 _coords)
{
	return TextureEffect_S2_c0_c0_c0(_input, float3x2(umatrix_S2_c0_c0) * _coords.xy1);
}
half4 RectBlur_S2_c0(half4 _input, float2 _coords)
{
	half4 _tmp_1_inColor = _input;
	float2 _tmp_2_coords = _coords;
	half xCoverage;
	half yCoverage;
	if (bool(int(1)))
	{
		half2 xy = max(half2(urect_S2_c0.xy - _tmp_2_coords), half2(_tmp_2_coords - urect_S2_c0.zw));
		xCoverage = MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(xy.x, 0.5))).w;
		yCoverage = MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(xy.y, 0.5))).w;
	}
	else
	{
		half4 rect = half4(half2(urect_S2_c0.xy - _tmp_2_coords), half2(_tmp_2_coords - urect_S2_c0.zw));
		xCoverage = (1.0 - MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(rect.x, 0.5))).w) - MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(rect.z, 0.5))).w;
		yCoverage = (1.0 - MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(rect.y, 0.5))).w) - MatrixEffect_S2_c0_c0(_tmp_1_inColor, float2(half2(rect.w, 0.5))).w;
	}
	return half4((_input * xCoverage) * yCoverage);
}
half4 DeviceSpace_S2(half4 _input)
{
	return RectBlur_S2_c0(_input, sk_FragCoord.xy);
}
void main()
{
	// Stage 0, QuadPerEdgeAAGeometryProcessor
	half4 outputColor_S0;
	outputColor_S0 = vcolor_S0;
	const half4 outputCoverage_S0 = half4(1);
	half4 output_S1;
	output_S1 = DisableCoverageAsAlpha_S1(outputColor_S0);
	half4 output_S2;
	output_S2 = DeviceSpace_S2(outputCoverage_S0);
	{
		// Xfer Processor: Porter Duff
		sk_FragColor = output_S1 * output_S2;
	}
}
)");

// This is the fragment shader used to blit the Viewer window when running the software rasterizer.
COMPILER_BENCH(small, R"(
uniform float3x3 umatrix_S1_c0;
uniform sampler2D uTextureSampler_0_S1;
flat in half4 vcolor_S0;
noperspective in float2 vTransformedCoords_3_S0;
out half4 sk_FragColor;
half4 TextureEffect_S1_c0_c0(half4 _input)
{
	return sample(uTextureSampler_0_S1, vTransformedCoords_3_S0);
}
half4 MatrixEffect_S1_c0(half4 _input)
{
	return TextureEffect_S1_c0_c0(_input);
}
half4 DisableCoverageAsAlpha_S1(half4 _input)
{
	_input = MatrixEffect_S1_c0(_input);
	half4 _tmp_0_inColor = _input;
	return half4(_input);
}
void main()
{
	// Stage 0, QuadPerEdgeAAGeometryProcessor
	half4 outputColor_S0;
	outputColor_S0 = vcolor_S0;
	const half4 outputCoverage_S0 = half4(1);
	half4 output_S1;
	output_S1 = DisableCoverageAsAlpha_S1(outputColor_S0);
	{
		// Xfer Processor: Porter Duff
		sk_FragColor = output_S1 * outputCoverage_S0;
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
        GrShaderCaps caps;
        SkSL::Compiler compiler(&caps);
        int after = heap_bytes_used();
        bench("sksl_compiler_baseline", after - before);
    }

    // Heap used by a compiler with the two main GPU modules (fragment + vertex) loaded
    {
        int before = heap_bytes_used();
        GrShaderCaps caps;
        SkSL::Compiler compiler(&caps);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kVertex);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kFragment);
        int after = heap_bytes_used();
        bench("sksl_compiler_gpu", after - before);
    }

    // Heap used by a compiler with the runtime shader, color filter and blending modules loaded
    {
        int before = heap_bytes_used();
        GrShaderCaps caps;
        SkSL::Compiler compiler(&caps);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeColorFilter);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeShader);
        compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeBlender);
        int after = heap_bytes_used();
        bench("sksl_compiler_runtimeeffect", after - before);
    }
}

#else

void RunSkSLMemoryBenchmarks(NanoJSONResultsWriter*) {}

#endif
