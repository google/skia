/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextFactory.h"
#include "SkCanvas.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "picture_utils.h"
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
DEFINE_bool(fps, false, "use fps instead of ms");
DEFINE_string(skp, "", "path to a single .skp file to benchmark");
DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
DEFINE_int32(verbosity, 4, "level of verbosity (0=none to 5=debug)");
DEFINE_bool(suppressHeader, false, "don't print a header row before the results");

static const char* header =
    "  median     accum       max       min   stddev  metric  samples  sample_ms  config    bench";

static const char* resultFormat =
    "%8.4g  %8.4g  %8.4g  %8.4g  %6.3g%%  %-6s  %7li  %9i  %-9s %s";

struct Sample {
    using clock = std::chrono::high_resolution_clock;

    Sample() : fFrames(0), fDuration(0) {}
    double seconds() const { return std::chrono::duration<double>(fDuration).count(); }
    double ms() const { return std::chrono::duration<double, std::milli>(fDuration).count(); }
    double value() const { return FLAGS_fps ? fFrames / this->seconds() : this->ms() / fFrames; }
    static const char* metric() { return FLAGS_fps ? "fps" : "ms"; }

    int fFrames;
    clock::duration fDuration;
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
static SkPlatformGpuFence insert_verified_fence(const SkGpuFenceSync*);
static void wait_fence_and_delete(const SkGpuFenceSync*, SkPlatformGpuFence);
static bool mkdir_p(const SkString& name);
static SkString join(const SkCommandLineFlags::StringArray&);
static void exitf(ExitErr, const char* format, ...);

static void run_benchmark(const SkGpuFenceSync* sync, SkCanvas* canvas, const SkPicture* skp,
                          std::vector<Sample>* samples) {
    using clock = Sample::clock;
    const clock::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    // Prime the graphics pipe.
    SkPlatformGpuFence frameN_minus_2, frameN_minus_1;
    {
        draw_skp_and_flush(canvas, skp);
        SkPlatformGpuFence frame0 = insert_verified_fence(sync);

        draw_skp_and_flush(canvas, skp);
        frameN_minus_2 = insert_verified_fence(sync);

        draw_skp_and_flush(canvas, skp);
        frameN_minus_1 = insert_verified_fence(sync);

        wait_fence_and_delete(sync, frame0);
    }

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        clock::time_point sampleStart = now;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            draw_skp_and_flush(canvas, skp);

            // Sync the GPU.
            wait_fence_and_delete(sync, frameN_minus_2);
            frameN_minus_2 = frameN_minus_1;
            frameN_minus_1 = insert_verified_fence(sync);

            now = clock::now();
            sample.fDuration = now - sampleStart;
            ++sample.fFrames;
        } while (sample.fDuration < sampleDuration);
    } while (now < endTime || 0 == samples->size() % 2);

    sync->deleteFence(frameN_minus_2);
    sync->deleteFence(frameN_minus_1);
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
    const double medianValue = values[values.size() / 2];
    const double accumValue = accum.value();

    double variance = 0;
    for (double value : values) {
        const double delta = value - accumValue;
        variance += delta * delta;
    }
    variance /= values.size();
    // Technically, this is the relative standard deviation.
    const double stddev = 100/*%*/ * sqrt(variance) / accumValue;

    printf(resultFormat, medianValue, accumValue, values.back(), values.front(), stddev,
           Sample::metric(), values.size(), FLAGS_sampleMs, config, bench);
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
        exitf(ExitErr::kUsage, "invalid config %s, must specify one (and only one) GPU config",
                               join(FLAGS_config).c_str());
    }

    // Parse the skp.
    if (FLAGS_skp.count() != 1) {
        exitf(ExitErr::kUsage, "invalid skp %s, must specify (and only one) skp path name.",
                               join(FLAGS_skp).c_str());
    }
    const char* skpfile = FLAGS_skp[0];
    std::unique_ptr<SkStream> skpstream(SkStream::MakeFromFile(skpfile));
    if (!skpstream) {
        exitf(ExitErr::kIO, "failed to open skp file %s", skpfile);
    }
    sk_sp<SkPicture> skp = SkPicture::MakeFromStream(skpstream.get());
    if (!skp) {
        exitf(ExitErr::kData, "failed to parse skp file %s", skpfile);
    }
    int width = SkTMin(SkScalarCeilToInt(skp->cullRect().width()), 2048),
        height = SkTMin(SkScalarCeilToInt(skp->cullRect().height()), 2048);
    if (FLAGS_verbosity >= 3 &&
        (width != skp->cullRect().width() || height != skp->cullRect().height())) {
        fprintf(stderr, "%s is too large (%ix%i), cropping to %ix%i.\n",
                        SkOSPath::Basename(skpfile).c_str(),
                        SkScalarCeilToInt(skp->cullRect().width()),
                        SkScalarCeilToInt(skp->cullRect().height()), width, height);
    }

    // Create a context.
    sk_gpu_test::GrContextFactory factory;
    sk_gpu_test::ContextInfo ctxInfo =
        factory.getContextInfo(config->getContextType(), config->getContextOptions());
    GrContext* ctx = ctxInfo.grContext();
    if (!ctx) {
        exitf(ExitErr::kUnavailable, "failed to create context for config %s",
                                     config->getTag().c_str());
    }
    if (ctx->caps()->maxRenderTargetSize() < SkTMax(width, height)) {
        exitf(ExitErr::kUnavailable, "render target size %ix%i not supported by platform (max: %i)",
                                     width, height, ctx->caps()->maxRenderTargetSize());
    }
    if (ctx->caps()->maxSampleCount() < config->getSamples()) {
        exitf(ExitErr::kUnavailable, "sample count %i not supported by platform (max: %i)",
                                     config->getSamples(), ctx->caps()->maxSampleCount());
    }
    sk_gpu_test::TestContext* testCtx = ctxInfo.testContext();
    if (!testCtx) {
        exitf(ExitErr::kSoftware, "testContext is null");
    }
    if (!testCtx->fenceSyncSupport()) {
        exitf(ExitErr::kUnavailable, "GPU does not support fence sync");
    }

    // Create a render target.
    SkImageInfo info = SkImageInfo::Make(width, height, config->getColorType(),
                                         kPremul_SkAlphaType, sk_ref_sp(config->getColorSpace()));
    uint32_t flags = config->getUseDIText() ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface =
        SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, config->getSamples(), &props);
    if (!surface) {
        exitf(ExitErr::kUnavailable, "failed to create %ix%i render target for config %s",
                                     width, height, config->getTag().c_str());
    }

    std::vector<Sample> samples;
    if (FLAGS_sampleMs > 0) {
        // +1 because we might take one more sample in order to have an odd number.
        samples.reserve(1 + (FLAGS_duration + FLAGS_sampleMs - 1) / FLAGS_sampleMs);
    } else {
        samples.reserve(2 * FLAGS_duration);
    }

    // Run the benchmark.
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(-skp->cullRect().x(), -skp->cullRect().y());
    run_benchmark(testCtx->fenceSync(), canvas, skp.get(), &samples);
    print_result(samples, config->getTag().c_str(), SkOSPath::Basename(skpfile).c_str());

    // Save a proof (if one was requested).
    if (!FLAGS_png.isEmpty()) {
        SkBitmap bmp;
        bmp.setInfo(info);
        if (!surface->getCanvas()->readPixels(&bmp, 0, 0)) {
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

static SkPlatformGpuFence insert_verified_fence(const SkGpuFenceSync* sync) {
    SkPlatformGpuFence fence = sync->insertFence();
    if (kInvalidPlatformGpuFence == fence) {
        exitf(ExitErr::kUnavailable, "failed to insert fence");
    }
    return fence;
}

static void wait_fence_and_delete(const SkGpuFenceSync* sync, SkPlatformGpuFence fence) {
    if (kInvalidPlatformGpuFence == fence) {
        exitf(ExitErr::kSoftware, "attempted to wait on invalid fence");
    }
    if (!sync->waitFence(fence)) {
        exitf(ExitErr::kUnavailable, "failed to wait for fence");
    }
    sync->deleteFence(fence);
}

bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty()) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static SkString join(const SkCommandLineFlags::StringArray& stringArray) {
    SkString joined;
    for (int i = 0; i < FLAGS_config.count(); ++i) {
        joined.appendf(i ? " %s" : "%s", FLAGS_config[i]);
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
