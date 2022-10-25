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
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/mock/GrMockCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <regex>

#include "src/sksl/generated/sksl_shared.minified.sksl"
#include "src/sksl/generated/sksl_compute.minified.sksl"
#include "src/sksl/generated/sksl_frag.minified.sksl"
#include "src/sksl/generated/sksl_gpu.minified.sksl"
#include "src/sksl/generated/sksl_public.minified.sksl"
#include "src/sksl/generated/sksl_rt_shader.minified.sksl"
#include "src/sksl/generated/sksl_vert.minified.sksl"
#if defined(SK_GRAPHITE_ENABLED)
#include "src/sksl/generated/sksl_graphite_frag.minified.sksl"
#include "src/sksl/generated/sksl_graphite_vert.minified.sksl"
#endif

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
    kSPIRV,
    kSkVM,     // raw SkVM bytecode
    kSkVMOpt,  // optimized SkVM bytecode
    kSkVMJIT,  // optimized native assembly code
};

class SkSLCompileBench : public Benchmark {
public:
    static const char* output_string(Output output) {
        switch (output) {
            case Output::kNone:    return "";
            case Output::kGLSL:    return "glsl_";
            case Output::kMetal:   return "metal_";
            case Output::kSPIRV:   return "spirv_";
            case Output::kSkVM:    return "skvm_";
            case Output::kSkVMOpt: return "skvm_opt_";
            case Output::kSkVMJIT: return "skvm_jit_";
        }
        SkUNREACHABLE;
    }

    SkSLCompileBench(std::string name, const char* src, bool optimize, Output output)
            : fName(std::string("sksl_") + (optimize ? "" : "unoptimized_") +
                    output_string(output) + name)
            , fSrc(src)
            , fCaps(GrContextOptions(), GrMockOptions())
            , fCompiler(fCaps.shaderCaps())
            , fOutput(output) {
        fSettings.fOptimize = optimize;
        // The test programs we compile don't follow Vulkan rules and thus produce invalid SPIR-V.
        // This is harmless, so long as we don't try to validate them.
        fSettings.fValidateSPIRV = false;

        this->fixUpSource();
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    bool usesRuntimeShader() const {
        return fOutput >= Output::kSkVM;
    }

    void fixUpSource() {
        auto fixup = [this](const char* input, const char* replacement) {
            fSrc = std::regex_replace(fSrc, std::regex(input), replacement);
        };

        // Runtime shaders which have slightly different conventions than fragment shaders.
        // Perform a handful of fixups to compensate. These are hand-tuned for our current set of
        // test shaders and will probably need to be updated if we add more.
        if (this->usesRuntimeShader()) {
            fixup(R"(void main\(\))",                              "half4 main(float2 xy)");
            fixup(R"(sk_FragColor =)",                             "return");
            fixup(R"(sk_FragCoord)",                               "_FragCoord");
            fixup(R"(uniform sampler2D )",                         "uniform shader ");
            fixup(R"((flat |noperspective |)in )",                 "uniform ");
            fixup(R"(sample\(([A-Za-z0-9_]+), ([A-Za-z0-9_]+)\))", "$01.eval($02)");
            fSrc = "#version 300\nuniform float4 _FragCoord;\n" + fSrc;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkSL::ProgramKind kind = this->usesRuntimeShader() ? SkSL::ProgramKind::kRuntimeShader
                                                                 : SkSL::ProgramKind::kFragment;
        for (int i = 0; i < loops; i++) {
            std::unique_ptr<SkSL::Program> program = fCompiler.convertProgram(kind, fSrc,
                                                                              fSettings);
            if (fCompiler.errorCount()) {
                SK_ABORT("shader compilation failed: %s\n", fCompiler.errorText().c_str());
            }
            std::string result;
            switch (fOutput) {
                case Output::kNone:    break;
                case Output::kGLSL:    SkAssertResult(fCompiler.toGLSL(*program,  &result)); break;
                case Output::kMetal:   SkAssertResult(fCompiler.toMetal(*program, &result)); break;
                case Output::kSPIRV:   SkAssertResult(fCompiler.toSPIRV(*program, &result)); break;
                case Output::kSkVM:
                case Output::kSkVMOpt:
                case Output::kSkVMJIT: SkAssertResult(CompileToSkVM(*program, fOutput)); break;
            }
        }
    }

    static bool CompileToSkVM(const SkSL::Program& program, Output mode) {
        const bool optimize = (mode >= Output::kSkVMOpt);
        const bool allowJIT = (mode >= Output::kSkVMJIT);
        skvm::Builder builder{skvm::Features{}};
        if (!SkSL::testingOnly_ProgramToSkVMShader(program, &builder, /*debugTrace=*/nullptr)) {
            return false;
        }
        if (optimize) {
            builder.done("SkSLBench", allowJIT);
        }
        return true;
    }

private:
    std::string fName;
    std::string fSrc;
    GrMockCaps fCaps;
    SkSL::Compiler fCompiler;
    SkSL::ProgramSettings fSettings;
    Output fOutput;

    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

#define COMPILER_BENCH(name, text)                                                                 \
static constexpr char name ## _SRC[] = text;                                                       \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/false, Output::kNone);)    \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kNone);)    \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kGLSL);)    \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kMetal);)   \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSPIRV);)   \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSkVM);)    \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSkVMOpt);) \
DEF_BENCH(return new SkSLCompileBench(#name, name ## _SRC, /*optimize=*/true,  Output::kSkVMJIT);)

// This fragment shader is from the third tile on the top row of GM_gradients_2pt_conical_outside.
// To get an ES2 compatible shader, nonconstantArrayIndexSupport in GrShaderCaps is forced off.
COMPILER_BENCH(large, R"(
uniform float3x3 umatrix_S1_c0;
uniform half4 uthresholds1_7_S1_c1_c0_c0;
uniform half4 uthresholds9_13_S1_c1_c0_c0;
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
half4 TextureEffect_S1_c0_c0(half4 _input, float2 _coords)
{
	return sample(uTextureSampler_0_S1, _coords).000r;
}
half4 MatrixEffect_S1_c0(half4 _input, float2 _coords)
{
	return TextureEffect_S1_c0_c0(_input, float3x2(umatrix_S1_c0) * _coords.xy1);
}
half4 UnrolledBinaryColorizer_S1_c1_c0_c0(half4 _input, float2 _coords)
{
	half4 _tmp_0_inColor = _input;
	float2 _tmp_1_coords = _coords;
	half t = half(_tmp_1_coords.x);
	float4 s;
	float4 b;
	{
		if (t < uthresholds1_7_S1_c1_c0_c0.y)
		{
			if (t < uthresholds1_7_S1_c1_c0_c0.x)
			{
				s = uscale_S1_c1_c0_c0[0];
				b = ubias_S1_c1_c0_c0[0];
			}
			else
			{
				s = uscale_S1_c1_c0_c0[1];
				b = ubias_S1_c1_c0_c0[1];
			}
		}
		else
		{
			if (t < uthresholds1_7_S1_c1_c0_c0.z)
			{
				s = uscale_S1_c1_c0_c0[2];
				b = ubias_S1_c1_c0_c0[2];
			}
			else
			{
				s = uscale_S1_c1_c0_c0[3];
				b = ubias_S1_c1_c0_c0[3];
			}
		}
	}
	return half4(half4(float(t) * s + b));
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
		outColor = UnrolledBinaryColorizer_S1_c1_c0_c0(_tmp_4_inColor, float2(half2(t.x, 0.0)));
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

// This fragment shader is taken from GM_lcdtext.
COMPILER_BENCH(small, R"(
uniform sampler2D uTextureSampler_0_S0;
noperspective in float2 vTextureCoords_S0;
flat in float vTexIndex_S0;
noperspective in half4 vinColor_S0;
void main()
{
	// Stage 0, BitmapText
	half4 outputColor_S0;
	outputColor_S0 = vinColor_S0;
	half4 texColor;
	{
		texColor = sample(uTextureSampler_0_S0, vTextureCoords_S0).rrrr;
	}
	half4 outputCoverage_S0 = texColor;
	{
		// Xfer Processor: Porter Duff
		sk_FragColor = outputColor_S0 * outputCoverage_S0;
	}
}
)");

COMPILER_BENCH(tiny, "void main() { sk_FragColor = half4(1); }");

#if defined(SK_BUILD_FOR_UNIX)

#include <malloc.h>
static int64_t heap_bytes_used() {
    return (int64_t)mallinfo().uordblks;
}

#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include <malloc/malloc.h>
static int64_t heap_bytes_used() {
    malloc_statistics_t stats;
    malloc_zone_pressure_relief(malloc_default_zone(), 0);
    malloc_zone_statistics(malloc_default_zone(), &stats);
    return (int64_t)stats.size_in_use;
}

#else

static int64_t heap_bytes_used() {
    return -1;
}

#endif

static void bench(NanoJSONResultsWriter* log, const char* name, int bytes) {
    SkDEBUGCODE(SkDebugf("%s: %d bytes\n", name, bytes);)
    log->beginObject(name);          // test
    log->beginObject("meta");        //   config
    log->appendS32("bytes", bytes);  //     sub_result
    log->endObject();                //   config
    log->endObject();                // test
}

// These benchmarks aren't timed, they produce memory usage statistics. They run standalone, and
// directly add their results to the nanobench log.
void RunSkSLModuleBenchmarks(NanoJSONResultsWriter* log) {
    // Heap used by a default compiler (with no modules loaded)
    int64_t before = heap_bytes_used();
    GrShaderCaps caps;
    SkSL::Compiler compiler(&caps);
    int baselineBytes = heap_bytes_used();
    if (baselineBytes >= 0) {
        baselineBytes = (baselineBytes - before);
        bench(log, "sksl_compiler_baseline", baselineBytes);
    }

    // Heap used by a compiler with the two main GPU modules (fragment + vertex) and runtime effects
    // (shader + color filter + blender) loaded. Ganesh will load all of these in regular usage.
    before = heap_bytes_used();
    compiler.moduleForProgramKind(SkSL::ProgramKind::kVertex);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kFragment);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeColorFilter);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeShader);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kRuntimeBlender);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kPrivateRuntimeColorFilter);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kPrivateRuntimeShader);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kPrivateRuntimeBlender);
    int64_t gpuBytes = heap_bytes_used();
    if (gpuBytes >= 0) {
        gpuBytes = (gpuBytes - before) + baselineBytes;
        bench(log, "sksl_compiler_gpu", gpuBytes);
    }

#ifdef SK_GRAPHITE_ENABLED
    // Heap used by a compiler with the Graphite modules loaded.
    before = heap_bytes_used();
    compiler.moduleForProgramKind(SkSL::ProgramKind::kGraphiteVertex);
    compiler.moduleForProgramKind(SkSL::ProgramKind::kGraphiteFragment);
    int64_t graphiteBytes = heap_bytes_used();
    if (graphiteBytes >= 0) {
        graphiteBytes = (graphiteBytes - before) + gpuBytes;
        bench(log, "sksl_compiler_graphite", graphiteBytes);
    }
#endif

    // Heap used by a compiler with compute-shader support loaded.
    before = heap_bytes_used();
    compiler.moduleForProgramKind(SkSL::ProgramKind::kCompute);
    int64_t computeBytes = heap_bytes_used();
    if (computeBytes >= 0) {
        computeBytes = (computeBytes - before) + baselineBytes;
        bench(log, "sksl_compiler_compute", computeBytes);
    }

    // Report the minified module sizes.
    int compilerGPUBinarySize = std::size(SKSL_MINIFIED_sksl_shared) +
                                std::size(SKSL_MINIFIED_sksl_gpu) +
                                std::size(SKSL_MINIFIED_sksl_vert) +
                                std::size(SKSL_MINIFIED_sksl_frag) +
                                std::size(SKSL_MINIFIED_sksl_public) +
                                std::size(SKSL_MINIFIED_sksl_rt_shader);
    bench(log, "sksl_binary_size_gpu", compilerGPUBinarySize);

#if defined(SK_GRAPHITE_ENABLED)
    int compilerGraphiteBinarySize = std::size(SKSL_MINIFIED_sksl_graphite_frag) +
                                     std::size(SKSL_MINIFIED_sksl_graphite_vert);
    bench(log, "sksl_binary_size_graphite", compilerGraphiteBinarySize);
#endif

    int compilerComputeBinarySize = std::size(SKSL_MINIFIED_sksl_compute);
    bench(log, "sksl_binary_size_compute", compilerComputeBinarySize);
}

class SkSLModuleLoaderBench : public Benchmark {
public:
    SkSLModuleLoaderBench(const char* name, std::vector<SkSL::ProgramKind> moduleList)
            : fName(name), fModuleList(std::move(moduleList)) {}

    const char* onGetName() override {
        return fName;
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    int calculateLoops(int defaultLoops) const override {
        return 1;
    }

    void onPreDraw(SkCanvas*) override {
        SkSL::ModuleLoader::Get().unloadModules();
    }

    void onDraw(int loops, SkCanvas*) override {
        SkASSERT(loops == 1);
        GrShaderCaps caps;
        SkSL::Compiler compiler(&caps);
        for (SkSL::ProgramKind kind : fModuleList) {
            compiler.moduleForProgramKind(kind);
        }
    }

    const char* fName;
    std::vector<SkSL::ProgramKind> fModuleList;
};

DEF_BENCH(return new SkSLModuleLoaderBench("sksl_module_loader_ganesh",
                                           {
                                                   SkSL::ProgramKind::kVertex,
                                                   SkSL::ProgramKind::kFragment,
                                                   SkSL::ProgramKind::kRuntimeColorFilter,
                                                   SkSL::ProgramKind::kRuntimeShader,
                                                   SkSL::ProgramKind::kRuntimeBlender,
                                                   SkSL::ProgramKind::kPrivateRuntimeColorFilter,
                                                   SkSL::ProgramKind::kPrivateRuntimeShader,
                                                   SkSL::ProgramKind::kPrivateRuntimeBlender,
                                                   SkSL::ProgramKind::kCompute,
                                           });)

DEF_BENCH(return new SkSLModuleLoaderBench("sksl_module_loader_graphite",
                                           {
                                                   SkSL::ProgramKind::kVertex,
                                                   SkSL::ProgramKind::kFragment,
                                                   SkSL::ProgramKind::kRuntimeColorFilter,
                                                   SkSL::ProgramKind::kRuntimeShader,
                                                   SkSL::ProgramKind::kRuntimeBlender,
                                                   SkSL::ProgramKind::kPrivateRuntimeColorFilter,
                                                   SkSL::ProgramKind::kPrivateRuntimeShader,
                                                   SkSL::ProgramKind::kPrivateRuntimeBlender,
                                                   SkSL::ProgramKind::kCompute,
                                                   SkSL::ProgramKind::kGraphiteVertex,
                                                   SkSL::ProgramKind::kGraphiteFragment,
                                           });)
