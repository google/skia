/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "bench/nanobench.h"

#include "bench/AndroidCodecBench.h"
#include "bench/Benchmark.h"
#include "bench/CodecBench.h"
#include "bench/CodecBenchPriv.h"
#include "bench/GMBench.h"
#include "bench/MSKPBench.h"
#include "bench/RecordingBench.h"
#include "bench/ResultsWriter.h"
#include "bench/SKPAnimationBench.h"
#include "bench/SKPBench.h"
#include "bench/SkGlyphCacheBench.h"
#include "bench/SkSLBench.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkMacros.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkLeanWindows.h"
#include "src/base/SkTime.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkShaderUtils.h"
#include "tools/AutoreleasePool.h"
#include "tools/CrashHandler.h"
#include "tools/MSKPPlayer.h"
#include "tools/ProcStats.h"
#include "tools/Stats.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommonFlags.h"
#include "tools/flags/CommonFlagsConfig.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/ios_utils.h"
#include "tools/trace/EventTracingPriv.h"
#include "tools/trace/SkDebugfTracer.h"

#if defined(SK_ENABLE_SVG)
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#endif

#ifdef SK_ENABLE_ANDROID_UTILS
#include "bench/BitmapRegionDecoderBench.h"
#include "client_utils/android/BitmapRegionDecoder.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "tools/GpuToolUtils.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"
#endif

#include <cinttypes>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <thread>

extern bool gSkForceRasterPipelineBlitter;
extern bool gForceHighPrecisionRasterPipeline;

#ifndef SK_BUILD_FOR_WIN
    #include <unistd.h>

#endif

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "tools/gpu/GrContextFactory.h"

using namespace skia_private;

using sk_gpu_test::ContextInfo;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::TestContext;

GrContextOptions grContextOpts;

static const int kAutoTuneLoops = 0;

static SkString loops_help_txt() {
    SkString help;
    help.printf("Number of times to run each bench. Set this to %d to auto-"
                "tune for each bench. Timings are only reported when auto-tuning.",
                kAutoTuneLoops);
    return help;
}

static SkString to_string(int n) {
    SkString str;
    str.appendS32(n);
    return str;
}

static DEFINE_int(loops, kAutoTuneLoops, loops_help_txt().c_str());

static DEFINE_int(samples, 10, "Number of samples to measure for each bench.");
static DEFINE_int(ms, 0, "If >0, run each bench for this many ms instead of obeying --samples.");
static DEFINE_int(overheadLoops, 100000, "Loops to estimate timer overhead.");
static DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
static DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
static DEFINE_int(gpuFrameLag, 5,
                    "If unknown, estimated maximum number of frames GPU allows to lag.");

static DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
static DEFINE_int(maxCalibrationAttempts, 3,
             "Try up to this many times to guess loops for a bench, or skip the bench.");
static DEFINE_int(maxLoops, 1000000, "Never run a bench more times than this.");
static DEFINE_string(clip, "0,0,1000,1000", "Clip for SKPs.");
static DEFINE_string(scales, "1.0", "Space-separated scales for SKPs.");
static DEFINE_string(zoom, "1.0,0",
                     "Comma-separated zoomMax,zoomPeriodMs factors for a periodic SKP zoom "
                     "function that ping-pongs between 1.0 and zoomMax.");
static DEFINE_bool(bbh, true, "Build a BBH for SKPs?");
static DEFINE_bool(loopSKP, true, "Loop SKPs like we do for micro benches?");
static DEFINE_int(flushEvery, 10, "Flush --outResultsFile every Nth run.");
static DEFINE_bool(gpuStats, false, "Print GPU stats after each gpu benchmark?");
static DEFINE_bool(gpuStatsDump, false, "Dump GPU stats after each benchmark to json");
static DEFINE_bool(dmsaaStatsDump, false, "Dump DMSAA stats after each benchmark to json");
static DEFINE_bool(keepAlive, false, "Print a message every so often so that we don't time out");
static DEFINE_bool(csv, false, "Print status in CSV format");
static DEFINE_string(sourceType, "",
        "Apply usual --match rules to source type: bench, gm, skp, image, etc.");
static DEFINE_string(benchType,  "",
        "Apply usual --match rules to bench type: micro, recording, "
        "piping, playback, skcodec, etc.");

static DEFINE_bool(forceRasterPipeline, false, "sets gSkForceRasterPipelineBlitter");
static DEFINE_bool(forceRasterPipelineHP, false, "sets gSkForceRasterPipelineBlitter and gForceHighPrecisionRasterPipeline");

static DEFINE_bool2(pre_log, p, false,
                    "Log before running each test. May be incomprehensible when threading");

static DEFINE_bool(cpu, true, "Run CPU-bound work?");
static DEFINE_bool(gpu, true, "Run GPU-bound work?");
static DEFINE_bool(dryRun, false,
                   "just print the tests that would be run, without actually running them.");
static DEFINE_string(images, "",
                     "List of images and/or directories to decode. A directory with no images"
                     " is treated as a fatal error.");
static DEFINE_bool(simpleCodec, false,
                   "Runs of a subset of the codec tests, always N32, Premul or Opaque");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

static DEFINE_bool2(quiet, q, false, "if true, don't print status updates.");
static DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");


static DEFINE_string(skps, "skps", "Directory to read skps from.");
static DEFINE_string(mskps, "mskps", "Directory to read mskps from.");
static DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");
static DEFINE_string(texttraces, "", "Directory to read TextBlobTrace files from.");

static DEFINE_int_2(threads, j, -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

static DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");

static DEFINE_string(key, "",
                     "Space-separated key/value pairs to add to JSON identifying this builder.");
static DEFINE_string(properties, "",
                     "Space-separated key/value pairs to add to JSON identifying this run.");

static DEFINE_bool(purgeBetweenBenches, false,
                   "Call SkGraphics::PurgeAllCaches() between each benchmark?");

static DEFINE_bool(splitPerfettoTracesByBenchmark, true,
                  "Create separate perfetto trace files for each benchmark?\n"
                  "Will only take effect if perfetto tracing is enabled. See --trace.");

static DEFINE_bool(runtimeCPUDetection, true, "Skip runtime CPU detection and optimization");

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }

static SkString humanize(double ms) {
    if (FLAGS_verbose) return SkStringPrintf("%" PRIu64, (uint64_t)(ms*1e6));
    return HumanizeMs(ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

bool Target::init(SkImageInfo info, Benchmark* bench) {
    if (Benchmark::Backend::kRaster == config.backend) {
        this->surface = SkSurfaces::Raster(info);
        if (!this->surface) {
            return false;
        }
    }
    return true;
}
bool Target::capturePixels(SkBitmap* bmp) {
    SkCanvas* canvas = this->getCanvas();
    if (!canvas) {
        return false;
    }
    bmp->allocPixels(canvas->imageInfo());
    if (!canvas->readPixels(*bmp, 0, 0)) {
        SkDebugf("Can't read canvas pixels.\n");
        return false;
    }
    return true;
}

struct GPUTarget : public Target {
    explicit GPUTarget(const Config& c) : Target(c) {}
    ContextInfo contextInfo;
    std::unique_ptr<GrContextFactory> factory;

    ~GPUTarget() override {
        // For Vulkan we need to release all our refs to the GrContext before destroy the vulkan
        // context which happens at the end of this destructor. Thus we need to release the surface
        // here which holds a ref to the GrContext.
        surface.reset();
    }

    void setup() override {
        this->contextInfo.testContext()->makeCurrent();
        // Make sure we're done with whatever came before.
        this->contextInfo.testContext()->finish();
    }
    void endTiming() override {
        if (this->contextInfo.testContext()) {
            this->contextInfo.testContext()->flushAndWaitOnSync(contextInfo.directContext());
        }
    }
    void syncCPU() override { this->contextInfo.testContext()->finish(); }

    bool needsFrameTiming(int* maxFrameLag) const override {
        if (!this->contextInfo.testContext()->getMaxGpuFrameLag(maxFrameLag)) {
            // Frame lag is unknown.
            *maxFrameLag = FLAGS_gpuFrameLag;
        }
        return true;
    }
    bool init(SkImageInfo info, Benchmark* bench) override {
        GrContextOptions options = grContextOpts;
        bench->modifyGrContextOptions(&options);
        this->factory = std::make_unique<GrContextFactory>(options);
        SkSurfaceProps props(this->config.surfaceFlags, kRGB_H_SkPixelGeometry);
        this->surface = SkSurfaces::RenderTarget(
                this->factory->get(this->config.ctxType, this->config.ctxOverrides),
                skgpu::Budgeted::kNo,
                info,
                this->config.samples,
                &props);
        this->contextInfo =
                this->factory->getContextInfo(this->config.ctxType, this->config.ctxOverrides);
        if (!this->surface) {
            return false;
        }
        if (!this->contextInfo.testContext()->fenceSyncSupport()) {
            SkDebugf("WARNING: GL context for config \"%s\" does not support fence sync. "
                     "Timings might not be accurate.\n", this->config.name.c_str());
        }
        return true;
    }

    void dumpStats() override {
        auto context = this->contextInfo.directContext();

        context->priv().printCacheStats();
        context->priv().printGpuStats();
        context->priv().printContextStats();
    }
};

#if defined(SK_GRAPHITE)
struct GraphiteTarget : public Target {
    explicit GraphiteTarget(const Config& c) : Target(c) {}
    using TestContext = skiatest::graphite::GraphiteTestContext;
    using ContextFactory = skiatest::graphite::ContextFactory;

    std::unique_ptr<ContextFactory> factory;

    TestContext* testContext;
    skgpu::graphite::Context* context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;

    ~GraphiteTarget() override {
        // For Vulkan we need to release all our refs before we destroy the vulkan context which
        // happens at the end of this destructor. Thus we need to release the surface here which
        // holds a ref to the Graphite device
        surface.reset();
    }

    void setup() override {}

    void endTiming() override {
        if (context && recorder) {
            std::unique_ptr<skgpu::graphite::Recording> recording = this->recorder->snap();
            if (recording) {
                this->testContext->submitRecordingAndWaitOnSync(this->context, recording.get());
            }
        }
    }
    void syncCPU() override {
        if (context && recorder) {
            // TODO: have a way to sync work with out submitting a Recording which is currently
            // required. Probably need to get to the point where the backend command buffers are
            // stored on the Context and not Recordings before this is feasible.
            std::unique_ptr<skgpu::graphite::Recording> recording = this->recorder->snap();
            if (recording) {
                skgpu::graphite::InsertRecordingInfo info;
                info.fRecording = recording.get();
                this->context->insertRecording(info);
            }
            this->context->submit(skgpu::graphite::SyncToCpu::kYes);
        }
    }

    bool needsFrameTiming(int* maxFrameLag) const override {
        SkAssertResult(this->testContext->getMaxGpuFrameLag(maxFrameLag));
        return true;
    }
    bool init(SkImageInfo info, Benchmark* bench) override {
        GrContextOptions options = grContextOpts;
        bench->modifyGrContextOptions(&options);
        // TODO: We should merge Ganesh and Graphite context options and then actually use the
        // context options when we make the factory here.
        this->factory = std::make_unique<ContextFactory>();

        skiatest::graphite::ContextInfo ctxInfo =
                this->factory->getContextInfo(this->config.ctxType);
        if (!ctxInfo.fContext) {
            return false;
        }
        this->testContext = ctxInfo.fTestContext;
        this->context = ctxInfo.fContext;

        this->recorder = this->context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
        if (!this->recorder) {
            return false;
        }

        this->surface = SkSurfaces::RenderTarget(this->recorder.get(), info);
        if (!this->surface) {
            return false;
        }
        // TODO: get fence stuff working
#if 0
        if (!this->contextInfo.testContext()->fenceSyncSupport()) {
            SkDebugf("WARNING: GL context for config \"%s\" does not support fence sync. "
                     "Timings might not be accurate.\n", this->config.name.c_str());
        }
#endif
        return true;
    }

    void dumpStats() override {
    }
};
#endif // SK_GRAPHITE

static double time(int loops, Benchmark* bench, Target* target) {
    SkCanvas* canvas = target->getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    bench->preDraw(canvas);
    double start = now_ms();
    canvas = target->beginTiming(canvas);

    bench->draw(loops, canvas);

    target->endTiming();
    double elapsed = now_ms() - start;
    bench->postDraw(canvas);
    return elapsed;
}

static double estimate_timer_overhead() {
    double overhead = 0;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        double start = now_ms();
        overhead += now_ms() - start;
    }
    return overhead / FLAGS_overheadLoops;
}

static int detect_forever_loops(int loops) {
    // look for a magic run-forever value
    if (loops < 0) {
        loops = SK_MaxS32;
    }
    return loops;
}

static int clamp_loops(int loops) {
    if (loops < 1) {
        SkDebugf("ERROR: clamping loops from %d to 1. "
                 "There's probably something wrong with the bench.\n", loops);
        return 1;
    }
    if (loops > FLAGS_maxLoops) {
        SkDebugf("WARNING: clamping loops from %d to FLAGS_maxLoops, %d.\n", loops, FLAGS_maxLoops);
        return FLAGS_maxLoops;
    }
    return loops;
}

static bool write_canvas_png(Target* target, const SkString& filename) {

    if (filename.isEmpty()) {
        return false;
    }
    if (target->getCanvas() &&
        kUnknown_SkColorType == target->getCanvas()->imageInfo().colorType()) {
        return false;
    }

    SkBitmap bmp;

    if (!target->capturePixels(&bmp)) {
        return false;
    }

    SkString dir = SkOSPath::Dirname(filename.c_str());
    if (!sk_mkdir(dir.c_str())) {
        SkDebugf("Can't make dir %s.\n", dir.c_str());
        return false;
    }
    SkFILEWStream stream(filename.c_str());
    if (!stream.isValid()) {
        SkDebugf("Can't write %s.\n", filename.c_str());
        return false;
    }
    if (!SkPngEncoder::Encode(&stream, bmp.pixmap(), {})) {
        SkDebugf("Can't encode a PNG.\n");
        return false;
    }
    return true;
}

static int kFailedLoops = -2;
static int setup_cpu_bench(const double overhead, Target* target, Benchmark* bench) {
    // First figure out approximately how many loops of bench it takes to make overhead negligible.
    double bench_plus_overhead = 0.0;
    int round = 0;
    int loops = bench->shouldLoop() ? FLAGS_loops : 1;
    if (kAutoTuneLoops == loops) {
        while (bench_plus_overhead < overhead) {
            if (round++ == FLAGS_maxCalibrationAttempts) {
                SkDebugf("WARNING: Can't estimate loops for %s (%s vs. %s); skipping.\n",
                         bench->getUniqueName(), HUMANIZE(bench_plus_overhead), HUMANIZE(overhead));
                return kFailedLoops;
            }
            bench_plus_overhead = time(1, bench, target);
        }
    }

    // Later we'll just start and stop the timer once but loop N times.
    // We'll pick N to make timer overhead negligible:
    //
    //          overhead
    //  -------------------------  < FLAGS_overheadGoal
    //  overhead + N * Bench Time
    //
    // where bench_plus_overhead ~=~ overhead + Bench Time.
    //
    // Doing some math, we get:
    //
    //  (overhead / FLAGS_overheadGoal) - overhead
    //  ------------------------------------------  < N
    //       bench_plus_overhead - overhead)
    //
    // Luckily, this also works well in practice. :)
    if (kAutoTuneLoops == loops) {
        const double numer = overhead / FLAGS_overheadGoal - overhead;
        const double denom = bench_plus_overhead - overhead;
        loops = (int)ceil(numer / denom);
        loops = clamp_loops(loops);
    } else {
        loops = detect_forever_loops(loops);
    }

    return loops;
}

static int setup_gpu_bench(Target* target, Benchmark* bench, int maxGpuFrameLag) {
    // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
    int loops = bench->shouldLoop() ? FLAGS_loops : 1;
    if (kAutoTuneLoops == loops) {
        loops = 1;
        double elapsed = 0;
        do {
            if (1<<30 == loops) {
                // We're about to wrap.  Something's wrong with the bench.
                loops = 0;
                break;
            }
            loops *= 2;
            // If the GPU lets frames lag at all, we need to make sure we're timing
            // _this_ round, not still timing last round.
            for (int i = 0; i < maxGpuFrameLag; i++) {
                elapsed = time(loops, bench, target);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);
        loops = clamp_loops(loops);

        // Make sure we're not still timing our calibration.
        target->syncCPU();
    } else {
        loops = detect_forever_loops(loops);
    }
    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < maxGpuFrameLag; i++) {
        time(loops, bench, target);
    }

    return loops;
}

#define kBogusContextType skgpu::ContextType::kGL
#define kBogusContextOverrides GrContextFactory::ContextOverrides::kNone

static std::optional<Config> create_config(const SkCommandLineConfig* config) {
    if (const auto* gpuConfig = config->asConfigGpu()) {
        if (!FLAGS_gpu) {
            SkDebugf("Skipping config '%s' as requested.\n", config->getTag().c_str());
            return std::nullopt;
        }

        const auto ctxType = gpuConfig->getContextType();
        const auto ctxOverrides = gpuConfig->getContextOverrides();
        const auto sampleCount = gpuConfig->getSamples();
        const auto colorType = gpuConfig->getColorType();
        if (gpuConfig->getSurfType() != SkCommandLineConfigGpu::SurfType::kDefault) {
            SkDebugf("This tool only supports the default surface type.");
            return std::nullopt;
        }

        GrContextFactory factory(grContextOpts);
        if (const auto ctx = factory.get(ctxType, ctxOverrides)) {
            GrBackendFormat format = ctx->defaultBackendFormat(colorType, GrRenderable::kYes);
            int supportedSampleCount =
                    ctx->priv().caps()->getRenderTargetSampleCount(sampleCount, format);
            if (sampleCount != supportedSampleCount) {
                SkDebugf("Configuration '%s' sample count %d is not a supported sample count.\n",
                         config->getTag().c_str(),
                         sampleCount);
                return std::nullopt;
            }
        } else {
            SkDebugf("No context was available matching config '%s'.\n", config->getTag().c_str());
            return std::nullopt;
        }

        return Config{gpuConfig->getTag(),
                      Benchmark::Backend::kGanesh,
                      colorType,
                      kPremul_SkAlphaType,
                      config->refColorSpace(),
                      sampleCount,
                      ctxType,
                      ctxOverrides,
                      gpuConfig->getSurfaceFlags()};
    }
#if defined(SK_GRAPHITE)
    if (const auto* gpuConfig = config->asConfigGraphite()) {
        if (!FLAGS_gpu) {
            SkDebugf("Skipping config '%s' as requested.\n", config->getTag().c_str());
            return std::nullopt;
        }

        const auto graphiteCtxType = gpuConfig->getContextType();
        const auto sampleCount = 1; // TODO: gpuConfig->getSamples();
        const auto colorType = gpuConfig->getColorType();

        using ContextFactory = skiatest::graphite::ContextFactory;

        ContextFactory factory(gpuConfig->asConfigGraphite()->getOptions());
        skiatest::graphite::ContextInfo ctxInfo = factory.getContextInfo(graphiteCtxType);
        skgpu::graphite::Context* ctx = ctxInfo.fContext;
        if (ctx) {
            // TODO: Add graphite ctx queries for supported sample count by color type.
#if 0
            GrBackendFormat format = ctx->defaultBackendFormat(colorType, GrRenderable::kYes);
            int supportedSampleCount =
                    ctx->priv().caps()->getRenderTargetSampleCount(sampleCount, format);
            if (sampleCount != supportedSampleCount) {
                SkDebugf("Configuration '%s' sample count %d is not a supported sample count.\n",
                         config->getTag().c_str(),
                         sampleCount);
                return std::nullopt;
            }
#else
            if (sampleCount > 1) {
                SkDebugf("Configuration '%s' sample count %d is not a supported sample count.\n",
                         config->getTag().c_str(),
                         sampleCount);
                return std::nullopt;
            }
#endif
        } else {
            SkDebugf("No context was available matching config '%s'.\n", config->getTag().c_str());
            return std::nullopt;
        }

        return Config{gpuConfig->getTag(),
                      Benchmark::Backend::kGraphite,
                      colorType,
                      kPremul_SkAlphaType,
                      config->refColorSpace(),
                      sampleCount,
                      graphiteCtxType,
                      kBogusContextOverrides,
                      0};
    }
#endif

#define CPU_CONFIG(name, backend, color, alpha)                                         \
    if (config->getBackend().equals(name)) {                                            \
        if (!FLAGS_cpu) {                                                               \
            SkDebugf("Skipping config '%s' as requested.\n", config->getTag().c_str()); \
            return std::nullopt;                                                      \
        }                                                                               \
        return Config{SkString(name),                                                   \
                      Benchmark::backend,                                               \
                      color,                                                            \
                      alpha,                                                            \
                      config->refColorSpace(),                                          \
                      0,                                                                \
                      kBogusContextType,                                                \
                      kBogusContextOverrides,                                           \
                      0};                                                               \
    }

    CPU_CONFIG("nonrendering", Backend::kNonRendering, kUnknown_SkColorType, kUnpremul_SkAlphaType)

    CPU_CONFIG("a8",    Backend::kRaster,    kAlpha_8_SkColorType, kPremul_SkAlphaType)
    CPU_CONFIG("565",   Backend::kRaster,    kRGB_565_SkColorType, kOpaque_SkAlphaType)
    CPU_CONFIG("8888",  Backend::kRaster,        kN32_SkColorType, kPremul_SkAlphaType)
    CPU_CONFIG("rgba",  Backend::kRaster,  kRGBA_8888_SkColorType, kPremul_SkAlphaType)
    CPU_CONFIG("bgra",  Backend::kRaster,  kBGRA_8888_SkColorType, kPremul_SkAlphaType)
    CPU_CONFIG("f16",   Backend::kRaster,   kRGBA_F16_SkColorType, kPremul_SkAlphaType)
    CPU_CONFIG("srgba", Backend::kRaster, kSRGBA_8888_SkColorType, kPremul_SkAlphaType)

#undef CPU_CONFIG

    SkDebugf("Unknown config '%s'.\n", config->getTag().c_str());
    return std::nullopt;
}

// Append all configs that are enabled and supported.
void create_configs(TArray<Config>* configs) {
    SkCommandLineConfigArray array;
    ParseConfigs(FLAGS_config, &array);
    for (int i = 0; i < array.size(); ++i) {
        if (std::optional<Config> config = create_config(array[i].get())) {
            configs->push_back(*config);
        }
    }

    // If no just default configs were requested, then we're okay.
    if (array.size() == 0 || FLAGS_config.size() == 0 ||
        // Otherwise, make sure that all specified configs have been created.
        array.size() == configs->size()) {
        return;
    }
    exit(1);
}

// disable warning : switch statement contains default but no 'case' labels
#if defined _WIN32
#pragma warning ( push )
#pragma warning ( disable : 4065 )
#endif

// If bench is enabled for config, returns a Target* for it, otherwise nullptr.
static Target* is_enabled(Benchmark* bench, const Config& config) {
    if (!bench->isSuitableFor(config.backend)) {
        return nullptr;
    }

    SkImageInfo info =
            SkImageInfo::Make(bench->getSize(), config.color, config.alpha, config.colorSpace);

    Target* target = nullptr;

    switch (config.backend) {
    case Benchmark::Backend::kGanesh:
        target = new GPUTarget(config);
        break;
#if defined(SK_GRAPHITE)
    case Benchmark::Backend::kGraphite:
        target = new GraphiteTarget(config);
        break;
#endif
    default:
        target = new Target(config);
        break;
    }

    if (!target->init(info, bench)) {
        delete target;
        return nullptr;
    }
    return target;
}

#if defined _WIN32
#pragma warning ( pop )
#endif

#ifdef SK_ENABLE_ANDROID_UTILS
static bool valid_brd_bench(sk_sp<SkData> encoded, SkColorType colorType, uint32_t sampleSize,
        uint32_t minOutputSize, int* width, int* height) {
    auto brd = android::skia::BitmapRegionDecoder::Make(encoded);
    if (nullptr == brd) {
        // This is indicates that subset decoding is not supported for a particular image format.
        return false;
    }

    if (sampleSize * minOutputSize > (uint32_t) brd->width() || sampleSize * minOutputSize >
            (uint32_t) brd->height()) {
        // This indicates that the image is not large enough to decode a
        // minOutputSize x minOutputSize subset at the given sampleSize.
        return false;
    }

    // Set the image width and height.  The calling code will use this to choose subsets to decode.
    *width = brd->width();
    *height = brd->height();
    return true;
}
#endif

static void cleanup_run(Target* target) {
    delete target;
}

static void collect_files(const CommandLineFlags::StringArray& paths,
                          const char*                          ext,
                          TArray<SkString>*                  list) {
    for (int i = 0; i < paths.size(); ++i) {
        if (SkStrEndsWith(paths[i], ext)) {
            list->push_back(SkString(paths[i]));
        } else {
            SkOSFile::Iter it(paths[i], ext);
            SkString path;
            while (it.next(&path)) {
                list->push_back(SkOSPath::Join(paths[i], path.c_str()));
            }
        }
    }
}

class BenchmarkStream {
public:
    BenchmarkStream() : fBenches(BenchRegistry::Head())
                      , fGMs(skiagm::GMRegistry::Head()) {
        collect_files(FLAGS_skps, ".skp", &fSKPs);
        collect_files(FLAGS_mskps, ".mskp", &fMSKPs);
        collect_files(FLAGS_svgs, ".svg", &fSVGs);
        collect_files(FLAGS_texttraces, ".trace", &fTextBlobTraces);

        if (4 != sscanf(FLAGS_clip[0], "%d,%d,%d,%d",
                        &fClip.fLeft, &fClip.fTop, &fClip.fRight, &fClip.fBottom)) {
            SkDebugf("Can't parse %s from --clip as an SkIRect.\n", FLAGS_clip[0]);
            exit(1);
        }

        for (int i = 0; i < FLAGS_scales.size(); i++) {
            if (1 != sscanf(FLAGS_scales[i], "%f", &fScales.push_back())) {
                SkDebugf("Can't parse %s from --scales as an SkScalar.\n", FLAGS_scales[i]);
                exit(1);
            }
        }

        if (2 != sscanf(FLAGS_zoom[0], "%f,%lf", &fZoomMax, &fZoomPeriodMs)) {
            SkDebugf("Can't parse %s from --zoom as a zoomMax,zoomPeriodMs.\n", FLAGS_zoom[0]);
            exit(1);
        }

        // Prepare the images for decoding
        if (!CommonFlags::CollectImages(FLAGS_images, &fImages)) {
            exit(1);
        }

        // Choose the candidate color types for image decoding
        fColorTypes.push_back(kN32_SkColorType);
        if (!FLAGS_simpleCodec) {
            fColorTypes.push_back(kRGB_565_SkColorType);
            fColorTypes.push_back(kAlpha_8_SkColorType);
            fColorTypes.push_back(kGray_8_SkColorType);
        }
    }

    static sk_sp<SkPicture> ReadPicture(const char* path) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (CommandLineFlags::ShouldSkip(FLAGS_match, SkOSPath::Basename(path).c_str())) {
            return nullptr;
        }

        std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path);
        if (!stream) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

        return SkPicture::MakeFromStream(stream.get());
    }

    static std::unique_ptr<MSKPPlayer> ReadMSKP(const char* path) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (CommandLineFlags::ShouldSkip(FLAGS_match, SkOSPath::Basename(path).c_str())) {
            return nullptr;
        }

        std::unique_ptr<SkStreamSeekable> stream = SkStream::MakeFromFile(path);
        if (!stream) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

        return MSKPPlayer::Make(stream.get());
    }

    static sk_sp<SkPicture> ReadSVGPicture(const char* path) {
        if (CommandLineFlags::ShouldSkip(FLAGS_match, SkOSPath::Basename(path).c_str())) {
            return nullptr;
        }
        sk_sp<SkData> data(SkData::MakeFromFileName(path));
        if (!data) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

#if defined(SK_ENABLE_SVG)
        SkMemoryStream stream(std::move(data));
        sk_sp<SkSVGDOM> svgDom =
                SkSVGDOM::Builder().setFontManager(ToolUtils::TestFontMgr()).make(stream);
        if (!svgDom) {
            SkDebugf("Could not parse %s.\n", path);
            return nullptr;
        }

        // Use the intrinsic SVG size if available, otherwise fall back to a default value.
        static const SkSize kDefaultContainerSize = SkSize::Make(128, 128);
        if (svgDom->containerSize().isEmpty()) {
            svgDom->setContainerSize(kDefaultContainerSize);
        }

        SkPictureRecorder recorder;
        svgDom->render(recorder.beginRecording(svgDom->containerSize().width(),
                                               svgDom->containerSize().height()));
        return recorder.finishRecordingAsPicture();
#else
        return nullptr;
#endif  // defined(SK_ENABLE_SVG)
    }

    Benchmark* next() {
        std::unique_ptr<Benchmark> bench;
        do {
            bench.reset(this->rawNext());
            if (!bench) {
                return nullptr;
            }
        } while (CommandLineFlags::ShouldSkip(FLAGS_sourceType, fSourceType) ||
                 CommandLineFlags::ShouldSkip(FLAGS_benchType, fBenchType));
        return bench.release();
    }

    Benchmark* rawNext() {
        if (fBenches) {
            Benchmark* bench = fBenches->get()(nullptr);
            fBenches = fBenches->next();
            fSourceType = "bench";
            fBenchType  = "micro";
            return bench;
        }

        while (fGMs) {
            std::unique_ptr<skiagm::GM> gm = fGMs->get()();
            if (gm->isBazelOnly()) {
                // We skip Bazel-only GMs because they might not be regular GMs. The Bazel build
                // reuses the notion of GMs to replace the notion of DM sources of various kinds,
                // such as codec sources and image generation sources. See comments in the
                // skiagm::GM::isBazelOnly function declaration for context.
                continue;
            }
            fGMs = fGMs->next();
            if (gm->runAsBench()) {
                fSourceType = "gm";
                fBenchType  = "micro";
                return new GMBench(std::move(gm));
            }
        }

        while (fCurrentTextBlobTrace < fTextBlobTraces.size()) {
            SkString path = fTextBlobTraces[fCurrentTextBlobTrace++];
            SkString basename = SkOSPath::Basename(path.c_str());
            static constexpr char kEnding[] = ".trace";
            if (basename.endsWith(kEnding)) {
                basename.remove(basename.size() - strlen(kEnding), strlen(kEnding));
            }
            fSourceType = "texttrace";
            fBenchType  = "micro";
            return CreateDiffCanvasBench(
                    SkStringPrintf("SkDiffBench-%s", basename.c_str()),
                    [path](){ return SkStream::MakeFromFile(path.c_str()); });
        }

        // First add all .skps as RecordingBenches.
        while (fCurrentRecording < fSKPs.size()) {
            const SkString& path = fSKPs[fCurrentRecording++];
            sk_sp<SkPicture> pic = ReadPicture(path.c_str());
            if (!pic) {
                continue;
            }
            SkString name = SkOSPath::Basename(path.c_str());
            fSourceType = "skp";
            fBenchType  = "recording";
            fSKPBytes = static_cast<double>(pic->approximateBytesUsed());
            fSKPOps   = pic->approximateOpCount();
            return new RecordingBench(name.c_str(), pic.get(), FLAGS_bbh);
        }

        // Add all .skps as DeserializePictureBenchs.
        while (fCurrentDeserialPicture < fSKPs.size()) {
            const SkString& path = fSKPs[fCurrentDeserialPicture++];
            sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
            if (!data) {
                continue;
            }
            SkString name = SkOSPath::Basename(path.c_str());
            fSourceType = "skp";
            fBenchType  = "deserial";
            fSKPBytes = static_cast<double>(data->size());
            fSKPOps   = 0;
            return new DeserializePictureBench(name.c_str(), std::move(data));
        }

        // Then once each for each scale as SKPBenches (playback).
        while (fCurrentScale < fScales.size()) {
            while (fCurrentSKP < fSKPs.size()) {
                const SkString& path = fSKPs[fCurrentSKP++];
                sk_sp<SkPicture> pic = ReadPicture(path.c_str());
                if (!pic) {
                    continue;
                }

                if (FLAGS_bbh) {
                    // The SKP we read off disk doesn't have a BBH.  Re-record so it grows one.
                    SkRTreeFactory factory;
                    SkPictureRecorder recorder;
                    pic->playback(recorder.beginRecording(pic->cullRect().width(),
                                                          pic->cullRect().height(),
                                                          &factory));
                    pic = recorder.finishRecordingAsPicture();
                }
                SkString name = SkOSPath::Basename(path.c_str());
                fSourceType = "skp";
                fBenchType = "playback";
                return new SKPBench(name.c_str(), pic.get(), fClip, fScales[fCurrentScale],
                                    FLAGS_loopSKP);
            }

            while (fCurrentSVG < fSVGs.size()) {
                const char* path = fSVGs[fCurrentSVG++].c_str();
                if (sk_sp<SkPicture> pic = ReadSVGPicture(path)) {
                    fSourceType = "svg";
                    fBenchType = "playback";
                    return new SKPBench(SkOSPath::Basename(path).c_str(), pic.get(), fClip,
                                        fScales[fCurrentScale], FLAGS_loopSKP);
                }
            }

            fCurrentSKP = 0;
            fCurrentSVG = 0;
            fCurrentScale++;
        }

        // Now loop over each skp again if we have an animation
        if (fZoomMax != 1.0f && fZoomPeriodMs > 0) {
            while (fCurrentAnimSKP < fSKPs.size()) {
                const SkString& path = fSKPs[fCurrentAnimSKP];
                sk_sp<SkPicture> pic = ReadPicture(path.c_str());
                if (!pic) {
                    fCurrentAnimSKP++;
                    continue;
                }

                fCurrentAnimSKP++;
                SkString name = SkOSPath::Basename(path.c_str());
                sk_sp<SKPAnimationBench::Animation> animation =
                    SKPAnimationBench::MakeZoomAnimation(fZoomMax, fZoomPeriodMs);
                return new SKPAnimationBench(name.c_str(), pic.get(), fClip, std::move(animation),
                                             FLAGS_loopSKP);
            }
        }

        // Read all MSKPs as benches
        while (fCurrentMSKP < fMSKPs.size()) {
            const SkString& path = fMSKPs[fCurrentMSKP++];
            std::unique_ptr<MSKPPlayer> player = ReadMSKP(path.c_str());
            if (!player) {
                continue;
            }
            SkString name = SkOSPath::Basename(path.c_str());
            fSourceType = "mskp";
            fBenchType = "mskp";
            return new MSKPBench(std::move(name), std::move(player));
        }

        for (; fCurrentCodec < fImages.size(); fCurrentCodec++) {
            fSourceType = "image";
            fBenchType = "skcodec";
            const SkString& path = fImages[fCurrentCodec];
            if (CommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }
            sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
            std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
            if (!codec) {
                // Nothing to time.
                SkDebugf("Cannot find codec for %s\n", path.c_str());
                continue;
            }

            while (fCurrentColorType < fColorTypes.size()) {
                const SkColorType colorType = fColorTypes[fCurrentColorType];

                SkAlphaType alphaType = codec->getInfo().alphaType();
                if (FLAGS_simpleCodec) {
                    if (kUnpremul_SkAlphaType == alphaType) {
                        alphaType = kPremul_SkAlphaType;
                    }

                    fCurrentColorType++;
                } else {
                    switch (alphaType) {
                        case kOpaque_SkAlphaType:
                            // We only need to test one alpha type (opaque).
                            fCurrentColorType++;
                            break;
                        case kUnpremul_SkAlphaType:
                        case kPremul_SkAlphaType:
                            if (0 == fCurrentAlphaType) {
                                // Test unpremul first.
                                alphaType = kUnpremul_SkAlphaType;
                                fCurrentAlphaType++;
                            } else {
                                // Test premul.
                                alphaType = kPremul_SkAlphaType;
                                fCurrentAlphaType = 0;
                                fCurrentColorType++;
                            }
                            break;
                        default:
                            SkASSERT(false);
                            fCurrentColorType++;
                            break;
                    }
                }

                // Make sure we can decode to this color type and alpha type.
                SkImageInfo info =
                        codec->getInfo().makeColorType(colorType).makeAlphaType(alphaType);
                const size_t rowBytes = info.minRowBytes();
                SkAutoMalloc storage(info.computeByteSize(rowBytes));

                const SkCodec::Result result = codec->getPixels(
                        info, storage.get(), rowBytes);
                switch (result) {
                    case SkCodec::kSuccess:
                    case SkCodec::kIncompleteInput:
                        return new CodecBench(SkOSPath::Basename(path.c_str()),
                                              encoded.get(), colorType, alphaType);
                    case SkCodec::kInvalidConversion:
                        // This is okay. Not all conversions are valid.
                        break;
                    default:
                        // This represents some sort of failure.
                        SkASSERT(false);
                        break;
                }
            }
            fCurrentColorType = 0;
        }

        // Run AndroidCodecBenches
        const int sampleSizes[] = { 2, 4, 8 };
        for (; fCurrentAndroidCodec < fImages.size(); fCurrentAndroidCodec++) {
            fSourceType = "image";
            fBenchType = "skandroidcodec";

            const SkString& path = fImages[fCurrentAndroidCodec];
            if (CommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }
            sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
            std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromData(encoded));
            if (!codec) {
                // Nothing to time.
                SkDebugf("Cannot find codec for %s\n", path.c_str());
                continue;
            }

            while (fCurrentSampleSize < (int) std::size(sampleSizes)) {
                int sampleSize = sampleSizes[fCurrentSampleSize];
                fCurrentSampleSize++;
                if (10 * sampleSize > std::min(codec->getInfo().width(), codec->getInfo().height())) {
                    // Avoid benchmarking scaled decodes of already small images.
                    break;
                }

                return new AndroidCodecBench(SkOSPath::Basename(path.c_str()),
                                             encoded.get(), sampleSize);
            }
            fCurrentSampleSize = 0;
        }

#ifdef SK_ENABLE_ANDROID_UTILS
        // Run the BRDBenches
        // We intend to create benchmarks that model the use cases in
        // android/libraries/social/tiledimage.  In this library, an image is decoded in 512x512
        // tiles.  The image can be translated freely, so the location of a tile may be anywhere in
        // the image.  For that reason, we will benchmark decodes in five representative locations
        // in the image.  Additionally, this use case utilizes power of two scaling, so we will
        // test on power of two sample sizes.  The output tile is always 512x512, so, when a
        // sampleSize is used, the size of the subset that is decoded is always
        // (sampleSize*512)x(sampleSize*512).
        // There are a few good reasons to only test on power of two sample sizes at this time:
        //     All use cases we are aware of only scale by powers of two.
        //     PNG decodes use the indicated sampling strategy regardless of the sample size, so
        //         these tests are sufficient to provide good coverage of our scaling options.
        const uint32_t brdSampleSizes[] = { 1, 2, 4, 8, 16 };
        const uint32_t minOutputSize = 512;
        for (; fCurrentBRDImage < fImages.size(); fCurrentBRDImage++) {
            fSourceType = "image";
            fBenchType = "BRD";

            const SkString& path = fImages[fCurrentBRDImage];
            if (CommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }

            while (fCurrentColorType < fColorTypes.size()) {
                while (fCurrentSampleSize < (int) std::size(brdSampleSizes)) {
                    while (fCurrentSubsetType <= kLastSingle_SubsetType) {

                        sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
                        const SkColorType colorType = fColorTypes[fCurrentColorType];
                        uint32_t sampleSize = brdSampleSizes[fCurrentSampleSize];
                        int currentSubsetType = fCurrentSubsetType++;

                        int width = 0;
                        int height = 0;
                        if (!valid_brd_bench(encoded, colorType, sampleSize, minOutputSize,
                                &width, &height)) {
                            break;
                        }

                        SkString basename = SkOSPath::Basename(path.c_str());
                        SkIRect subset;
                        const uint32_t subsetSize = sampleSize * minOutputSize;
                        switch (currentSubsetType) {
                            case kTopLeft_SubsetType:
                                basename.append("_TopLeft");
                                subset = SkIRect::MakeXYWH(0, 0, subsetSize, subsetSize);
                                break;
                            case kTopRight_SubsetType:
                                basename.append("_TopRight");
                                subset = SkIRect::MakeXYWH(width - subsetSize, 0, subsetSize,
                                        subsetSize);
                                break;
                            case kMiddle_SubsetType:
                                basename.append("_Middle");
                                subset = SkIRect::MakeXYWH((width - subsetSize) / 2,
                                        (height - subsetSize) / 2, subsetSize, subsetSize);
                                break;
                            case kBottomLeft_SubsetType:
                                basename.append("_BottomLeft");
                                subset = SkIRect::MakeXYWH(0, height - subsetSize, subsetSize,
                                        subsetSize);
                                break;
                            case kBottomRight_SubsetType:
                                basename.append("_BottomRight");
                                subset = SkIRect::MakeXYWH(width - subsetSize,
                                        height - subsetSize, subsetSize, subsetSize);
                                break;
                            default:
                                SkASSERT(false);
                        }

                        return new BitmapRegionDecoderBench(basename.c_str(), encoded.get(),
                                colorType, sampleSize, subset);
                    }
                    fCurrentSubsetType = 0;
                    fCurrentSampleSize++;
                }
                fCurrentSampleSize = 0;
                fCurrentColorType++;
            }
            fCurrentColorType = 0;
        }
#endif // SK_ENABLE_ANDROID_UTILS

        return nullptr;
    }

    void fillCurrentOptions(NanoJSONResultsWriter& log) const {
        log.appendCString("source_type", fSourceType);
        log.appendCString("bench_type",  fBenchType);
        if (0 == strcmp(fSourceType, "skp")) {
            log.appendString("clip",
                    SkStringPrintf("%d %d %d %d", fClip.fLeft, fClip.fTop,
                                                  fClip.fRight, fClip.fBottom));
            SkASSERT_RELEASE(fCurrentScale < fScales.size());  // debugging paranoia
            log.appendString("scale", SkStringPrintf("%.2g", fScales[fCurrentScale]));
        }
    }

    void fillCurrentMetrics(NanoJSONResultsWriter& log) const {
        if (0 == strcmp(fBenchType, "recording")) {
            log.appendMetric("bytes", fSKPBytes);
            log.appendMetric("ops", fSKPOps);
        }
    }

private:
#ifdef SK_ENABLE_ANDROID_UTILS
    enum SubsetType {
        kTopLeft_SubsetType     = 0,
        kTopRight_SubsetType    = 1,
        kMiddle_SubsetType      = 2,
        kBottomLeft_SubsetType  = 3,
        kBottomRight_SubsetType = 4,
        kTranslate_SubsetType   = 5,
        kZoom_SubsetType        = 6,
        kLast_SubsetType        = kZoom_SubsetType,
        kLastSingle_SubsetType  = kBottomRight_SubsetType,
    };
#endif

    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkIRect            fClip;
    TArray<SkScalar> fScales;
    TArray<SkString> fSKPs;
    TArray<SkString> fMSKPs;
    TArray<SkString> fSVGs;
    TArray<SkString> fTextBlobTraces;
    TArray<SkString> fImages;
    TArray<SkColorType, true> fColorTypes;
    SkScalar           fZoomMax;
    double             fZoomPeriodMs;

    double fSKPBytes, fSKPOps;

    const char* fSourceType;  // What we're benching: bench, GM, SKP, ...
    const char* fBenchType;   // How we bench it: micro, recording, playback, ...
    int fCurrentRecording = 0;
    int fCurrentDeserialPicture = 0;
    int fCurrentMSKP = 0;
    int fCurrentScale = 0;
    int fCurrentSKP = 0;
    int fCurrentSVG = 0;
    int fCurrentTextBlobTrace = 0;
    int fCurrentCodec = 0;
    int fCurrentAndroidCodec = 0;
#ifdef SK_ENABLE_ANDROID_UTILS
    int fCurrentBRDImage = 0;
    int fCurrentSubsetType = 0;
#endif
    int fCurrentColorType = 0;
    int fCurrentAlphaType = 0;
    int fCurrentSampleSize = 0;
    int fCurrentAnimSKP = 0;
};

// Some runs (mostly, Valgrind) are so slow that the bot framework thinks we've hung.
// This prints something every once in a while so that it knows we're still working.
static void start_keepalive() {
    static std::thread* intentionallyLeaked = new std::thread([]{
        for (;;) {
            static const int kSec = 1200;
        #if defined(SK_BUILD_FOR_WIN)
            Sleep(kSec * 1000);
        #else
            sleep(kSec);
        #endif
            SkDebugf("\nBenchmarks still running...\n");
        }
    });
    (void)intentionallyLeaked;
    SK_INTENTIONALLY_LEAKED(intentionallyLeaked);
}

class NanobenchShaderErrorHandler : public GrContextOptions::ShaderErrorHandler {
    void compileError(const char* shader, const char* errors) override {
        // Nanobench should abort if any shader can't compile. Failure is much better than
        // reporting meaningless performance metrics.
        std::string message = SkShaderUtils::BuildShaderErrorMessage(shader, errors);
        SK_ABORT("\n%s", message.c_str());
    }
};

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    initializeEventTracingForTools();

#if defined(SK_BUILD_FOR_IOS)
    cd_Documents();
#endif
    SetupCrashHandler();
    if (FLAGS_runtimeCPUDetection) {
        SkGraphics::Init();
    }

    // Our benchmarks only currently decode .png or .jpg files
    SkCodecs::Register(SkPngDecoder::Decoder());
    SkCodecs::Register(SkJpegDecoder::Decoder());

    SkTaskGroup::Enabler enabled(FLAGS_threads);

    CommonFlags::SetCtxOptions(&grContextOpts);

    NanobenchShaderErrorHandler errorHandler;
    grContextOpts.fShaderErrorHandler = &errorHandler;

    if (kAutoTuneLoops != FLAGS_loops) {
        FLAGS_samples     = 1;
        FLAGS_gpuFrameLag = 0;
    }

    if (!FLAGS_writePath.isEmpty()) {
        SkDebugf("Writing files to %s.\n", FLAGS_writePath[0]);
        if (!sk_mkdir(FLAGS_writePath[0])) {
            SkDebugf("Could not create %s. Files won't be written.\n", FLAGS_writePath[0]);
            FLAGS_writePath.set(0, nullptr);
        }
    }

    std::unique_ptr<SkWStream> logStream(new SkNullWStream);
    if (!FLAGS_outResultsFile.isEmpty()) {
#if defined(SK_RELEASE)
        logStream.reset(new SkFILEWStream(FLAGS_outResultsFile[0]));
#else
        SkDebugf("I'm ignoring --outResultsFile because this is a Debug build.");
        return 1;
#endif
    }
    NanoJSONResultsWriter log(logStream.get(), SkJSONWriter::Mode::kPretty);
    log.beginObject(); // root

    if (1 == FLAGS_properties.size() % 2) {
        SkDebugf("ERROR: --properties must be passed with an even number of arguments.\n");
        return 1;
    }
    for (int i = 1; i < FLAGS_properties.size(); i += 2) {
        log.appendCString(FLAGS_properties[i-1], FLAGS_properties[i]);
    }

    if (1 == FLAGS_key.size() % 2) {
        SkDebugf("ERROR: --key must be passed with an even number of arguments.\n");
        return 1;
    }
    if (FLAGS_key.size()) {
        log.beginObject("key");
        for (int i = 1; i < FLAGS_key.size(); i += 2) {
            log.appendCString(FLAGS_key[i - 1], FLAGS_key[i]);
        }
        log.endObject(); // key
    }

    const double overhead = estimate_timer_overhead();
    if (!FLAGS_quiet && !FLAGS_csv) {
        SkDebugf("Timer overhead: %s\n", HUMANIZE(overhead));
    }

    TArray<double> samples;

    if (kAutoTuneLoops != FLAGS_loops) {
        SkDebugf("Fixed number of loops; times would only be misleading so we won't print them.\n");
    } else if (FLAGS_quiet) {
        SkDebugf("! -> high variance, ? -> moderate variance\n");
        SkDebugf("    micros   \tbench\n");
    } else if (FLAGS_csv) {
        SkDebugf("min,median,mean,max,stddev,config,bench\n");
    } else if (FLAGS_ms) {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\tsamples\tconfig\tbench\n");
    } else {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tconfig\tbench\n",
                 FLAGS_samples, "samples");
    }

    GrRecordingContextPriv::DMSAAStats combinedDMSAAStats;

    TArray<Config> configs;
    create_configs(&configs);

    if (FLAGS_keepAlive) {
        start_keepalive();
    }

    gSkForceRasterPipelineBlitter     = FLAGS_forceRasterPipelineHP || FLAGS_forceRasterPipeline;
    gForceHighPrecisionRasterPipeline = FLAGS_forceRasterPipelineHP;

    // The SkSL memory benchmark must run before any GPU painting occurs. SkSL allocates memory for
    // its modules the first time they are accessed, and this test is trying to measure the size of
    // those allocations. If a paint has already occurred, some modules will have already been
    // loaded, so we won't be able to capture a delta for them.
    log.beginObject("results");
    RunSkSLModuleBenchmarks(&log);

    int runs = 0;
    BenchmarkStream benchStream;
    AutoreleasePool pool;
    while (Benchmark* b = benchStream.next()) {
        std::unique_ptr<Benchmark> bench(b);
        if (CommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName())) {
            continue;
        }

        if (!configs.empty()) {
            log.beginBench(
                    bench->getUniqueName(), bench->getSize().width(), bench->getSize().height());
            bench->delayedSetup();
        }
        for (int i = 0; i < configs.size(); ++i) {
            Target* target = is_enabled(b, configs[i]);
            if (!target) {
                continue;
            }

            // During HWUI output this canvas may be nullptr.
            SkCanvas* canvas = target->getCanvas();
            const char* config = target->config.name.c_str();

            if (FLAGS_pre_log || FLAGS_dryRun) {
                SkDebugf("Running %s\t%s\n"
                         , bench->getUniqueName()
                         , config);
                if (FLAGS_dryRun) {
                    continue;
                }
            }

            if (FLAGS_purgeBetweenBenches) {
                SkGraphics::PurgeAllCaches();
            }

            if (FLAGS_splitPerfettoTracesByBenchmark) {
                TRACE_EVENT_API_NEW_TRACE_SECTION(TRACE_STR_COPY(bench->getUniqueName()));
            }
            TRACE_EVENT2("skia", "Benchmark", "name", TRACE_STR_COPY(bench->getUniqueName()),
                                              "config", TRACE_STR_COPY(config));

            target->setup();
            bench->perCanvasPreDraw(canvas);

            int maxFrameLag;
            int loops = target->needsFrameTiming(&maxFrameLag)
                ? setup_gpu_bench(target, bench.get(), maxFrameLag)
                : setup_cpu_bench(overhead, target, bench.get());

            if (kFailedLoops == loops) {
                // Can't be timed.  A warning note has already been printed.
                cleanup_run(target);
                continue;
            }

            if (runs == 0 && FLAGS_ms < 1000) {
                // Run the first bench for 1000ms to warm up the nanobench if FLAGS_ms < 1000.
                // Otherwise, the first few benches' measurements will be inaccurate.
                auto stop = now_ms() + 1000;
                do {
                    time(loops, bench.get(), target);
                    pool.drain();
                } while (now_ms() < stop);
            }

            if (FLAGS_ms) {
                samples.clear();
                auto stop = now_ms() + FLAGS_ms;
                do {
                    samples.push_back(time(loops, bench.get(), target) / loops);
                    pool.drain();
                } while (now_ms() < stop);
            } else {
                samples.reset(FLAGS_samples);
                for (int s = 0; s < FLAGS_samples; s++) {
                    samples[s] = time(loops, bench.get(), target) / loops;
                    pool.drain();
                }
            }

            // Scale each result to the benchmark's own units, time/unit.
            for (double& sample : samples) {
                sample *= (1.0 / bench->getUnits());
            }

            TArray<SkString> keys;
            TArray<double> values;
            if (configs[i].backend == Benchmark::Backend::kGanesh) {
                if (FLAGS_gpuStatsDump) {
                    // TODO cache stats
                    bench->getGpuStats(canvas, &keys, &values);
                }
                if (FLAGS_dmsaaStatsDump && bench->getDMSAAStats(canvas->recordingContext())) {
                    const auto& dmsaaStats = canvas->recordingContext()->priv().dmsaaStats();
                    dmsaaStats.dumpKeyValuePairs(&keys, &values);
                    dmsaaStats.dump();
                    combinedDMSAAStats.merge(dmsaaStats);
                }
            }

            bench->perCanvasPostDraw(canvas);

            if (Benchmark::Backend::kNonRendering != target->config.backend &&
                !FLAGS_writePath.isEmpty() && FLAGS_writePath[0]) {
                SkString pngFilename = SkOSPath::Join(FLAGS_writePath[0], config);
                pngFilename = SkOSPath::Join(pngFilename.c_str(), bench->getUniqueName());
                pngFilename.append(".png");
                write_canvas_png(target, pngFilename);
            }

            // Building stats.plot often shows up in profiles,
            // so skip building it when we're not going to print it anyway.
            const bool want_plot = !FLAGS_quiet && !FLAGS_ms;

            Stats stats(samples, want_plot);
            log.beginObject(config);

            log.beginObject("options");
            log.appendCString("name", bench->getName());
            benchStream.fillCurrentOptions(log);
            log.endObject(); // options

            // Metrics
            log.appendMetric("min_ms", stats.min);
            log.appendMetric("min_ratio", sk_ieee_double_divide(stats.median, stats.min));
            log.beginArray("samples");
            for (double sample : samples) {
                log.appendDoubleDigits(sample, 16);
            }
            log.endArray(); // samples
            benchStream.fillCurrentMetrics(log);
            if (!keys.empty()) {
                // dump to json, only SKPBench currently returns valid keys / values
                SkASSERT(keys.size() == values.size());
                for (int j = 0; j < keys.size(); j++) {
                    log.appendMetric(keys[j].c_str(), values[j]);
                }
            }

            log.endObject(); // config

            if (runs++ % FLAGS_flushEvery == 0) {
                log.flush();
            }

            if (kAutoTuneLoops != FLAGS_loops) {
                if (configs.size() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%4d/%-4dMB\t%s\t%s "
                         , sk_tools::getCurrResidentSetSizeMB()
                         , sk_tools::getMaxResidentSetSizeMB()
                         , bench->getUniqueName()
                         , config);
                SkDebugf("\n");
            } else if (FLAGS_quiet) {
                const char* mark = " ";
                const double stddev_percent =
                    sk_ieee_double_divide(100 * sqrt(stats.var), stats.mean);
                if (stddev_percent >  5) mark = "?";
                if (stddev_percent > 10) mark = "!";

                SkDebugf("%10.2f %s\t%s\t%s\n",
                         stats.median*1e3, mark, bench->getUniqueName(), config);
            } else if (FLAGS_csv) {
                const double stddev_percent =
                    sk_ieee_double_divide(100 * sqrt(stats.var), stats.mean);
                SkDebugf("%g,%g,%g,%g,%g,%s,%s\n"
                         , stats.min
                         , stats.median
                         , stats.mean
                         , stats.max
                         , stddev_percent
                         , config
                         , bench->getUniqueName()
                         );
            } else {
                const double stddev_percent =
                    sk_ieee_double_divide(100 * sqrt(stats.var), stats.mean);
                SkDebugf("%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                        , sk_tools::getCurrResidentSetSizeMB()
                        , sk_tools::getMaxResidentSetSizeMB()
                        , loops
                        , HUMANIZE(stats.min)
                        , HUMANIZE(stats.median)
                        , HUMANIZE(stats.mean)
                        , HUMANIZE(stats.max)
                        , stddev_percent
                        , FLAGS_ms ? to_string(samples.size()).c_str() : stats.plot.c_str()
                        , config
                        , bench->getUniqueName()
                        );
            }

            if (FLAGS_gpuStats && Benchmark::Backend::kGanesh == configs[i].backend) {
                target->dumpStats();
            }

            if (FLAGS_verbose) {
                SkDebugf("Samples:  ");
                for (int j = 0; j < samples.size(); j++) {
                    SkDebugf("%s  ", HUMANIZE(samples[j]));
                }
                SkDebugf("%s\n", bench->getUniqueName());
            }
            cleanup_run(target);
            pool.drain();
        }
        if (!configs.empty()) {
            log.endBench();
        }
    }

    if (FLAGS_dmsaaStatsDump) {
        SkDebugf("<<Total Combined DMSAA Stats>>\n");
        combinedDMSAAStats.dump();
    }

    SkGraphics::PurgeAllCaches();

    log.beginBench("memory_usage", 0, 0);
    log.beginObject("meta"); // config
    log.appendS32("max_rss_mb", sk_tools::getMaxResidentSetSizeMB());
    log.endObject(); // config
    log.endBench();

    log.endObject(); // results
    log.endObject(); // root
    log.flush();

    return 0;
}
