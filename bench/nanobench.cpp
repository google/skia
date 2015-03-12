/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "Benchmark.h"
#include "CrashHandler.h"
#include "DecodingBench.h"
#include "DecodingSubsetBench.h"
#include "GMBench.h"
#include "ProcStats.h"
#include "ResultsWriter.h"
#include "RecordingBench.h"
#include "SKPBench.h"
#include "Stats.h"
#include "Timer.h"

#include "SkBBoxHierarchy.h"
#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkPictureUtils.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"

#if SK_SUPPORT_GPU
    #include "gl/GrGLDefines.h"
    #include "GrContextFactory.h"
    SkAutoTDelete<GrContextFactory> gGrFactory;
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

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

DEFINE_int32(loops, kDefaultLoops, loops_help_txt().c_str());

DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU allows to lag.");
DEFINE_bool(gpuCompressAlphaMasks, false, "Compress masks generated from falling back to "
                                          "software path rendering.");

DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_int32(maxCalibrationAttempts, 3,
             "Try up to this many times to guess loops for a bench, or skip the bench.");
DEFINE_int32(maxLoops, 1000000, "Never run a bench more times than this.");
DEFINE_string(clip, "0,0,1000,1000", "Clip for SKPs.");
DEFINE_string(scales, "1.0", "Space-separated scales for SKPs.");
DEFINE_bool(bbh, true, "Build a BBH for SKPs?");
DEFINE_bool(mpd, true, "Use MultiPictureDraw for the SKPs?");
DEFINE_int32(flushEvery, 10, "Flush --outResultsFile every Nth run.");
DEFINE_bool(resetGpuContext, true, "Reset the GrContext before running each test.");
DEFINE_bool(gpuStats, false, "Print GPU stats after each gpu benchmark?");

static SkString humanize(double ms) {
    if (FLAGS_verbose) return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    return HumanizeMs(ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

static double time(int loops, Benchmark* bench, SkCanvas* canvas, SkGLContext* gl) {
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    WallTimer timer;
    timer.start();
    if (bench) {
        bench->draw(loops, canvas);
    }
    if (canvas) {
        canvas->flush();
    }
#if SK_SUPPORT_GPU
    if (gl) {
        SK_GL(*gl, Flush());
        gl->swapBuffers();
    }
#endif
    timer.end();
    return timer.fWall;
}

static double estimate_timer_overhead() {
    double overhead = 0;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        overhead += time(1, NULL, NULL, NULL);
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

static bool write_canvas_png(SkCanvas* canvas, const SkString& filename) {
    if (filename.isEmpty()) {
        return false;
    }
    if (kUnknown_SkColorType == canvas->imageInfo().colorType()) {
        return false;
    }
    SkBitmap bmp;
    bmp.setInfo(canvas->imageInfo());
    if (!canvas->readPixels(&bmp, 0, 0)) {
        SkDebugf("Can't read canvas pixels.\n");
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
static int cpu_bench(const double overhead, Benchmark* bench, SkCanvas* canvas, double* samples) {
    // First figure out approximately how many loops of bench it takes to make overhead negligible.
    double bench_plus_overhead = 0.0;
    int round = 0;
    if (kAutoTuneLoops == FLAGS_loops) {
        while (bench_plus_overhead < overhead) {
            if (round++ == FLAGS_maxCalibrationAttempts) {
                SkDebugf("WARNING: Can't estimate loops for %s (%s vs. %s); skipping.\n",
                         bench->getUniqueName(), HUMANIZE(bench_plus_overhead), HUMANIZE(overhead));
                return kFailedLoops;
            }
            bench_plus_overhead = time(1, bench, canvas, NULL);
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
    int loops = FLAGS_loops;
    if (kAutoTuneLoops == loops) {
        const double numer = overhead / FLAGS_overheadGoal - overhead;
        const double denom = bench_plus_overhead - overhead;
        loops = (int)ceil(numer / denom);
        loops = clamp_loops(loops);
    } else {
        loops = detect_forever_loops(loops);
    }

    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, NULL) / loops;
    }
    return loops;
}

#if SK_SUPPORT_GPU
static void setup_gl(SkGLContext* gl) {
    gl->makeCurrent();
    // Make sure we're done with whatever came before.
    SK_GL(*gl, Finish());
}

static int gpu_bench(SkGLContext* gl,
                     Benchmark* bench,
                     SkCanvas* canvas,
                     double* samples) {
    // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
    int loops = FLAGS_loops;
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
            // _this_ round, not still timing last round.  We force this by looping
            // more times than any reasonable GPU will allow frames to lag.
            for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
                elapsed = time(loops, bench, canvas, gl);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);
        loops = clamp_loops(loops);

        // Might as well make sure we're not still timing our calibration.
        SK_GL(*gl, Finish());
    } else {
        loops = detect_forever_loops(loops);
    }

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
        time(loops, bench, canvas, gl);
    }

    // Now, actually do the timing!
    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, gl) / loops;
    }
    return loops;
}
#endif

static SkString to_lower(const char* str) {
    SkString lower(str);
    for (size_t i = 0; i < lower.size(); i++) {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

struct Config {
    const char* name;
    Benchmark::Backend backend;
    SkColorType color;
    SkAlphaType alpha;
    int samples;
#if SK_SUPPORT_GPU
    GrContextFactory::GLContextType ctxType;
    bool useDFText;
#else
    int bogusInt;
    bool bogusBool;
#endif
};

struct Target {
    explicit Target(const Config& c) : config(c) {}
    const Config config;
    SkAutoTDelete<SkSurface> surface;
#if SK_SUPPORT_GPU
    SkGLContext* gl;
#endif
};

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
        return sampleCnt <= ctx->getMaxSampleCount();
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

    Target* target = new Target(config);

    if (Benchmark::kRaster_Backend == config.backend) {
        target->surface.reset(SkSurface::NewRaster(info));
    }
#if SK_SUPPORT_GPU
    else if (Benchmark::kGPU_Backend == config.backend) {
        uint32_t flags = config.useDFText ? SkSurfaceProps::kUseDistanceFieldFonts_Flag : 0;
        SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
        target->surface.reset(SkSurface::NewRenderTarget(gGrFactory->get(config.ctxType),
                                                         SkSurface::kNo_Budgeted, info,
                                                         config.samples, &props));
        target->gl = gGrFactory->getGLContext(config.ctxType);
    }
#endif

    if (Benchmark::kNonRendering_Backend != config.backend && !target->surface.get()) {
        delete target;
        return NULL;
    }
    return target;
}

// Creates targets for a benchmark and a set of configs.
static void create_targets(SkTDArray<Target*>* targets, Benchmark* b,
                           const SkTDArray<Config>& configs) {
    for (int i = 0; i < configs.count(); ++i) {
        if (Target* t = is_enabled(b, configs[i])) {
            targets->push(t);
        }

    }
}

#if SK_SUPPORT_GPU
static void fill_gpu_options(ResultsWriter* log, SkGLContext* ctx) {
    const GrGLubyte* version;
    SK_GL_RET(*ctx, version, GetString(GR_GL_VERSION));
    log->configOption("GL_VERSION", (const char*)(version));

    SK_GL_RET(*ctx, version, GetString(GR_GL_RENDERER));
    log->configOption("GL_RENDERER", (const char*) version);

    SK_GL_RET(*ctx, version, GetString(GR_GL_VENDOR));
    log->configOption("GL_VENDOR", (const char*) version);

    SK_GL_RET(*ctx, version, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
    log->configOption("GL_SHADING_LANGUAGE_VERSION", (const char*) version);
}
#endif

class BenchmarkStream {
public:
    BenchmarkStream() : fBenches(BenchRegistry::Head())
                      , fGMs(skiagm::GMRegistry::Head())
                      , fCurrentRecording(0)
                      , fCurrentScale(0)
                      , fCurrentSKP(0)
                      , fCurrentUseMPD(0)
                      , fCurrentImage(0)
                      , fCurrentSubsetImage(0)
                      , fCurrentColorType(0)
                      , fDivisor(2) {
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

        fUseMPDs.push_back() = false;
        if (FLAGS_mpd) {
            fUseMPDs.push_back() = true;
        }

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
            { kN32_SkColorType, kRGB_565_SkColorType, kAlpha_8_SkColorType };
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
                            (name.c_str(), pic.get(), fClip,
                             fScales[fCurrentScale], fUseMPDs[fCurrentUseMPD++]));
                }
                fCurrentUseMPD = 0;
                fCurrentSKP++;
            }
            fCurrentSKP = 0;
            fCurrentScale++;
        }

        // Run the DecodingBenches
        while (fCurrentImage < fImages.count()) {
            while (fCurrentColorType < fColorTypes.count()) {
                const SkString& path = fImages[fCurrentImage];
                SkColorType colorType = fColorTypes[fCurrentColorType];
                fCurrentColorType++;
                // Check if the image decodes before creating the benchmark
                SkBitmap bitmap;
                if (SkImageDecoder::DecodeFile(path.c_str(), &bitmap,
                        colorType, SkImageDecoder::kDecodePixels_Mode)) {
                    return new DecodingBench(path, colorType);
                }
            }
            fCurrentColorType = 0;
            fCurrentImage++;
        }

        // Run the DecodingSubsetBenches
        while (fCurrentSubsetImage < fImages.count()) {
            while (fCurrentColorType < fColorTypes.count()) {
                const SkString& path = fImages[fCurrentSubsetImage];
                SkColorType colorType = fColorTypes[fCurrentColorType];
                fCurrentColorType++;
                // Check if the image decodes before creating the benchmark
                SkAutoTUnref<SkData> encoded(
                        SkData::NewFromFileName(path.c_str()));
                SkAutoTDelete<SkMemoryStream> stream(
                        new SkMemoryStream(encoded));
                SkAutoTDelete<SkImageDecoder>
                    decoder(SkImageDecoder::Factory(stream.get()));
                if (!decoder) {
                    SkDebugf("Cannot find decoder for %s\n", path.c_str());
                } else {
                    stream->rewind();
                    int w, h;
                    bool success;
                    if (!decoder->buildTileIndex(stream.detach(), &w, &h)
                            || w*h == 1) {
                        // This is not an error, but in this case we still
                        // do not want to run the benchmark.
                        success = false;
                    } else if (fDivisor > w || fDivisor > h) {
                        SkDebugf("Divisor %d is too big for %s %dx%d\n",
                                fDivisor, path.c_str(), w, h);
                        success = false;
                    } else {
                        const int sW  = w / fDivisor;
                        const int sH = h / fDivisor;
                        SkBitmap bitmap;
                        success = true;
                        for (int y = 0; y < h; y += sH) {
                            for (int x = 0; x < w; x += sW) {
                                SkIRect rect = SkIRect::MakeXYWH(x, y, sW, sH);
                                success &= decoder->decodeSubset(&bitmap, rect,
                                                                 colorType);
                            }
                        }
                    }
                    // Create the benchmark if successful
                    if (success) {
                        return new DecodingSubsetBench(path, colorType,
                                                       fDivisor);
                    }
                }
            }
            fCurrentColorType = 0;
            fCurrentSubsetImage++;
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
    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkIRect            fClip;
    SkTArray<SkScalar> fScales;
    SkTArray<SkString> fSKPs;
    SkTArray<bool>     fUseMPDs;
    SkTArray<SkString> fImages;
    SkTArray<SkColorType> fColorTypes;

    double fSKPBytes, fSKPOps;

    const char* fSourceType;  // What we're benching: bench, GM, SKP, ...
    const char* fBenchType;   // How we bench it: micro, recording, playback, ...
    int fCurrentRecording;
    int fCurrentScale;
    int fCurrentSKP;
    int fCurrentUseMPD;
    int fCurrentImage;
    int fCurrentSubsetImage;
    int fCurrentColorType;
    const int fDivisor;
};

int nanobench_main();
int nanobench_main() {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled;

#if SK_SUPPORT_GPU
    GrContext::Options grContextOpts;
    grContextOpts.fDrawPathToCompressedTexture = FLAGS_gpuCompressAlphaMasks;
    gGrFactory.reset(SkNEW_ARGS(GrContextFactory, (grContextOpts)));
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

    SkAutoTMalloc<double> samples(FLAGS_samples);

    if (kAutoTuneLoops != FLAGS_loops) {
        SkDebugf("Fixed number of loops; times would only be misleading so we won't print them.\n");
    } else if (FLAGS_verbose) {
        // No header.
    } else if (FLAGS_quiet) {
        SkDebugf("median\tbench\tconfig\n");
    } else {
        SkDebugf("maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tconfig\tbench\n",
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

        SkTDArray<Target*> targets;
        create_targets(&targets, bench.get(), configs);

        if (!targets.isEmpty()) {
            log->bench(bench->getUniqueName(), bench->getSize().fX, bench->getSize().fY);
            bench->preDraw();
        }
        for (int j = 0; j < targets.count(); j++) {
            SkCanvas* canvas = targets[j]->surface.get() ? targets[j]->surface->getCanvas() : NULL;
            const char* config = targets[j]->config.name;

#if SK_SUPPORT_GPU
            if (Benchmark::kGPU_Backend == targets[j]->config.backend) {
                setup_gl(targets[j]->gl);
            }
#endif

            bench->perCanvasPreDraw(canvas);

            const int loops =
#if SK_SUPPORT_GPU
                Benchmark::kGPU_Backend == targets[j]->config.backend
                ? gpu_bench(targets[j]->gl, bench.get(), canvas, samples.get())
                :
#endif
                 cpu_bench(       overhead, bench.get(), canvas, samples.get());

            bench->perCanvasPostDraw(canvas);

            if (canvas && !FLAGS_writePath.isEmpty() && FLAGS_writePath[0]) {
                SkString pngFilename = SkOSPath::Join(FLAGS_writePath[0], config);
                pngFilename = SkOSPath::Join(pngFilename.c_str(), bench->getUniqueName());
                pngFilename.append(".png");
                write_canvas_png(canvas, pngFilename);
            }

            if (kFailedLoops == loops) {
                // Can't be timed.  A warning note has already been printed.
                continue;
            }

            Stats stats(samples.get(), FLAGS_samples);
            log->config(config);
            log->configOption("name", bench->getName());
            benchStream.fillCurrentOptions(log.get());
#if SK_SUPPORT_GPU
            if (Benchmark::kGPU_Backend == targets[j]->config.backend) {
                fill_gpu_options(log.get(), targets[j]->gl);
            }
#endif
            log->metric("min_ms",    stats.min);
            if (runs++ % FLAGS_flushEvery == 0) {
                log->flush();
            }

            if (kAutoTuneLoops != FLAGS_loops) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%4dM\t%s\t%s\n"
                         , sk_tools::getBestResidentSetSizeMB()
                         , bench->getUniqueName()
                         , config);
            } else if (FLAGS_verbose) {
                for (int i = 0; i < FLAGS_samples; i++) {
                    SkDebugf("%s  ", HUMANIZE(samples[i]));
                }
                SkDebugf("%s\n", bench->getUniqueName());
            } else if (FLAGS_quiet) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\t%s\n", HUMANIZE(stats.median), bench->getUniqueName(), config);
            } else {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf("%4dM\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                        , sk_tools::getBestResidentSetSizeMB()
                        , loops
                        , HUMANIZE(stats.min)
                        , HUMANIZE(stats.median)
                        , HUMANIZE(stats.mean)
                        , HUMANIZE(stats.max)
                        , stddev_percent
                        , stats.plot.c_str()
                        , config
                        , bench->getUniqueName()
                        );
            }
#if SK_SUPPORT_GPU
            if (FLAGS_gpuStats &&
                Benchmark::kGPU_Backend == targets[j]->config.backend) {
                gGrFactory->get(targets[j]->config.ctxType)->printCacheStats();
                gGrFactory->get(targets[j]->config.ctxType)->printGpuStats();
            }
#endif
        }
        targets.deleteAll();

#if SK_SUPPORT_GPU
        if (FLAGS_abandonGpuContext) {
            gGrFactory->abandonContexts();
        }
        if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
            gGrFactory->destroyContexts();
        }
#endif
    }

    log->bench("memory_usage", 0,0);
    log->config("meta");
    log->metric("max_rss_mb", sk_tools::getMaxResidentSetSizeMB());

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return nanobench_main();
}
#endif
