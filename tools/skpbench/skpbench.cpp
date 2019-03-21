/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommonFlags.h"
#include "CommonFlagsGpu.h"
#include "DDLPromiseImageHelper.h"
#include "DDLTileHelper.h"
#include "GpuTimer.h"
#include "GrCaps.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "SkCanvas.h"
#include "SkDeferredDisplayList.h"
#include "SkGr.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPerlinNoiseShader.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "SkTaskGroup.h"
#include "ToolUtils.h"
#include "flags/CommandLineFlags.h"
#include "flags/CommonFlagsConfig.h"

#ifdef SK_XML
#include "SkDOM.h"
#include "../experimental/svg/model/SkSVGDOM.h"
#endif

#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <vector>

/**
 * This is a minimalist program whose sole purpose is to open a .skp or .svg file, benchmark it on a
 * single config, and exit. It is intended to be used through skpbench.py rather than invoked
 * directly. Limiting the entire process to a single config/skp pair helps to keep the results
 * repeatable.
 *
 * No tiling, looping, or other fanciness is used; it just draws the skp whole into a size-matched
 * render target and syncs the GPU after each draw.
 *
 * Currently, only GPU configs are supported.
 */

static DEFINE_bool(ddl, false, "record the skp into DDLs before rendering");
static DEFINE_int32(ddlNumAdditionalThreads, 0,
                    "number of DDL recording threads in addition to main one");
static DEFINE_int32(ddlTilingWidthHeight, 0, "number of tiles along one edge when in DDL mode");
static DEFINE_bool(ddlRecordTime, false, "report just the cpu time spent recording DDLs");

static DEFINE_int32(duration, 5000, "number of milliseconds to run the benchmark");
static DEFINE_int32(sampleMs, 50, "minimum duration of a sample");
static DEFINE_bool(gpuClock, false, "time on the gpu clock (gpu work only)");
static DEFINE_bool(fps, false, "use fps instead of ms");
static DEFINE_string(src, "",
                     "path to a single .skp or .svg file, or 'warmup' for a builtin warmup run");
static DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
static DEFINE_int32(verbosity, 4, "level of verbosity (0=none to 5=debug)");
static DEFINE_bool(suppressHeader, false, "don't print a header row before the results");

static const char* header =
"   accum    median       max       min   stddev  samples  sample_ms  clock  metric  config    bench";

static const char* resultFormat =
"%8.4g  %8.4g  %8.4g  %8.4g  %6.3g%%  %7li  %9i  %-5s  %-6s  %-9s %s";

static constexpr int kNumFlushesToPrimeCache = 3;

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

static void draw_skp_and_flush(SkSurface*, const SkPicture*);
static sk_sp<SkPicture> create_warmup_skp();
static sk_sp<SkPicture> create_skp_from_svg(SkStream*, const char* filename);
static bool mkdir_p(const SkString& name);
static SkString         join(const CommandLineFlags::StringArray&);
static void exitf(ExitErr, const char* format, ...);

static void ddl_sample(GrContext* context, DDLTileHelper* tiles, GpuSync* gpuSync, Sample* sample,
                       std::chrono::high_resolution_clock::time_point* startStopTime) {
    using clock = std::chrono::high_resolution_clock;

    clock::time_point start = *startStopTime;

    tiles->createDDLsInParallel();

    if (!FLAGS_ddlRecordTime) {
        tiles->drawAllTilesAndFlush(context, true);
        if (gpuSync) {
            gpuSync->syncToPreviousFrame();
        }
    }

    *startStopTime = clock::now();

    tiles->resetAllTiles();

    if (sample) {
        SkASSERT(gpuSync);
        sample->fDuration += *startStopTime - start;
        sample->fFrames++;
    }
}

static void run_ddl_benchmark(const sk_gpu_test::FenceSync* fenceSync,
                              GrContext* context, SkCanvas* finalCanvas,
                              SkPicture* inputPicture, std::vector<Sample>* samples) {
    using clock = std::chrono::high_resolution_clock;
    const Sample::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    SkIRect viewport = finalCanvas->imageInfo().bounds();

    DDLPromiseImageHelper promiseImageHelper;
    sk_sp<SkData> compressedPictureData = promiseImageHelper.deflateSKP(inputPicture);
    if (!compressedPictureData) {
        exitf(ExitErr::kUnavailable, "DDL: conversion of skp failed");
    }

    promiseImageHelper.uploadAllToGPU(context);

    DDLTileHelper tiles(finalCanvas, viewport, FLAGS_ddlTilingWidthHeight);

    tiles.createSKPPerTile(compressedPictureData.get(), promiseImageHelper);

    SkTaskGroup::Enabler enabled(FLAGS_ddlNumAdditionalThreads);

    clock::time_point startStopTime = clock::now();

    ddl_sample(context, &tiles, nullptr, nullptr, &startStopTime);
    GpuSync gpuSync(fenceSync);
    ddl_sample(context, &tiles, &gpuSync, nullptr, &startStopTime);

    clock::duration cumulativeDuration = std::chrono::milliseconds(0);

    do {
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            ddl_sample(context, &tiles, &gpuSync, &sample, &startStopTime);
        } while (sample.fDuration < sampleDuration);

        cumulativeDuration += sample.fDuration;
    } while (cumulativeDuration < benchDuration || 0 == samples->size() % 2);

    if (!FLAGS_png.isEmpty()) {
        // The user wants to see the final result
        tiles.composeAllTiles(finalCanvas);
    }
}

static void run_benchmark(const sk_gpu_test::FenceSync* fenceSync, SkSurface* surface,
                          const SkPicture* skp, std::vector<Sample>* samples) {
    using clock = std::chrono::high_resolution_clock;
    const Sample::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    draw_skp_and_flush(surface, skp); // draw 1
    GpuSync gpuSync(fenceSync);

    for (int i = 1; i < kNumFlushesToPrimeCache; ++i) {
        draw_skp_and_flush(surface, skp); // draw N
        // Waits for draw N-1 to finish (after draw N's cpu work is done).
        gpuSync.syncToPreviousFrame();
    }

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        clock::time_point sampleStart = now;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            draw_skp_and_flush(surface, skp);
            gpuSync.syncToPreviousFrame();

            now = clock::now();
            sample.fDuration = now - sampleStart;
            ++sample.fFrames;
        } while (sample.fDuration < sampleDuration);
    } while (now < endTime || 0 == samples->size() % 2);
}

static void run_gpu_time_benchmark(sk_gpu_test::GpuTimer* gpuTimer,
                                   const sk_gpu_test::FenceSync* fenceSync, SkSurface* surface,
                                   const SkPicture* skp, std::vector<Sample>* samples) {
    using sk_gpu_test::PlatformTimerQuery;
    using clock = std::chrono::steady_clock;
    const clock::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    if (!gpuTimer->disjointSupport()) {
        fprintf(stderr, "WARNING: GPU timer cannot detect disjoint operations; "
                        "results may be unreliable\n");
    }

    draw_skp_and_flush(surface, skp);
    GpuSync gpuSync(fenceSync);

    PlatformTimerQuery previousTime = 0;
    for (int i = 1; i < kNumFlushesToPrimeCache; ++i) {
        gpuTimer->queueStart();
        draw_skp_and_flush(surface, skp);
        previousTime = gpuTimer->queueStop();
        gpuSync.syncToPreviousFrame();
    }

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        const clock::time_point sampleEndTime = now + sampleDuration;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            gpuTimer->queueStart();
            draw_skp_and_flush(surface, skp);
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
    CommandLineFlags::SetUsage(
            "Use skpbench.py instead. "
            "You usually don't want to use this program directly.");
    CommandLineFlags::Parse(argc, argv);

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
    if (FLAGS_src.count() != 1) {
        exitf(ExitErr::kUsage,
              "invalid input '%s': must specify a single .skp or .svg file, or 'warmup'",
              join(FLAGS_src).c_str());
    }

    SkGraphics::Init();

    sk_sp<SkPicture> skp;
    SkString srcname;
    if (0 == strcmp(FLAGS_src[0], "warmup")) {
        skp = create_warmup_skp();
        srcname = "warmup";
    } else {
        SkString srcfile(FLAGS_src[0]);
        std::unique_ptr<SkStream> srcstream(SkStream::MakeFromFile(srcfile.c_str()));
        if (!srcstream) {
            exitf(ExitErr::kIO, "failed to open file %s", srcfile.c_str());
        }
        if (srcfile.endsWith(".svg")) {
            skp = create_skp_from_svg(srcstream.get(), srcfile.c_str());
        } else {
            skp = SkPicture::MakeFromStream(srcstream.get());
        }
        if (!skp) {
            exitf(ExitErr::kData, "failed to parse file %s", srcfile.c_str());
        }
        srcname = SkOSPath::Basename(srcfile.c_str());
    }
    int width = SkTMin(SkScalarCeilToInt(skp->cullRect().width()), 2048),
        height = SkTMin(SkScalarCeilToInt(skp->cullRect().height()), 2048);
    if (FLAGS_verbosity >= 3 &&
        (width != skp->cullRect().width() || height != skp->cullRect().height())) {
        fprintf(stderr, "%s is too large (%ix%i), cropping to %ix%i.\n",
                        srcname.c_str(), SkScalarCeilToInt(skp->cullRect().width()),
                        SkScalarCeilToInt(skp->cullRect().height()), width, height);
    }

    if (config->getSurfType() != SkCommandLineConfigGpu::SurfType::kDefault) {
        exitf(ExitErr::kUnavailable, "This tool only supports the default surface type. (%s)",
              config->getTag().c_str());
    }

    // Create a context.
    GrContextOptions ctxOptions;
    SetCtxOptionsFromCommonFlags(&ctxOptions);
    sk_gpu_test::GrContextFactory factory(ctxOptions);
    sk_gpu_test::ContextInfo ctxInfo =
        factory.getContextInfo(config->getContextType(), config->getContextOverrides());
    GrContext* ctx = ctxInfo.grContext();
    if (!ctx) {
        exitf(ExitErr::kUnavailable, "failed to create context for config %s",
                                     config->getTag().c_str());
    }
    if (ctx->maxRenderTargetSize() < SkTMax(width, height)) {
        exitf(ExitErr::kUnavailable, "render target size %ix%i not supported by platform (max: %i)",
              width, height, ctx->maxRenderTargetSize());
    }
    GrPixelConfig grPixConfig = SkColorType2GrPixelConfig(config->getColorType());
    if (kUnknown_GrPixelConfig == grPixConfig) {
        exitf(ExitErr::kUnavailable, "failed to get GrPixelConfig from SkColorType: %d",
                                     config->getColorType());
    }
    int supportedSampleCount = ctx->priv().caps()->getRenderTargetSampleCount(
            config->getSamples(), grPixConfig);
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
        if (FLAGS_ddl) {
            run_ddl_benchmark(testCtx->fenceSync(), ctx, canvas, skp.get(), &samples);
        } else {
            run_benchmark(testCtx->fenceSync(), surface.get(), skp.get(), &samples);
        }
    } else {
        if (FLAGS_ddl) {
            exitf(ExitErr::kUnavailable, "DDL: GPU-only timing not supported");
        }
        if (!testCtx->gpuTimingSupport()) {
            exitf(ExitErr::kUnavailable, "GPU does not support timing");
        }
        run_gpu_time_benchmark(testCtx->gpuTimer(), testCtx->fenceSync(), surface.get(), skp.get(),
                               &samples);
    }
    print_result(samples, config->getTag().c_str(), srcname.c_str());

    // Save a proof (if one was requested).
    if (!FLAGS_png.isEmpty()) {
        SkBitmap bmp;
        bmp.allocPixels(info);
        if (!surface->getCanvas()->readPixels(bmp, 0, 0)) {
            exitf(ExitErr::kUnavailable, "failed to read canvas pixels for png");
        }
        if (!mkdir_p(SkOSPath::Dirname(FLAGS_png[0]))) {
            exitf(ExitErr::kIO, "failed to create directory for png \"%s\"", FLAGS_png[0]);
        }
        if (!ToolUtils::EncodeImageToFile(FLAGS_png[0], bmp, SkEncodedImageFormat::kPNG, 100)) {
            exitf(ExitErr::kIO, "failed to save png to \"%s\"", FLAGS_png[0]);
        }
    }

    exit(0);
}

static void draw_skp_and_flush(SkSurface* surface, const SkPicture* skp) {
    auto canvas = surface->getCanvas();
    canvas->drawPicture(skp);
    surface->flush();
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
    ToolUtils::make_big_path(bigPath);
    recording->drawPath(bigPath, stroke);

    // Use a perlin shader to warmup the GPU.
    SkPaint perlin;
    perlin.setShader(SkPerlinNoiseShader::MakeTurbulence(0.1f, 0.1f, 1, 0, nullptr));
    recording->drawRect(bounds, perlin);

    return recorder.finishRecordingAsPicture();
}

static sk_sp<SkPicture> create_skp_from_svg(SkStream* stream, const char* filename) {
#ifdef SK_XML
    SkDOM xml;
    if (!xml.build(*stream)) {
        exitf(ExitErr::kData, "failed to parse xml in file %s", filename);
    }
    sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromDOM(xml);
    if (!svg) {
        exitf(ExitErr::kData, "failed to build svg dom from file %s", filename);
    }

    static constexpr SkRect bounds{0, 0, 1200, 1200};
    SkPictureRecorder recorder;
    SkCanvas* recording = recorder.beginRecording(bounds);

    svg->setContainerSize(SkSize::Make(recording->getBaseLayerSize()));
    svg->render(recording);

    return recorder.finishRecordingAsPicture();
#endif
    exitf(ExitErr::kData, "SK_XML is disabled; cannot open svg file %s", filename);
    return nullptr;
}

bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty()) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static SkString join(const CommandLineFlags::StringArray& stringArray) {
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
