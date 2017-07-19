/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GpuTimer.h"
#include "GrContextFactory.h"
#include "SkGr.h"

#include "SkCanvas.h"
#include "SkCommonFlagsPathRenderer.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPerlinNoiseShader.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "picture_utils.h"
#include "sk_tool_utils.h"
#include "flags/SkCommandLineFlags.h"
#include "flags/SkCommonFlagsConfig.h"
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <vector>

/**
 * This is a minimalist program whose sole purpose is to open an skp file, benchmark it on a single
 * config, and exit. It is intended to be used through skpbench.py rather than invoked directly.
 * Limiting the entire process to a single config/skp pair helps to keep the results repeatable.
 *
 * No tiling, looping, or other fanciness is used; it just draws the skp whole into a size-matched
 * render target and syncs the GPU after each draw.
 *
 * Currently, only GPU configs are supported.
 */

DEFINE_int32(duration, 5000, "number of milliseconds to run the benchmark");
DEFINE_int32(sampleMs, 50, "minimum duration of a sample");
DEFINE_bool(gpuClock, false, "time on the gpu clock (gpu work only)");
DEFINE_bool(fps, false, "use fps instead of ms");
DEFINE_string(skp, "", "path to a single .skp file, or 'warmup' for a builtin warmup run");
DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
DEFINE_int32(verbosity, 4, "level of verbosity (0=none to 5=debug)");
DEFINE_bool(suppressHeader, false, "don't print a header row before the results");
DEFINE_pathrenderer_flag;

static const char* header =
"   accum    median       max       min   stddev  samples  sample_ms  clock  metric  config    bench";

static const char* resultFormat =
"%8.4g  %8.4g  %8.4g  %8.4g  %6.3g%%  %7li  %9i  %-5s  %-6s  %-9s %s";

struct Sample {
    using duration = std::chrono::nanoseconds;

    Sample() : fFrames(0), fDuration(0) {}
    double seconds() const { return std::chrono::duration<double>(fDuration).count(); }
    double ms() const { return std::chrono::duration<double, std::milli>(fDuration).count(); }
    double value() const { return FLAGS_fps ? fFrames / this->seconds() : this->ms() / fFrames; }
    static const char* metric() { return FLAGS_fps ? "fps" : "ms"; }

    int        fFrames;
    duration   fDuration;
};

class GpuSync {
public:
    GpuSync(const sk_gpu_test::FenceSync* fenceSync);
    ~GpuSync();

    void syncToPreviousFrame();

private:
    void updateFence();

    const sk_gpu_test::FenceSync* const   fFenceSync;
    sk_gpu_test::PlatformFence            fFence;
};

enum class ExitErr {
    kOk           = 0,
    kUsage        = 64,
    kData         = 65,
    kUnavailable  = 69,
    kIO           = 74,
    kSoftware     = 70
};

static void draw_skp_and_flush(SkCanvas*, const SkPicture*);
static sk_sp<SkPicture> create_warmup_skp();
static bool mkdir_p(const SkString& name);
static SkString join(const SkCommandLineFlags::StringArray&);
static void exitf(ExitErr, const char* format, ...);

static void run_benchmark(const sk_gpu_test::FenceSync* fenceSync, SkCanvas* canvas,
                          const SkPicture* skp, std::vector<Sample>* samples) {
    using clock = std::chrono::high_resolution_clock;
    const Sample::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    draw_skp_and_flush(canvas, skp);
    GpuSync gpuSync(fenceSync);

    draw_skp_and_flush(canvas, skp);
    gpuSync.syncToPreviousFrame();

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        clock::time_point sampleStart = now;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            draw_skp_and_flush(canvas, skp);
            gpuSync.syncToPreviousFrame();

            now = clock::now();
            sample.fDuration = now - sampleStart;
            ++sample.fFrames;
        } while (sample.fDuration < sampleDuration);
    } while (now < endTime || 0 == samples->size() % 2);
}

static void run_gpu_time_benchmark(sk_gpu_test::GpuTimer* gpuTimer,
                                   const sk_gpu_test::FenceSync* fenceSync, SkCanvas* canvas,
                                   const SkPicture* skp, std::vector<Sample>* samples) {
    using sk_gpu_test::PlatformTimerQuery;
    using clock = std::chrono::steady_clock;
    const clock::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    if (!gpuTimer->disjointSupport()) {
        fprintf(stderr, "WARNING: GPU timer cannot detect disjoint operations; "
                        "results may be unreliable\n");
    }

    draw_skp_and_flush(canvas, skp);
    GpuSync gpuSync(fenceSync);

    gpuTimer->queueStart();
    draw_skp_and_flush(canvas, skp);
    PlatformTimerQuery previousTime = gpuTimer->queueStop();
    gpuSync.syncToPreviousFrame();

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        const clock::time_point sampleEndTime = now + sampleDuration;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            gpuTimer->queueStart();
            draw_skp_and_flush(canvas, skp);
            PlatformTimerQuery time = gpuTimer->queueStop();
            gpuSync.syncToPreviousFrame();

            switch (gpuTimer->checkQueryStatus(previousTime)) {
                using QueryStatus = sk_gpu_test::GpuTimer::QueryStatus;
                case QueryStatus::kInvalid:
                    exitf(ExitErr::kUnavailable, "GPU timer failed");
                case QueryStatus::kPending:
                    exitf(ExitErr::kUnavailable, "timer query still not ready after fence sync");
                case QueryStatus::kDisjoint:
                    if (FLAGS_verbosity >= 4) {
                        fprintf(stderr, "discarding timer query due to disjoint operations.\n");
                    }
                    break;
                case QueryStatus::kAccurate:
                    sample.fDuration += gpuTimer->getTimeElapsed(previousTime);
                    ++sample.fFrames;
                    break;
            }
            gpuTimer->deleteQuery(previousTime);
            previousTime = time;
            now = clock::now();
        } while (now < sampleEndTime || 0 == sample.fFrames);
    } while (now < endTime || 0 == samples->size() % 2);

    gpuTimer->deleteQuery(previousTime);
}

void print_result(const std::vector<Sample>& samples, const char* config, const char* bench)  {
    if (0 == (samples.size() % 2)) {
        exitf(ExitErr::kSoftware, "attempted to gather stats on even number of samples");
    }

    Sample accum = Sample();
    std::vector<double> values;
    values.reserve(samples.size());
    for (const Sample& sample : samples) {
        accum.fFrames += sample.fFrames;
        accum.fDuration += sample.fDuration;
        values.push_back(sample.value());
    }
    std::sort(values.begin(), values.end());

    const double accumValue = accum.value();
    double variance = 0;
    for (double value : values) {
        const double delta = value - accumValue;
        variance += delta * delta;
    }
    variance /= values.size();
    // Technically, this is the relative standard deviation.
    const double stddev = 100/*%*/ * sqrt(variance) / accumValue;

    printf(resultFormat, accumValue, values[values.size() / 2], values.back(), values.front(),
           stddev, values.size(), FLAGS_sampleMs, FLAGS_gpuClock ? "gpu" : "cpu", Sample::metric(),
           config, bench);
    printf("\n");
    fflush(stdout);
}

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Use skpbench.py instead. "
                                 "You usually don't want to use this program directly.");
    SkCommandLineFlags::Parse(argc, argv);

    if (!FLAGS_suppressHeader) {
        printf("%s\n", header);
    }
    if (FLAGS_duration <= 0) {
        exit(0); // This can be used to print the header and quit.
    }

    // Parse the config.
    const SkCommandLineConfigGpu* config = nullptr; // Initialize for spurious warning.
    SkCommandLineConfigArray configs;
    ParseConfigs(FLAGS_config, &configs);
    if (configs.count() != 1 || !(config = configs[0]->asConfigGpu())) {
        exitf(ExitErr::kUsage, "invalid config '%s': must specify one (and only one) GPU config",
                               join(FLAGS_config).c_str());
    }

    // Parse the skp.
    if (FLAGS_skp.count() != 1) {
        exitf(ExitErr::kUsage, "invalid skp '%s': must specify a single skp file, or 'warmup'",
                               join(FLAGS_skp).c_str());
    }
    sk_sp<SkPicture> skp;
    SkString skpname;
    if (0 == strcmp(FLAGS_skp[0], "warmup")) {
        skp = create_warmup_skp();
        skpname = "warmup";
    } else {
        const char* skpfile = FLAGS_skp[0];
        std::unique_ptr<SkStream> skpstream(SkStream::MakeFromFile(skpfile));
        if (!skpstream) {
            exitf(ExitErr::kIO, "failed to open skp file %s", skpfile);
        }
        skp = SkPicture::MakeFromStream(skpstream.get());
        if (!skp) {
            exitf(ExitErr::kData, "failed to parse skp file %s", skpfile);
        }
        skpname = SkOSPath::Basename(skpfile);
    }
    int width = SkTMin(SkScalarCeilToInt(skp->cullRect().width()), 2048),
        height = SkTMin(SkScalarCeilToInt(skp->cullRect().height()), 2048);
    if (FLAGS_verbosity >= 3 &&
        (width != skp->cullRect().width() || height != skp->cullRect().height())) {
        fprintf(stderr, "%s is too large (%ix%i), cropping to %ix%i.\n",
                        skpname.c_str(), SkScalarCeilToInt(skp->cullRect().width()),
                        SkScalarCeilToInt(skp->cullRect().height()), width, height);
    }

    // Create a context.
    GrContextOptions ctxOptions;
    ctxOptions.fGpuPathRenderers = CollectGpuPathRenderersFromFlags();
    sk_gpu_test::GrContextFactory factory(ctxOptions);
    sk_gpu_test::ContextInfo ctxInfo =
        factory.getContextInfo(config->getContextType(), config->getContextOverrides());
    GrContext* ctx = ctxInfo.grContext();
    if (!ctx) {
        exitf(ExitErr::kUnavailable, "failed to create context for config %s",
                                     config->getTag().c_str());
    }
    if (ctx->caps()->maxRenderTargetSize() < SkTMax(width, height)) {
        exitf(ExitErr::kUnavailable, "render target size %ix%i not supported by platform (max: %i)",
                                     width, height, ctx->caps()->maxRenderTargetSize());
    }
    GrPixelConfig grPixConfig = SkImageInfo2GrPixelConfig(config->getColorType(),
                                                          config->getColorSpace(),
                                                          *ctx->caps());
    int supportedSampleCount = ctx->caps()->getSampleCount(config->getSamples(), grPixConfig);
    if (supportedSampleCount != config->getSamples()) {
        exitf(ExitErr::kUnavailable, "sample count %i not supported by platform",
                                     config->getSamples());
    }
    sk_gpu_test::TestContext* testCtx = ctxInfo.testContext();
    if (!testCtx) {
        exitf(ExitErr::kSoftware, "testContext is null");
    }
    if (!testCtx->fenceSyncSupport()) {
        exitf(ExitErr::kUnavailable, "GPU does not support fence sync");
    }

    // Create a render target.
    SkImageInfo info =
            SkImageInfo::Make(width, height, config->getColorType(), config->getAlphaType(),
                              sk_ref_sp(config->getColorSpace()));
    uint32_t flags = config->getUseDIText() ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface =
        SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, config->getSamples(), &props);
    if (!surface) {
        exitf(ExitErr::kUnavailable, "failed to create %ix%i render target for config %s",
                                     width, height, config->getTag().c_str());
    }

    // Run the benchmark.
    std::vector<Sample> samples;
    if (FLAGS_sampleMs > 0) {
        // +1 because we might take one more sample in order to have an odd number.
        samples.reserve(1 + (FLAGS_duration + FLAGS_sampleMs - 1) / FLAGS_sampleMs);
    } else {
        samples.reserve(2 * FLAGS_duration);
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(-skp->cullRect().x(), -skp->cullRect().y());
    if (!FLAGS_gpuClock) {
        run_benchmark(testCtx->fenceSync(), canvas, skp.get(), &samples);
    } else {
        if (!testCtx->gpuTimingSupport()) {
            exitf(ExitErr::kUnavailable, "GPU does not support timing");
        }
        run_gpu_time_benchmark(testCtx->gpuTimer(), testCtx->fenceSync(), canvas, skp.get(),
                               &samples);
    }
    print_result(samples, config->getTag().c_str(), skpname.c_str());

    // Save a proof (if one was requested).
    if (!FLAGS_png.isEmpty()) {
        SkBitmap bmp;
        bmp.allocPixels(info);
        if (!surface->getCanvas()->readPixels(bmp, 0, 0)) {
            exitf(ExitErr::kUnavailable, "failed to read canvas pixels for png");
        }
        const SkString &dirname = SkOSPath::Dirname(FLAGS_png[0]),
                       &basename = SkOSPath::Basename(FLAGS_png[0]);
        if (!mkdir_p(dirname)) {
            exitf(ExitErr::kIO, "failed to create directory \"%s\" for png", dirname.c_str());
        }
        if (!sk_tools::write_bitmap_to_disk(bmp, dirname, nullptr, basename)) {
            exitf(ExitErr::kIO, "failed to save png to \"%s\"", FLAGS_png[0]);
        }
    }

    exit(0);
}

static void draw_skp_and_flush(SkCanvas* canvas, const SkPicture* skp) {
    canvas->drawPicture(skp);
    canvas->flush();
}

static sk_sp<SkPicture> create_warmup_skp() {
    static constexpr SkRect bounds{0, 0, 500, 500};
    SkPictureRecorder recorder;
    SkCanvas* recording = recorder.beginRecording(bounds);

    recording->clear(SK_ColorWHITE);

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(2);

    // Use a big path to (theoretically) warmup the CPU.
    SkPath bigPath;
    sk_tool_utils::make_big_path(bigPath);
    recording->drawPath(bigPath, stroke);

    // Use a perlin shader to warmup the GPU.
    SkPaint perlin;
    perlin.setShader(SkPerlinNoiseShader::MakeTurbulence(0.1f, 0.1f, 1, 0, nullptr));
    recording->drawRect(bounds, perlin);

    return recorder.finishRecordingAsPicture();
}

bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty()) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static SkString join(const SkCommandLineFlags::StringArray& stringArray) {
    SkString joined;
    for (int i = 0; i < stringArray.count(); ++i) {
        joined.appendf(i ? " %s" : "%s", stringArray[i]);
    }
    return joined;
}

static void exitf(ExitErr err, const char* format, ...) {
    fprintf(stderr, ExitErr::kSoftware == err ? "INTERNAL ERROR: " : "ERROR: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ExitErr::kSoftware == err ? "; this should never happen.\n": ".\n");
    exit((int)err);
}

GpuSync::GpuSync(const sk_gpu_test::FenceSync* fenceSync)
    : fFenceSync(fenceSync) {
    this->updateFence();
}

GpuSync::~GpuSync() {
    fFenceSync->deleteFence(fFence);
}

void GpuSync::syncToPreviousFrame() {
    if (sk_gpu_test::kInvalidFence == fFence) {
        exitf(ExitErr::kSoftware, "attempted to sync with invalid fence");
    }
    if (!fFenceSync->waitFence(fFence)) {
        exitf(ExitErr::kUnavailable, "failed to wait for fence");
    }
    fFenceSync->deleteFence(fFence);
    this->updateFence();
}

void GpuSync::updateFence() {
    fFence = fFenceSync->insertFence();
    if (sk_gpu_test::kInvalidFence == fFence) {
        exitf(ExitErr::kUnavailable, "failed to insert fence");
    }
}
