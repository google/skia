/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "nanobench.h"

#include "AndroidCodecBench.h"
#include "Benchmark.h"
#include "BitmapRegionDecoderBench.h"
#include "CodecBench.h"
#include "CodecBenchPriv.h"
#include "ColorCodecBench.h"
#include "CrashHandler.h"
#include "GMBench.h"
#include "ProcStats.h"
#include "RecordingBench.h"
#include "ResultsWriter.h"
#include "SKPAnimationBench.h"
#include "SKPBench.h"
#include "SkAndroidCodec.h"
#include "SkAutoMalloc.h"
#include "SkBBoxHierarchy.h"
#include "SkBitmapRegionDecoder.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCommonFlags.h"
#include "SkCommonFlagsConfig.h"
#include "SkCommonFlagsPathRenderer.h"
#include "SkData.h"
#include "SkDebugfTracer.h"
#include "SkGraphics.h"
#include "SkLeanWindows.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPictureRecorder.h"
#include "SkSVGDOM.h"
#include "SkScan.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include "SkThreadUtils.h"
#include "Stats.h"
#include "ThermalManager.h"
#include "ios_utils.h"

#include <stdlib.h>

extern bool gSkForceRasterPipelineBlitter;

#ifndef SK_BUILD_FOR_WIN32
    #include <unistd.h>
#endif

#if SK_SUPPORT_GPU
    #include "gl/GrGLDefines.h"
    #include "GrCaps.h"
    #include "GrContextFactory.h"
    #include "gl/GrGLUtil.h"
    #include "SkGr.h"
    using sk_gpu_test::GrContextFactory;
    using sk_gpu_test::TestContext;
    std::unique_ptr<GrContextFactory> gGrFactory;
#endif

    struct GrContextOptions;

static const int kAutoTuneLoops = 0;

#if !defined(__has_feature)
    #define  __has_feature(x) 0
#endif

static const int kDefaultLoops =
#if defined(SK_DEBUG) || __has_feature(address_sanitizer)
    1;
#else
    kAutoTuneLoops;
#endif

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

DEFINE_int32(loops, kDefaultLoops, loops_help_txt().c_str());

DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(ms, 0, "If >0, run each bench for this many ms instead of obeying --samples.");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_int32(gpuFrameLag, 5, "If unknown, estimated maximum number of frames GPU allows to lag.");

DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_int32(maxCalibrationAttempts, 3,
             "Try up to this many times to guess loops for a bench, or skip the bench.");
DEFINE_int32(maxLoops, 1000000, "Never run a bench more times than this.");
DEFINE_string(clip, "0,0,1000,1000", "Clip for SKPs.");
DEFINE_string(scales, "1.0", "Space-separated scales for SKPs.");
DEFINE_string(zoom, "1.0,0", "Comma-separated zoomMax,zoomPeriodMs factors for a periodic SKP zoom "
                             "function that ping-pongs between 1.0 and zoomMax.");
DEFINE_bool(bbh, true, "Build a BBH for SKPs?");
DEFINE_bool(lite, false, "Use SkLiteRecorder in recording benchmarks?");
DEFINE_bool(mpd, true, "Use MultiPictureDraw for the SKPs?");
DEFINE_bool(loopSKP, true, "Loop SKPs like we do for micro benches?");
DEFINE_int32(flushEvery, 10, "Flush --outResultsFile every Nth run.");
DEFINE_bool(resetGpuContext, true, "Reset the GrContext before running each test.");
DEFINE_bool(gpuStats, false, "Print GPU stats after each gpu benchmark?");
DEFINE_bool(gpuStatsDump, false, "Dump GPU states after each benchmark to json");
DEFINE_bool(keepAlive, false, "Print a message every so often so that we don't time out");
DEFINE_string(useThermalManager, "0,1,10,1000", "enabled,threshold,sleepTimeMs,TimeoutMs for "
                                                "thermalManager\n");
DEFINE_bool(csv, false, "Print status in CSV format");
DEFINE_string(sourceType, "",
        "Apply usual --match rules to source type: bench, gm, skp, image, etc.");
DEFINE_string(benchType,  "",
        "Apply usual --match rules to bench type: micro, recording, piping, playback, skcodec, etc.");

DEFINE_bool(forceRasterPipeline, false, "sets gSkForceRasterPipelineBlitter");

#if SK_SUPPORT_GPU
DEFINE_pathrenderer_flag;
#endif

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }

static SkString humanize(double ms) {
    if (FLAGS_verbose) return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    return HumanizeMs(ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

bool Target::init(SkImageInfo info, Benchmark* bench) {
    if (Benchmark::kRaster_Backend == config.backend) {
        this->surface = SkSurface::MakeRaster(info);
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

#if SK_SUPPORT_GPU
struct GPUTarget : public Target {
    explicit GPUTarget(const Config& c) : Target(c), context(nullptr) { }
    TestContext* context;

    void setup() override {
        this->context->makeCurrent();
        // Make sure we're done with whatever came before.
        this->context->finish();
    }
    void endTiming() override {
        if (this->context) {
            this->context->waitOnSyncOrSwap();
        }
    }
    void fence() override {
        this->context->finish();
    }

    bool needsFrameTiming(int* maxFrameLag) const override {
        if (!this->context->getMaxGpuFrameLag(maxFrameLag)) {
            // Frame lag is unknown.
            *maxFrameLag = FLAGS_gpuFrameLag;
        }
        return true;
    }
    bool init(SkImageInfo info, Benchmark* bench) override {
        uint32_t flags = this->config.useDFText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag :
                                                  0;
        SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
        this->surface = SkSurface::MakeRenderTarget(gGrFactory->get(this->config.ctxType,
                                                                    this->config.ctxOverrides),
                                                         SkBudgeted::kNo, info,
                                                         this->config.samples, &props);
        this->context = gGrFactory->getContextInfo(this->config.ctxType,
                                                   this->config.ctxOverrides).testContext();
        if (!this->surface.get()) {
            return false;
        }
        if (!this->context->fenceSyncSupport()) {
            SkDebugf("WARNING: GL context for config \"%s\" does not support fence sync. "
                     "Timings might not be accurate.\n", this->config.name.c_str());
        }
        return true;
    }
    void fillOptions(ResultsWriter* log) override {
        const GrGLubyte* version;
        if (this->context->backend() == kOpenGL_GrBackend) {
            const GrGLInterface* gl =
                    reinterpret_cast<const GrGLInterface*>(this->context->backendContext());
            GR_GL_CALL_RET(gl, version, GetString(GR_GL_VERSION));
            log->configOption("GL_VERSION", (const char*)(version));

            GR_GL_CALL_RET(gl, version, GetString(GR_GL_RENDERER));
            log->configOption("GL_RENDERER", (const char*) version);

            GR_GL_CALL_RET(gl, version, GetString(GR_GL_VENDOR));
            log->configOption("GL_VENDOR", (const char*) version);

            GR_GL_CALL_RET(gl, version, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
            log->configOption("GL_SHADING_LANGUAGE_VERSION", (const char*) version);
        }
    }
};

#endif

static double time(int loops, Benchmark* bench, Target* target) {
    SkCanvas* canvas = target->getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    bench->preDraw(canvas);
    double start = now_ms();
    canvas = target->beginTiming(canvas);
    bench->draw(loops, canvas);
    if (canvas) {
        canvas->flush();
    }
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
    if (!SkEncodeImage(&stream, bmp, SkEncodedImageFormat::kPNG, 100)) {
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
    int loops = bench->calculateLoops(FLAGS_loops);
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
    int loops = bench->calculateLoops(FLAGS_loops);
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
        target->fence();
    } else {
        loops = detect_forever_loops(loops);
    }

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < maxGpuFrameLag - 1; i++) {
        time(loops, bench, target);
    }

    return loops;
}

#if SK_SUPPORT_GPU
#define kBogusContextType GrContextFactory::kGL_ContextType
#define kBogusContextOverrides GrContextFactory::ContextOverrides::kNone
#else
#define kBogusContextType 0
#define kBogusContextOverrides 0
#endif

static void create_config(const SkCommandLineConfig* config, SkTArray<Config>* configs) {

#if SK_SUPPORT_GPU
    if (const auto* gpuConfig = config->asConfigGpu()) {
        if (!FLAGS_gpu)
            return;

        const auto ctxType = gpuConfig->getContextType();
        const auto ctxOverrides = gpuConfig->getContextOverrides();
        const auto sampleCount = gpuConfig->getSamples();
        const auto colorType = gpuConfig->getColorType();
        auto colorSpace = gpuConfig->getColorSpace();

        if (const GrContext* ctx = gGrFactory->get(ctxType, ctxOverrides)) {
            GrPixelConfig grPixConfig = SkImageInfo2GrPixelConfig(colorType, colorSpace,
                                                                  *ctx->caps());
            int supportedSampleCount = ctx->caps()->getSampleCount(sampleCount, grPixConfig);
            if (sampleCount != supportedSampleCount) {
                SkDebugf("Configuration sample count %d is not a supported sample count.\n",
                    sampleCount);
                return;
            }
        } else {
            SkDebugf("No context was available matching config type and options.\n");
            return;
        }

        Config target = {
            gpuConfig->getTag(),
            Benchmark::kGPU_Backend,
            colorType,
            kPremul_SkAlphaType,
            sk_ref_sp(colorSpace),
            sampleCount,
            ctxType,
            ctxOverrides,
            gpuConfig->getUseDIText()
        };

        configs->push_back(target);
        return;
    }
#endif

    #define CPU_CONFIG(name, backend, color, alpha, colorSpace)                \
        if (config->getTag().equals(#name)) {                                  \
            Config config = {                                                  \
                SkString(#name), Benchmark::backend, color, alpha, colorSpace, \
                0, kBogusContextType, kBogusContextOverrides, false            \
            };                                                                 \
            configs->push_back(config);                                        \
            return;                                                            \
        }

    if (FLAGS_cpu) {
        CPU_CONFIG(nonrendering, kNonRendering_Backend,
                   kUnknown_SkColorType, kUnpremul_SkAlphaType, nullptr)

        CPU_CONFIG(8888, kRaster_Backend,
                   kN32_SkColorType, kPremul_SkAlphaType, nullptr)
        CPU_CONFIG(565,  kRaster_Backend,
                   kRGB_565_SkColorType, kOpaque_SkAlphaType, nullptr)
        auto srgbColorSpace = SkColorSpace::MakeSRGB();
        CPU_CONFIG(srgb, kRaster_Backend,
                   kN32_SkColorType,  kPremul_SkAlphaType, srgbColorSpace)
        auto srgbLinearColorSpace = SkColorSpace::MakeSRGBLinear();
        CPU_CONFIG(f16,  kRaster_Backend,
                   kRGBA_F16_SkColorType, kPremul_SkAlphaType, srgbLinearColorSpace)
    }

    #undef CPU_CONFIG
}

// Append all configs that are enabled and supported.
void create_configs(SkTArray<Config>* configs) {
    SkCommandLineConfigArray array;
    ParseConfigs(FLAGS_config, &array);
    for (int i = 0; i < array.count(); ++i) {
        create_config(array[i].get(), configs);
    }
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

    SkImageInfo info = SkImageInfo::Make(bench->getSize().fX, bench->getSize().fY,
                                         config.color, config.alpha, config.colorSpace);

    Target* target = nullptr;

    switch (config.backend) {
#if SK_SUPPORT_GPU
    case Benchmark::kGPU_Backend:
        target = new GPUTarget(config);
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

static bool valid_brd_bench(sk_sp<SkData> encoded, SkColorType colorType, uint32_t sampleSize,
        uint32_t minOutputSize, int* width, int* height) {
    std::unique_ptr<SkBitmapRegionDecoder> brd(
            SkBitmapRegionDecoder::Create(encoded, SkBitmapRegionDecoder::kAndroidCodec_Strategy));
    if (nullptr == brd.get()) {
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

static void cleanup_run(Target* target) {
    delete target;
#if SK_SUPPORT_GPU
    if (FLAGS_abandonGpuContext) {
        gGrFactory->abandonContexts();
    }
    if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
        gGrFactory->destroyContexts();
    }
#endif
}

static void collect_files(const SkCommandLineFlags::StringArray& paths, const char* ext,
                          SkTArray<SkString>* list) {
    for (int i = 0; i < paths.count(); ++i) {
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
                      , fGMs(skiagm::GMRegistry::Head())
                      , fCurrentRecording(0)
                      , fCurrentPiping(0)
                      , fCurrentScale(0)
                      , fCurrentSKP(0)
                      , fCurrentSVG(0)
                      , fCurrentUseMPD(0)
                      , fCurrentCodec(0)
                      , fCurrentAndroidCodec(0)
                      , fCurrentBRDImage(0)
                      , fCurrentColorImage(0)
                      , fCurrentColorType(0)
                      , fCurrentAlphaType(0)
                      , fCurrentSubsetType(0)
                      , fCurrentSampleSize(0)
                      , fCurrentAnimSKP(0) {
        collect_files(FLAGS_skps, ".skp", &fSKPs);
        collect_files(FLAGS_svgs, ".svg", &fSVGs);

        if (4 != sscanf(FLAGS_clip[0], "%d,%d,%d,%d",
                        &fClip.fLeft, &fClip.fTop, &fClip.fRight, &fClip.fBottom)) {
            SkDebugf("Can't parse %s from --clip as an SkIRect.\n", FLAGS_clip[0]);
            exit(1);
        }

        for (int i = 0; i < FLAGS_scales.count(); i++) {
            if (1 != sscanf(FLAGS_scales[i], "%f", &fScales.push_back())) {
                SkDebugf("Can't parse %s from --scales as an SkScalar.\n", FLAGS_scales[i]);
                exit(1);
            }
        }

        if (2 != sscanf(FLAGS_zoom[0], "%f,%lf", &fZoomMax, &fZoomPeriodMs)) {
            SkDebugf("Can't parse %s from --zoom as a zoomMax,zoomPeriodMs.\n", FLAGS_zoom[0]);
            exit(1);
        }

        if (FLAGS_mpd) {
            fUseMPDs.push_back() = true;
        }
        fUseMPDs.push_back() = false;

        // Prepare the images for decoding
        if (!CollectImages(FLAGS_images, &fImages)) {
            exit(1);
        }
        if (!CollectImages(FLAGS_colorImages, &fColorImages)) {
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
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, SkOSPath::Basename(path).c_str())) {
            return nullptr;
        }

        std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path);
        if (!stream) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

        return SkPicture::MakeFromStream(stream.get());
    }

    static sk_sp<SkPicture> ReadSVGPicture(const char* path) {
        SkFILEStream stream(path);
        if (!stream.isValid()) {
            SkDebugf("Could not read %s.\n", path);
            return nullptr;
        }

        sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(stream);
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
    }

    Benchmark* next() {
        std::unique_ptr<Benchmark> bench;
        do {
            bench.reset(this->rawNext());
            if (!bench) {
                return nullptr;
            }
        } while(SkCommandLineFlags::ShouldSkip(FLAGS_sourceType, fSourceType) ||
                SkCommandLineFlags::ShouldSkip(FLAGS_benchType,  fBenchType));
        return bench.release();
    }

    Benchmark* rawNext() {
        if (fBenches) {
            Benchmark* bench = fBenches->factory()(nullptr);
            fBenches = fBenches->next();
            fSourceType = "bench";
            fBenchType  = "micro";
            return bench;
        }

        while (fGMs) {
            std::unique_ptr<skiagm::GM> gm(fGMs->factory()(nullptr));
            fGMs = fGMs->next();
            if (gm->runAsBench()) {
                fSourceType = "gm";
                fBenchType  = "micro";
                return new GMBench(gm.release());
            }
        }

        // First add all .skps as RecordingBenches.
        while (fCurrentRecording < fSKPs.count()) {
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
            return new RecordingBench(name.c_str(), pic.get(), FLAGS_bbh, FLAGS_lite);
        }

        // Add all .skps as PipeBenches.
        while (fCurrentPiping < fSKPs.count()) {
            const SkString& path = fSKPs[fCurrentPiping++];
            sk_sp<SkPicture> pic = ReadPicture(path.c_str());
            if (!pic) {
                continue;
            }
            SkString name = SkOSPath::Basename(path.c_str());
            fSourceType = "skp";
            fBenchType  = "piping";
            fSKPBytes = static_cast<double>(pic->approximateBytesUsed());
            fSKPOps   = pic->approximateOpCount();
            return new PipingBench(name.c_str(), pic.get());
        }

        // Then once each for each scale as SKPBenches (playback).
        while (fCurrentScale < fScales.count()) {
            while (fCurrentSKP < fSKPs.count()) {
                const SkString& path = fSKPs[fCurrentSKP];
                sk_sp<SkPicture> pic = ReadPicture(path.c_str());
                if (!pic) {
                    fCurrentSKP++;
                    continue;
                }

                while (fCurrentUseMPD < fUseMPDs.count()) {
                    if (FLAGS_bbh) {
                        // The SKP we read off disk doesn't have a BBH.  Re-record so it grows one.
                        SkRTreeFactory factory;
                        SkPictureRecorder recorder;
                        pic->playback(recorder.beginRecording(pic->cullRect().width(),
                                                              pic->cullRect().height(),
                                                              &factory,
                                                              0));
                        pic = recorder.finishRecordingAsPicture();
                    }
                    SkString name = SkOSPath::Basename(path.c_str());
                    fSourceType = "skp";
                    fBenchType = "playback";
                    return new SKPBench(name.c_str(), pic.get(), fClip, fScales[fCurrentScale],
                                        fUseMPDs[fCurrentUseMPD++], FLAGS_loopSKP);
                }
                fCurrentUseMPD = 0;
                fCurrentSKP++;
            }

            while (fCurrentSVG++ < fSVGs.count()) {
                const char* path = fSVGs[fCurrentSVG - 1].c_str();
                if (sk_sp<SkPicture> pic = ReadSVGPicture(path)) {
                    fSourceType = "svg";
                    fBenchType = "playback";
                    return new SKPBench(SkOSPath::Basename(path).c_str(), pic.get(), fClip,
                                        fScales[fCurrentScale], false, FLAGS_loopSKP);
                }
            }

            fCurrentSKP = 0;
            fCurrentSVG = 0;
            fCurrentScale++;
        }

        // Now loop over each skp again if we have an animation
        if (fZoomMax != 1.0f && fZoomPeriodMs > 0) {
            while (fCurrentAnimSKP < fSKPs.count()) {
                const SkString& path = fSKPs[fCurrentAnimSKP];
                sk_sp<SkPicture> pic = ReadPicture(path.c_str());
                if (!pic) {
                    fCurrentAnimSKP++;
                    continue;
                }

                fCurrentAnimSKP++;
                SkString name = SkOSPath::Basename(path.c_str());
                sk_sp<SKPAnimationBench::Animation> animation(
                    SKPAnimationBench::CreateZoomAnimation(fZoomMax, fZoomPeriodMs));
                return new SKPAnimationBench(name.c_str(), pic.get(), fClip, animation.get(),
                                             FLAGS_loopSKP);
            }
        }

        for (; fCurrentCodec < fImages.count(); fCurrentCodec++) {
            fSourceType = "image";
            fBenchType = "skcodec";
            const SkString& path = fImages[fCurrentCodec];
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }
            sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
            std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(encoded));
            if (!codec) {
                // Nothing to time.
                SkDebugf("Cannot find codec for %s\n", path.c_str());
                continue;
            }

            while (fCurrentColorType < fColorTypes.count()) {
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
                SkAutoMalloc storage(info.getSafeSize(rowBytes));

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
        for (; fCurrentAndroidCodec < fImages.count(); fCurrentAndroidCodec++) {
            fSourceType = "image";
            fBenchType = "skandroidcodec";

            const SkString& path = fImages[fCurrentAndroidCodec];
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }
            sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
            std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::NewFromData(encoded));
            if (!codec) {
                // Nothing to time.
                SkDebugf("Cannot find codec for %s\n", path.c_str());
                continue;
            }

            while (fCurrentSampleSize < (int) SK_ARRAY_COUNT(sampleSizes)) {
                int sampleSize = sampleSizes[fCurrentSampleSize];
                fCurrentSampleSize++;
                if (10 * sampleSize > SkTMin(codec->getInfo().width(), codec->getInfo().height())) {
                    // Avoid benchmarking scaled decodes of already small images.
                    break;
                }

                return new AndroidCodecBench(SkOSPath::Basename(path.c_str()),
                                             encoded.get(), sampleSize);
            }
            fCurrentSampleSize = 0;
        }

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
        for (; fCurrentBRDImage < fImages.count(); fCurrentBRDImage++) {
            fSourceType = "image";
            fBenchType = "BRD";

            const SkString& path = fImages[fCurrentBRDImage];
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                continue;
            }

            while (fCurrentColorType < fColorTypes.count()) {
                while (fCurrentSampleSize < (int) SK_ARRAY_COUNT(brdSampleSizes)) {
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

        while (fCurrentColorImage < fColorImages.count()) {
            fSourceType = "colorimage";
            fBenchType = "skcolorcodec";
            const SkString& path = fColorImages[fCurrentColorImage];
            fCurrentColorImage++;
            sk_sp<SkData> encoded = SkData::MakeFromFileName(path.c_str());
            if (encoded) {
                return new ColorCodecBench(SkOSPath::Basename(path.c_str()).c_str(),
                                           std::move(encoded));
            } else {
                SkDebugf("Could not read file %s.\n", path.c_str());
            }
        }

        return nullptr;
    }

    void fillCurrentOptions(ResultsWriter* log) const {
        log->configOption("source_type", fSourceType);
        log->configOption("bench_type",  fBenchType);
        if (0 == strcmp(fSourceType, "skp")) {
            log->configOption("clip",
                    SkStringPrintf("%d %d %d %d", fClip.fLeft, fClip.fTop,
                                                  fClip.fRight, fClip.fBottom).c_str());
            SkASSERT_RELEASE(fCurrentScale < fScales.count());  // debugging paranoia
            log->configOption("scale", SkStringPrintf("%.2g", fScales[fCurrentScale]).c_str());
            if (fCurrentUseMPD > 0) {
                SkASSERT(1 == fCurrentUseMPD || 2 == fCurrentUseMPD);
                log->configOption("multi_picture_draw", fUseMPDs[fCurrentUseMPD-1] ? "true" : "false");
            }
        }
        if (0 == strcmp(fBenchType, "recording")) {
            log->metric("bytes", fSKPBytes);
            log->metric("ops",   fSKPOps);
        }
    }

private:
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

    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkIRect            fClip;
    SkTArray<SkScalar> fScales;
    SkTArray<SkString> fSKPs;
    SkTArray<SkString> fSVGs;
    SkTArray<bool>     fUseMPDs;
    SkTArray<SkString> fImages;
    SkTArray<SkString> fColorImages;
    SkTArray<SkColorType, true> fColorTypes;
    SkScalar           fZoomMax;
    double             fZoomPeriodMs;

    double fSKPBytes, fSKPOps;

    const char* fSourceType;  // What we're benching: bench, GM, SKP, ...
    const char* fBenchType;   // How we bench it: micro, recording, playback, ...
    int fCurrentRecording;
    int fCurrentPiping;
    int fCurrentScale;
    int fCurrentSKP;
    int fCurrentSVG;
    int fCurrentUseMPD;
    int fCurrentCodec;
    int fCurrentAndroidCodec;
    int fCurrentBRDImage;
    int fCurrentColorImage;
    int fCurrentColorType;
    int fCurrentAlphaType;
    int fCurrentSubsetType;
    int fCurrentSampleSize;
    int fCurrentAnimSKP;
};

// Some runs (mostly, Valgrind) are so slow that the bot framework thinks we've hung.
// This prints something every once in a while so that it knows we're still working.
static void start_keepalive() {
    struct Loop {
        static void forever(void*) {
            for (;;) {
                static const int kSec = 1200;
            #if defined(SK_BUILD_FOR_WIN)
                Sleep(kSec * 1000);
            #else
                sleep(kSec);
            #endif
                SkDebugf("\nBenchmarks still running...\n");
            }
        }
    };
    static SkThread* intentionallyLeaked = new SkThread(Loop::forever);
    intentionallyLeaked->start();
}

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_trace) {
        SkEventTracer::SetInstance(new SkDebugfTracer);
    }
#if defined(SK_BUILD_FOR_IOS)
    cd_Documents();
#endif
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);

#if SK_SUPPORT_GPU
    GrContextOptions grContextOpts;
    grContextOpts.fGpuPathRenderers = CollectGpuPathRenderersFromFlags();
    gGrFactory.reset(new GrContextFactory(grContextOpts));
#endif

    if (FLAGS_veryVerbose) {
        FLAGS_verbose = true;
    }

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

    std::unique_ptr<ResultsWriter> log(new ResultsWriter);
    if (!FLAGS_outResultsFile.isEmpty()) {
#if defined(SK_RELEASE)
        log.reset(new NanoJSONResultsWriter(FLAGS_outResultsFile[0]));
#else
        SkDebugf("I'm ignoring --outResultsFile because this is a Debug build.");
        return 1;
#endif
    }

    if (1 == FLAGS_properties.count() % 2) {
        SkDebugf("ERROR: --properties must be passed with an even number of arguments.\n");
        return 1;
    }
    for (int i = 1; i < FLAGS_properties.count(); i += 2) {
        log->property(FLAGS_properties[i-1], FLAGS_properties[i]);
    }

    if (1 == FLAGS_key.count() % 2) {
        SkDebugf("ERROR: --key must be passed with an even number of arguments.\n");
        return 1;
    }
    for (int i = 1; i < FLAGS_key.count(); i += 2) {
        log->key(FLAGS_key[i-1], FLAGS_key[i]);
    }

    const double overhead = estimate_timer_overhead();
    SkDebugf("Timer overhead: %s\n", HUMANIZE(overhead));

    SkTArray<double> samples;

    if (kAutoTuneLoops != FLAGS_loops) {
        SkDebugf("Fixed number of loops; times would only be misleading so we won't print them.\n");
    } else if (FLAGS_quiet) {
        SkDebugf("! -> high variance, ? -> moderate variance\n");
        SkDebugf("    micros   \tbench\n");
    } else if (FLAGS_ms) {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\tsamples\tconfig\tbench\n");
    } else {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tconfig\tbench\n",
                 FLAGS_samples, "samples");
    }

    SkTArray<Config> configs;
    create_configs(&configs);

#ifdef THERMAL_MANAGER_SUPPORTED
    int tmEnabled, tmThreshold, tmSleepTimeMs, tmTimeoutMs;
    if (4 != sscanf(FLAGS_useThermalManager[0], "%d,%d,%d,%d",
                    &tmEnabled, &tmThreshold, &tmSleepTimeMs, &tmTimeoutMs)) {
        SkDebugf("Can't parse %s from --useThermalManager.\n", FLAGS_useThermalManager[0]);
        exit(1);
    }
    ThermalManager tm(tmThreshold, tmSleepTimeMs, tmTimeoutMs);
#endif

    if (FLAGS_keepAlive) {
        start_keepalive();
    }

    gSkUseAnalyticAA = FLAGS_analyticAA;

    if (FLAGS_forceAnalyticAA) {
        gSkForceAnalyticAA = true;
    }
    if (FLAGS_forceRasterPipeline) {
        gSkForceRasterPipelineBlitter = true;
    }

    int runs = 0;
    BenchmarkStream benchStream;
    while (Benchmark* b = benchStream.next()) {
        std::unique_ptr<Benchmark> bench(b);
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName())) {
            continue;
        }

        if (!configs.empty()) {
            log->bench(bench->getUniqueName(), bench->getSize().fX, bench->getSize().fY);
            bench->delayedSetup();
        }
        for (int i = 0; i < configs.count(); ++i) {
#ifdef THERMAL_MANAGER_SUPPORTED
            if (tmEnabled && !tm.coolOffIfNecessary()) {
                SkDebugf("Could not cool off, timings will be throttled\n");
            }
#endif
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

            target->setup();
            bench->perCanvasPreDraw(canvas);

            int maxFrameLag;
            int loops = target->needsFrameTiming(&maxFrameLag)
                ? setup_gpu_bench(target, bench.get(), maxFrameLag)
                : setup_cpu_bench(overhead, target, bench.get());

            if (FLAGS_ms) {
                samples.reset();
                auto stop = now_ms() + FLAGS_ms;
                do {
                    samples.push_back(time(loops, bench.get(), target) / loops);
                } while (now_ms() < stop);
            } else {
                samples.reset(FLAGS_samples);
                for (int s = 0; s < FLAGS_samples; s++) {
                    samples[s] = time(loops, bench.get(), target) / loops;
                }
            }

#if SK_SUPPORT_GPU
            SkTArray<SkString> keys;
            SkTArray<double> values;
            bool gpuStatsDump = FLAGS_gpuStatsDump && Benchmark::kGPU_Backend == configs[i].backend;
            if (gpuStatsDump) {
                // TODO cache stats
                bench->getGpuStats(canvas, &keys, &values);
            }
#endif

            bench->perCanvasPostDraw(canvas);

            if (Benchmark::kNonRendering_Backend != target->config.backend &&
                !FLAGS_writePath.isEmpty() && FLAGS_writePath[0]) {
                SkString pngFilename = SkOSPath::Join(FLAGS_writePath[0], config);
                pngFilename = SkOSPath::Join(pngFilename.c_str(), bench->getUniqueName());
                pngFilename.append(".png");
                write_canvas_png(target, pngFilename);
            }

            if (kFailedLoops == loops) {
                // Can't be timed.  A warning note has already been printed.
                cleanup_run(target);
                continue;
            }

            Stats stats(samples);
            log->config(config);
            log->configOption("name", bench->getName());
            benchStream.fillCurrentOptions(log.get());
            target->fillOptions(log.get());
            log->metric("min_ms",    stats.min);
            log->metrics("samples",    samples);
#if SK_SUPPORT_GPU
            if (gpuStatsDump) {
                // dump to json, only SKPBench currently returns valid keys / values
                SkASSERT(keys.count() == values.count());
                for (int i = 0; i < keys.count(); i++) {
                    log->metric(keys[i].c_str(), values[i]);
                }
            }
#endif

            if (runs++ % FLAGS_flushEvery == 0) {
                log->flush();
            }

            if (kAutoTuneLoops != FLAGS_loops) {
                if (configs.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%4d/%-4dMB\t%s\t%s\n"
                         , sk_tools::getCurrResidentSetSizeMB()
                         , sk_tools::getMaxResidentSetSizeMB()
                         , bench->getUniqueName()
                         , config);
            } else if (FLAGS_quiet) {
                const char* mark = " ";
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                if (stddev_percent >  5) mark = "?";
                if (stddev_percent > 10) mark = "!";

                SkDebugf("%10.2f %s\t%s\t%s\n",
                         stats.median*1e3, mark, bench->getUniqueName(), config);
            } else if (FLAGS_csv) {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
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
                const char* format = "%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n";
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf(format
                        , sk_tools::getCurrResidentSetSizeMB()
                        , sk_tools::getMaxResidentSetSizeMB()
                        , loops
                        , HUMANIZE(stats.min)
                        , HUMANIZE(stats.median)
                        , HUMANIZE(stats.mean)
                        , HUMANIZE(stats.max)
                        , stddev_percent
                        , FLAGS_ms ? to_string(samples.count()).c_str() : stats.plot.c_str()
                        , config
                        , bench->getUniqueName()
                        );
            }

#if SK_SUPPORT_GPU
            if (FLAGS_gpuStats && Benchmark::kGPU_Backend == configs[i].backend) {
                GrContext* context = gGrFactory->get(configs[i].ctxType,
                                                     configs[i].ctxOverrides);
                context->printCacheStats();
                context->printGpuStats();
            }
#endif

            if (FLAGS_verbose) {
                SkDebugf("Samples:  ");
                for (int i = 0; i < samples.count(); i++) {
                    SkDebugf("%s  ", HUMANIZE(samples[i]));
                }
                SkDebugf("%s\n", bench->getUniqueName());
            }
            cleanup_run(target);
        }
    }

    SkGraphics::PurgeAllCaches();

    log->bench("memory_usage", 0,0);
    log->config("meta");
    log->metric("max_rss_mb", sk_tools::getMaxResidentSetSizeMB());

#if SK_SUPPORT_GPU
    // Make sure we clean up the global GrContextFactory here, otherwise we might race with the
    // SkEventTracer destructor
    gGrFactory.reset(nullptr);
#endif

    return 0;
}
