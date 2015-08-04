/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "nanobench.h"

#include "Benchmark.h"
#include "CodecBench.h"
#include "CrashHandler.h"
#include "DecodingBench.h"
#include "GMBench.h"
#include "ProcStats.h"
#include "ResultsWriter.h"
#include "RecordingBench.h"
#include "SKPAnimationBench.h"
#include "SKPBench.h"
#include "SubsetBenchPriv.h"
#include "SubsetSingleBench.h"
#include "SubsetTranslateBench.h"
#include "SubsetZoomBench.h"
#include "Stats.h"
#include "Timer.h"

#include "SkBBoxHierarchy.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCommonFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkPictureUtils.h"
#include "SkScanlineDecoder.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include "nanobenchAndroid.h"
#endif

#if SK_SUPPORT_GPU
    #include "gl/GrGLDefines.h"
    #include "GrCaps.h"
    #include "GrContextFactory.h"
    SkAutoTDelete<GrContextFactory> gGrFactory;
#endif

    struct GrContextOptions;

__SK_FORCE_IMAGE_DECODER_LINKING;

static const int kTimedSampling = 0;

static const int kAutoTuneLoops = 0;

static const int kDefaultLoops =
#ifdef SK_DEBUG
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
DEFINE_string(samplingTime, "0", "Amount of time to run each bench. Takes precedence over samples."
                                 "Must be \"0\", \"%%lfs\", or \"%%lfms\"");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_int32(gpuFrameLag, 5, "If unknown, estimated maximum number of frames GPU allows to lag.");
DEFINE_bool(gpuCompressAlphaMasks, false, "Compress masks generated from falling back to "
                                          "software path rendering.");

DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_int32(maxCalibrationAttempts, 3,
             "Try up to this many times to guess loops for a bench, or skip the bench.");
DEFINE_int32(maxLoops, 1000000, "Never run a bench more times than this.");
DEFINE_string(clip, "0,0,1000,1000", "Clip for SKPs.");
DEFINE_string(scales, "1.0", "Space-separated scales for SKPs.");
DEFINE_string(zoom, "1.0,0", "Comma-separated zoomMax,zoomPeriodMs factors for a periodic SKP zoom "
                             "function that ping-pongs between 1.0 and zoomMax.");
DEFINE_bool(bbh, true, "Build a BBH for SKPs?");
DEFINE_bool(mpd, true, "Use MultiPictureDraw for the SKPs?");
DEFINE_bool(loopSKP, true, "Loop SKPs like we do for micro benches?");
DEFINE_int32(flushEvery, 10, "Flush --outResultsFile every Nth run.");
DEFINE_bool(resetGpuContext, true, "Reset the GrContext before running each test.");
DEFINE_bool(gpuStats, false, "Print GPU stats after each gpu benchmark?");

static SkString humanize(double ms) {
    if (FLAGS_verbose) return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    return HumanizeMs(ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

bool Target::init(SkImageInfo info, Benchmark* bench) {
    if (Benchmark::kRaster_Backend == config.backend) {
        this->surface.reset(SkSurface::NewRaster(info));
        if (!this->surface.get()) {
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
    bmp->setInfo(canvas->imageInfo());
    if (!canvas->readPixels(bmp, 0, 0)) {
        SkDebugf("Can't read canvas pixels.\n");
        return false;
    }
    return true;
}

#if SK_SUPPORT_GPU
struct GPUTarget : public Target {
    explicit GPUTarget(const Config& c) : Target(c), gl(NULL) { }
    SkGLContext* gl;

    void setup() override {
        this->gl->makeCurrent();
        // Make sure we're done with whatever came before.
        SK_GL(*this->gl, Finish());
    }
    void endTiming() override {
        if (this->gl) {
            SK_GL(*this->gl, Flush());
            this->gl->swapBuffers();
        }
    }
    void fence() override {
        SK_GL(*this->gl, Finish());
    }

    bool needsFrameTiming(int* maxFrameLag) const override {
        if (!this->gl->getMaxGpuFrameLag(maxFrameLag)) {
            // Frame lag is unknown.
            *maxFrameLag = FLAGS_gpuFrameLag;
        }
        return true;
    }
    bool init(SkImageInfo info, Benchmark* bench) override {
        uint32_t flags = this->config.useDFText ? SkSurfaceProps::kUseDistanceFieldFonts_Flag : 0;
        SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
        this->surface.reset(SkSurface::NewRenderTarget(gGrFactory->get(this->config.ctxType),
                                                         SkSurface::kNo_Budgeted, info,
                                                         this->config.samples, &props));
        this->gl = gGrFactory->getGLContext(this->config.ctxType);
        if (!this->surface.get()) {
            return false;
        }
        if (!this->gl->fenceSyncSupport()) {
            SkDebugf("WARNING: GL context for config \"%s\" does not support fence sync. "
                     "Timings might not be accurate.\n", this->config.name);
        }
        return true;
    }
    void fillOptions(ResultsWriter* log) override {
        const GrGLubyte* version;
        SK_GL_RET(*this->gl, version, GetString(GR_GL_VERSION));
        log->configOption("GL_VERSION", (const char*)(version));

        SK_GL_RET(*this->gl, version, GetString(GR_GL_RENDERER));
        log->configOption("GL_RENDERER", (const char*) version);

        SK_GL_RET(*this->gl, version, GetString(GR_GL_VENDOR));
        log->configOption("GL_VENDOR", (const char*) version);

        SK_GL_RET(*this->gl, version, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
        log->configOption("GL_SHADING_LANGUAGE_VERSION", (const char*) version);
    }
};

#endif

static double time(int loops, Benchmark* bench, Target* target) {
    SkCanvas* canvas = target->getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    WallTimer timer;
    timer.start();
    canvas = target->beginTiming(canvas);
    bench->draw(loops, canvas);
    if (canvas) {
        canvas->flush();
    }
    target->endTiming();
    timer.end();
    return timer.fWall;
}

static double estimate_timer_overhead() {
    double overhead = 0;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        WallTimer timer;
        timer.start();
        timer.end();
        overhead += timer.fWall;
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
    if (!SkImageEncoder::EncodeStream(&stream, bmp, SkImageEncoder::kPNG_Type, 100)) {
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
    // where bench_plus_overhead ≈ overhead + Bench Time.
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

static SkString to_lower(const char* str) {
    SkString lower(str);
    for (size_t i = 0; i < lower.size(); i++) {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

static bool is_cpu_config_allowed(const char* name) {
    for (int i = 0; i < FLAGS_config.count(); i++) {
        if (to_lower(FLAGS_config[i]).equals(name)) {
            return true;
        }
    }
    return false;
}

#if SK_SUPPORT_GPU
static bool is_gpu_config_allowed(const char* name, GrContextFactory::GLContextType ctxType,
                                  int sampleCnt) {
    if (!is_cpu_config_allowed(name)) {
        return false;
    }
    if (const GrContext* ctx = gGrFactory->get(ctxType)) {
        return sampleCnt <= ctx->caps()->maxSampleCount();
    }
    return false;
}
#endif

#if SK_SUPPORT_GPU
#define kBogusGLContextType GrContextFactory::kNative_GLContextType
#else
#define kBogusGLContextType 0
#endif

// Append all configs that are enabled and supported.
static void create_configs(SkTDArray<Config>* configs) {
    #define CPU_CONFIG(name, backend, color, alpha)                       \
        if (is_cpu_config_allowed(#name)) {                               \
            Config config = { #name, Benchmark::backend, color, alpha, 0, \
                              kBogusGLContextType, false };               \
            configs->push(config);                                        \
        }

    if (FLAGS_cpu) {
        CPU_CONFIG(nonrendering, kNonRendering_Backend, kUnknown_SkColorType, kUnpremul_SkAlphaType)
        CPU_CONFIG(8888, kRaster_Backend, kN32_SkColorType, kPremul_SkAlphaType)
        CPU_CONFIG(565, kRaster_Backend, kRGB_565_SkColorType, kOpaque_SkAlphaType)
    }

#if SK_SUPPORT_GPU
    #define GPU_CONFIG(name, ctxType, samples, useDFText)                        \
        if (is_gpu_config_allowed(#name, GrContextFactory::ctxType, samples)) {  \
            Config config = {                                                    \
                #name,                                                           \
                Benchmark::kGPU_Backend,                                         \
                kN32_SkColorType,                                                \
                kPremul_SkAlphaType,                                             \
                samples,                                                         \
                GrContextFactory::ctxType,                                       \
                useDFText };                                                     \
            configs->push(config);                                               \
        }

    if (FLAGS_gpu) {
        GPU_CONFIG(gpu, kNative_GLContextType, 0, false)
        GPU_CONFIG(msaa4, kNative_GLContextType, 4, false)
        GPU_CONFIG(msaa16, kNative_GLContextType, 16, false)
        GPU_CONFIG(nvprmsaa4, kNVPR_GLContextType, 4, false)
        GPU_CONFIG(nvprmsaa16, kNVPR_GLContextType, 16, false)
        GPU_CONFIG(gpudft, kNative_GLContextType, 0, true)
        GPU_CONFIG(debug, kDebug_GLContextType, 0, false)
        GPU_CONFIG(nullgpu, kNull_GLContextType, 0, false)
#ifdef SK_ANGLE
        GPU_CONFIG(angle, kANGLE_GLContextType, 0, false)
#endif
#if SK_MESA
        GPU_CONFIG(mesa, kMESA_GLContextType, 0, false)
#endif
    }
#endif

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if (is_cpu_config_allowed("hwui")) {
        Config config = { "hwui", Benchmark::kHWUI_Backend, kRGBA_8888_SkColorType,
                          kPremul_SkAlphaType, 0, kBogusGLContextType, false };
        configs->push(config);
    }
#endif
}

// If bench is enabled for config, returns a Target* for it, otherwise NULL.
static Target* is_enabled(Benchmark* bench, const Config& config) {
    if (!bench->isSuitableFor(config.backend)) {
        return NULL;
    }

    SkImageInfo info = SkImageInfo::Make(bench->getSize().fX, bench->getSize().fY,
                                         config.color, config.alpha);

    Target* target = NULL;

    switch (config.backend) {
#if SK_SUPPORT_GPU
    case Benchmark::kGPU_Backend:
        target = new GPUTarget(config);
        break;
#endif
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    case Benchmark::kHWUI_Backend:
        target = new HWUITarget(config, bench);
        break;
#endif
    default:
        target = new Target(config);
        break;
    }

    if (!target->init(info, bench)) {
        delete target;
        return NULL;
    }
    return target;
}

/*
 * Returns true if set up for a subset decode succeeds, false otherwise
 * If the set-up succeeds, the width and height parameters will be set
 */
static bool valid_subset_bench(const SkString& path, SkColorType colorType, bool useCodec,
        int* width, int* height) {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    SkAutoTDelete<SkMemoryStream> stream(new SkMemoryStream(encoded));

    // Check that we can create a codec or image decoder.
    if (useCodec) {
        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.detach()));
        if (NULL == codec) {
            SkDebugf("Could not create codec for %s.  Skipping bench.\n", path.c_str());
            return false;
        }

        // These will be initialized by SkCodec if the color type is kIndex8 and
        // unused otherwise.
        SkPMColor colors[256];
        int colorCount;
        const SkImageInfo info = codec->getInfo().makeColorType(colorType);
        SkAutoTDeleteArray<uint8_t> row(SkNEW_ARRAY(uint8_t, info.minRowBytes()));
        SkAutoTDelete<SkScanlineDecoder> scanlineDecoder(SkScanlineDecoder::NewFromData(encoded));
        if (NULL == scanlineDecoder || scanlineDecoder->start(info, NULL,
                colors, &colorCount) != SkCodec::kSuccess)
        {
            SkDebugf("Could not create scanline decoder for %s with color type %s.  "
                    "Skipping bench.\n", path.c_str(), get_color_name(colorType));
            return false;
        }
        *width = info.width();
        *height = info.height();
    } else {
        SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
        if (NULL == decoder) {
            SkDebugf("Could not create decoder for %s.  Skipping bench.\n", path.c_str());
            return false;
        }
        //FIXME: See skbug.com/3921
        if (kIndex_8_SkColorType == colorType || kGray_8_SkColorType == colorType) {
            SkDebugf("Cannot use image subset decoder for %s with color type %s.  "
                    "Skipping bench.\n", path.c_str(), get_color_name(colorType));
            return false;
        }
        if (!decoder->buildTileIndex(stream.detach(), width, height)) {
            SkDebugf("Could not build tile index for %s.  Skipping bench.\n", path.c_str());
            return false;
        }
    }

    // Check if the image is large enough for a meaningful subset benchmark.
    if (*width <= 512 && *height <= 512) {
        // This should not print a message since it is not an error.
        return false;
    }

    return true;
}

static void cleanup_run(Target* target) {
    SkDELETE(target);
#if SK_SUPPORT_GPU
    if (FLAGS_abandonGpuContext) {
        gGrFactory->abandonContexts();
    }
    if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
        gGrFactory->destroyContexts();
    }
#endif
}

class BenchmarkStream {
public:
    BenchmarkStream() : fBenches(BenchRegistry::Head())
                      , fGMs(skiagm::GMRegistry::Head())
                      , fCurrentRecording(0)
                      , fCurrentScale(0)
                      , fCurrentSKP(0)
                      , fCurrentUseMPD(0)
                      , fCurrentCodec(0)
                      , fCurrentImage(0)
                      , fCurrentSubsetImage(0)
                      , fCurrentColorType(0)
                      , fCurrentSubsetType(0)
                      , fUseCodec(0)
                      , fCurrentAnimSKP(0) {
        for (int i = 0; i < FLAGS_skps.count(); i++) {
            if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
                fSKPs.push_back() = FLAGS_skps[i];
            } else {
                SkOSFile::Iter it(FLAGS_skps[i], ".skp");
                SkString path;
                while (it.next(&path)) {
                    fSKPs.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
                }
            }
        }

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
        for (int i = 0; i < FLAGS_images.count(); i++) {
            const char* flag = FLAGS_images[i];
            if (sk_isdir(flag)) {
                // If the value passed in is a directory, add all the images
                SkOSFile::Iter it(flag);
                SkString file;
                while (it.next(&file)) {
                    fImages.push_back() = SkOSPath::Join(flag, file.c_str());
                }
            } else if (sk_exists(flag)) {
                // Also add the value if it is a single image
                fImages.push_back() = flag;
            }
        }

        // Choose the candidate color types for image decoding
        const SkColorType colorTypes[] =
            { kN32_SkColorType,
              kRGB_565_SkColorType,
              kAlpha_8_SkColorType,
              kIndex_8_SkColorType,
              kGray_8_SkColorType };
        fColorTypes.push_back_n(SK_ARRAY_COUNT(colorTypes), colorTypes);
    }

    static bool ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic) {
        // Not strictly necessary, as it will be checked again later,
        // but helps to avoid a lot of pointless work if we're going to skip it.
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
            return false;
        }

        SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
        if (stream.get() == NULL) {
            SkDebugf("Could not read %s.\n", path);
            return false;
        }

        pic->reset(SkPicture::CreateFromStream(stream.get()));
        if (pic->get() == NULL) {
            SkDebugf("Could not read %s as an SkPicture.\n", path);
            return false;
        }
        return true;
    }

    Benchmark* next() {
        if (fBenches) {
            Benchmark* bench = fBenches->factory()(NULL);
            fBenches = fBenches->next();
            fSourceType = "bench";
            fBenchType  = "micro";
            return bench;
        }

        while (fGMs) {
            SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(NULL));
            fGMs = fGMs->next();
            if (gm->runAsBench()) {
                fSourceType = "gm";
                fBenchType  = "micro";
                return SkNEW_ARGS(GMBench, (gm.detach()));
            }
        }

        // First add all .skps as RecordingBenches.
        while (fCurrentRecording < fSKPs.count()) {
            const SkString& path = fSKPs[fCurrentRecording++];
            SkAutoTUnref<SkPicture> pic;
            if (!ReadPicture(path.c_str(), &pic)) {
                continue;
            }
            SkString name = SkOSPath::Basename(path.c_str());
            fSourceType = "skp";
            fBenchType  = "recording";
            fSKPBytes = static_cast<double>(SkPictureUtils::ApproximateBytesUsed(pic));
            fSKPOps   = pic->approximateOpCount();
            return SkNEW_ARGS(RecordingBench, (name.c_str(), pic.get(), FLAGS_bbh));
        }

        // Then once each for each scale as SKPBenches (playback).
        while (fCurrentScale < fScales.count()) {
            while (fCurrentSKP < fSKPs.count()) {
                const SkString& path = fSKPs[fCurrentSKP];
                SkAutoTUnref<SkPicture> pic;
                if (!ReadPicture(path.c_str(), &pic)) {
                    fCurrentSKP++;
                    continue;
                }

                while (fCurrentUseMPD < fUseMPDs.count()) {
                    if (FLAGS_bbh) {
                        // The SKP we read off disk doesn't have a BBH.  Re-record so it grows one.
                        SkRTreeFactory factory;
                        SkPictureRecorder recorder;
                        static const int kFlags = SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag;
                        pic->playback(recorder.beginRecording(pic->cullRect().width(),
                                                              pic->cullRect().height(),
                                                              &factory,
                                                              fUseMPDs[fCurrentUseMPD] ? kFlags : 0));
                        pic.reset(recorder.endRecording());
                    }
                    SkString name = SkOSPath::Basename(path.c_str());
                    fSourceType = "skp";
                    fBenchType = "playback";
                    return SkNEW_ARGS(SKPBench,
                                      (name.c_str(), pic.get(), fClip, fScales[fCurrentScale],
                                       fUseMPDs[fCurrentUseMPD++], FLAGS_loopSKP));

                }
                fCurrentUseMPD = 0;
                fCurrentSKP++;
            }
            fCurrentSKP = 0;
            fCurrentScale++;
        }

        // Now loop over each skp again if we have an animation
        if (fZoomMax != 1.0f && fZoomPeriodMs > 0) {
            while (fCurrentAnimSKP < fSKPs.count()) {
                const SkString& path = fSKPs[fCurrentAnimSKP];
                SkAutoTUnref<SkPicture> pic;
                if (!ReadPicture(path.c_str(), &pic)) {
                    fCurrentAnimSKP++;
                    continue;
                }

                fCurrentAnimSKP++;
                SkString name = SkOSPath::Basename(path.c_str());
                SkAutoTUnref<SKPAnimationBench::Animation> animation(
                    SKPAnimationBench::CreateZoomAnimation(fZoomMax, fZoomPeriodMs));
                return SkNEW_ARGS(SKPAnimationBench, (name.c_str(), pic.get(), fClip, animation,
                                                      FLAGS_loopSKP));
            }
        }


        for (; fCurrentCodec < fImages.count(); fCurrentCodec++) {
            const SkString& path = fImages[fCurrentCodec];
            SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded));
            if (!codec) {
                // Nothing to time.
                SkDebugf("Cannot find codec for %s\n", path.c_str());
                continue;
            }

            while (fCurrentColorType < fColorTypes.count()) {
                const SkColorType colorType = fColorTypes[fCurrentColorType];
                fCurrentColorType++;

                // Make sure we can decode to this color type.
                SkImageInfo info = codec->getInfo().makeColorType(colorType);
                SkAlphaType alphaType;
                if (!SkColorTypeValidateAlphaType(colorType, info.alphaType(),
                                                  &alphaType)) {
                    continue;
                }
                if (alphaType != info.alphaType()) {
                    info = info.makeAlphaType(alphaType);
                }

                const size_t rowBytes = info.minRowBytes();
                SkAutoMalloc storage(info.getSafeSize(rowBytes));

                // Used if fCurrentColorType is kIndex_8_SkColorType
                int colorCount = 256;
                SkPMColor colors[256];

                const SkCodec::Result result = codec->getPixels(
                        info, storage.get(), rowBytes, NULL, colors,
                        &colorCount);
                switch (result) {
                    case SkCodec::kSuccess:
                    case SkCodec::kIncompleteInput:
                        return new CodecBench(SkOSPath::Basename(path.c_str()),
                                encoded, colorType);
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

        // Run the DecodingBenches
        while (fCurrentImage < fImages.count()) {
            while (fCurrentColorType < fColorTypes.count()) {
                const SkString& path = fImages[fCurrentImage];
                SkColorType colorType = fColorTypes[fCurrentColorType];
                fCurrentColorType++;
                // Check if the image decodes to the right color type
                // before creating the benchmark
                SkBitmap bitmap;
                if (SkImageDecoder::DecodeFile(path.c_str(), &bitmap,
                        colorType, SkImageDecoder::kDecodePixels_Mode)
                        && bitmap.colorType() == colorType) {
                    return new DecodingBench(path, colorType);
                }
            }
            fCurrentColorType = 0;
            fCurrentImage++;
        }

        // Run the SubsetBenches
        bool useCodecOpts[] = { true, false };
        while (fUseCodec < 2) {
            bool useCodec = useCodecOpts[fUseCodec];
            while (fCurrentSubsetImage < fImages.count()) {
                while (fCurrentColorType < fColorTypes.count()) {
                    const SkString& path = fImages[fCurrentSubsetImage];
                    SkColorType colorType = fColorTypes[fCurrentColorType];
                    while (fCurrentSubsetType <= kLast_SubsetType) {
                        int width = 0;
                        int height = 0;
                        int currentSubsetType = fCurrentSubsetType++;
                        if (valid_subset_bench(path, colorType, useCodec, &width, &height)) {
                            switch (currentSubsetType) {
                                case kTopLeft_SubsetType:
                                    return new SubsetSingleBench(path, colorType, width/3,
                                            height/3, 0, 0, useCodec);
                                case kTopRight_SubsetType:
                                    return new SubsetSingleBench(path, colorType, width/3,
                                            height/3, 2*width/3, 0, useCodec);
                                case kMiddle_SubsetType:
                                    return new SubsetSingleBench(path, colorType, width/3,
                                            height/3, width/3, height/3, useCodec);
                                case kBottomLeft_SubsetType:
                                    return new SubsetSingleBench(path, colorType, width/3,
                                            height/3, 0, 2*height/3, useCodec);
                                case kBottomRight_SubsetType:
                                    return new SubsetSingleBench(path, colorType, width/3,
                                            height/3, 2*width/3, 2*height/3, useCodec);
                                case kTranslate_SubsetType:
                                    return new SubsetTranslateBench(path, colorType, 512, 512,
                                            useCodec);
                                case kZoom_SubsetType:
                                    return new SubsetZoomBench(path, colorType, 512, 512,
                                            useCodec);
                            }
                        } else {
                            break;
                        }
                    }
                    fCurrentSubsetType = 0;
                    fCurrentColorType++;
                }
                fCurrentColorType = 0;
                fCurrentSubsetImage++;
            }
            fCurrentSubsetImage = 0;
            fUseCodec++;
        }

        return NULL;
    }

    void fillCurrentOptions(ResultsWriter* log) const {
        log->configOption("source_type", fSourceType);
        log->configOption("bench_type",  fBenchType);
        if (0 == strcmp(fSourceType, "skp")) {
            log->configOption("clip",
                    SkStringPrintf("%d %d %d %d", fClip.fLeft, fClip.fTop,
                                                  fClip.fRight, fClip.fBottom).c_str());
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
        kLast_SubsetType        = kZoom_SubsetType
    };

    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkIRect            fClip;
    SkTArray<SkScalar> fScales;
    SkTArray<SkString> fSKPs;
    SkTArray<bool>     fUseMPDs;
    SkTArray<SkString> fImages;
    SkTArray<SkColorType> fColorTypes;
    SkScalar           fZoomMax;
    double             fZoomPeriodMs;

    double fSKPBytes, fSKPOps;

    const char* fSourceType;  // What we're benching: bench, GM, SKP, ...
    const char* fBenchType;   // How we bench it: micro, recording, playback, ...
    int fCurrentRecording;
    int fCurrentScale;
    int fCurrentSKP;
    int fCurrentUseMPD;
    int fCurrentCodec;
    int fCurrentImage;
    int fCurrentSubsetImage;
    int fCurrentColorType;
    int fCurrentSubsetType;
    int fUseCodec;
    int fCurrentAnimSKP;
};

int nanobench_main();
int nanobench_main() {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);

#if SK_SUPPORT_GPU
    GrContextOptions grContextOpts;
    grContextOpts.fDrawPathToCompressedTexture = FLAGS_gpuCompressAlphaMasks;
    gGrFactory.reset(SkNEW_ARGS(GrContextFactory, (grContextOpts)));
#endif

    if (FLAGS_veryVerbose) {
        FLAGS_verbose = true;
    }

    double samplingTimeMs = 0;
    if (0 != strcmp("0", FLAGS_samplingTime[0])) {
        SkSTArray<8, char> timeUnit;
        timeUnit.push_back_n(static_cast<int>(strlen(FLAGS_samplingTime[0])) + 1);
        if (2 != sscanf(FLAGS_samplingTime[0], "%lf%s", &samplingTimeMs, timeUnit.begin()) ||
            (0 != strcmp("s", timeUnit.begin()) && 0 != strcmp("ms", timeUnit.begin()))) {
            SkDebugf("Invalid --samplingTime \"%s\". Must be \"0\", \"%%lfs\", or \"%%lfms\"\n",
                     FLAGS_samplingTime[0]);
            exit(0);
        }
        if (0 == strcmp("s", timeUnit.begin())) {
            samplingTimeMs *= 1000;
        }
        if (samplingTimeMs) {
            FLAGS_samples = kTimedSampling;
        }
    }

    if (kAutoTuneLoops != FLAGS_loops) {
        FLAGS_samples     = 1;
        FLAGS_gpuFrameLag = 0;
    }

    if (!FLAGS_writePath.isEmpty()) {
        SkDebugf("Writing files to %s.\n", FLAGS_writePath[0]);
        if (!sk_mkdir(FLAGS_writePath[0])) {
            SkDebugf("Could not create %s. Files won't be written.\n", FLAGS_writePath[0]);
            FLAGS_writePath.set(0, NULL);
        }
    }

    SkAutoTDelete<ResultsWriter> log(SkNEW(ResultsWriter));
    if (!FLAGS_outResultsFile.isEmpty()) {
        log.reset(SkNEW(NanoJSONResultsWriter(FLAGS_outResultsFile[0])));
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
        SkDebugf("median\tbench\tconfig\n");
    } else if (kTimedSampling == FLAGS_samples) {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\tsamples\tconfig\tbench\n");
    } else {
        SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tconfig\tbench\n",
                 FLAGS_samples, "samples");
    }

    SkTDArray<Config> configs;
    create_configs(&configs);

    int runs = 0;
    BenchmarkStream benchStream;
    while (Benchmark* b = benchStream.next()) {
        SkAutoTDelete<Benchmark> bench(b);
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName())) {
            continue;
        }

        if (!configs.isEmpty()) {
            log->bench(bench->getUniqueName(), bench->getSize().fX, bench->getSize().fY);
            bench->preDraw();
        }
        for (int i = 0; i < configs.count(); ++i) {
            Target* target = is_enabled(b, configs[i]);
            if (!target) {
                continue;
            }

            // During HWUI output this canvas may be NULL.
            SkCanvas* canvas = target->getCanvas();
            const char* config = target->config.name;

            target->setup();
            bench->perCanvasPreDraw(canvas);

            int maxFrameLag;
            const int loops = target->needsFrameTiming(&maxFrameLag)
                ? setup_gpu_bench(target, bench.get(), maxFrameLag)
                : setup_cpu_bench(overhead, target, bench.get());

            if (kTimedSampling != FLAGS_samples) {
                samples.reset(FLAGS_samples);
                for (int s = 0; s < FLAGS_samples; s++) {
                    samples[s] = time(loops, bench, target) / loops;
                }
            } else if (samplingTimeMs) {
                samples.reset();
                if (FLAGS_verbose) {
                    SkDebugf("Begin sampling %s for %ims\n",
                             bench->getUniqueName(), static_cast<int>(samplingTimeMs));
                }
                WallTimer timer;
                timer.start();
                do {
                    samples.push_back(time(loops, bench, target) / loops);
                    timer.end();
                } while (timer.fWall < samplingTimeMs);
            }

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
                if (configs.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\t%s\n", HUMANIZE(stats.median), bench->getUniqueName(), config);
            } else {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf("%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                        , sk_tools::getCurrResidentSetSizeMB()
                        , sk_tools::getMaxResidentSetSizeMB()
                        , loops
                        , HUMANIZE(stats.min)
                        , HUMANIZE(stats.median)
                        , HUMANIZE(stats.mean)
                        , HUMANIZE(stats.max)
                        , stddev_percent
                        , kTimedSampling != FLAGS_samples ? stats.plot.c_str()
                                                          : to_string(samples.count()).c_str()
                        , config
                        , bench->getUniqueName()
                        );
            }
#if SK_SUPPORT_GPU
            if (FLAGS_gpuStats &&
                Benchmark::kGPU_Backend == configs[i].backend) {
                gGrFactory->get(configs[i].ctxType)->printCacheStats();
                gGrFactory->get(configs[i].ctxType)->printGpuStats();
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

    log->bench("memory_usage", 0,0);
    log->config("meta");
    log->metric("max_rss_mb", sk_tools::getMaxResidentSetSizeMB());

#if SK_SUPPORT_GPU
    // Make sure we clean up the global GrContextFactory here, otherwise we might race with the
    // SkEventTracer destructor
    gGrFactory.reset(NULL);
#endif

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return nanobench_main();
}
#endif
