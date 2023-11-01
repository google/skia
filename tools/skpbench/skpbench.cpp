/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/BigPath.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/docs/SkMultiPictureDocument.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/utils/SkOSPath.h"
#include "tools/DDLPromiseImageHelper.h"
#include "tools/DDLTileHelper.h"
#include "tools/EncodeUtils.h"
#include "tools/SkSharingProc.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/flags/CommonFlagsConfig.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/FlushFinishTracker.h"
#include "tools/gpu/GpuTimer.h"
#include "tools/gpu/GrContextFactory.h"

#if defined(SK_ENABLE_SVG)
#include "modules/svg/include/SkSVGDOM.h"
#include "src/xml/SkDOM.h"
#endif

#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
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
 * Well, maybe a little fanciness, MSKP's can be loaded and played. The animation is played as many
 * times as necessary to reach the target sample duration and FPS is reported.
 *
 * Currently, only GPU configs are supported.
 */

static DEFINE_bool(ddl, false, "record the skp into DDLs before rendering");
static DEFINE_int(ddlNumRecordingThreads, 0, "number of DDL recording threads (0=num_cores)");
static DEFINE_int(ddlTilingWidthHeight, 0, "number of tiles along one edge when in DDL mode");

static DEFINE_bool(comparableDDL, false, "render in a way that is comparable to 'comparableSKP'");
static DEFINE_bool(comparableSKP, false, "report in a way that is comparable to 'comparableDDL'");

static DEFINE_int(duration, 5000, "number of milliseconds to run the benchmark");
static DEFINE_int(sampleMs, 50, "minimum duration of a sample");
static DEFINE_bool(gpuClock, false, "time on the gpu clock (gpu work only)");
static DEFINE_bool(fps, false, "use fps instead of ms");
static DEFINE_string(src, "",
                     "path to a single .skp or .svg file, or 'warmup' for a builtin warmup run");
static DEFINE_string(png, "", "if set, save a .png proof to disk at this file location");
static DEFINE_int(verbosity, 4, "level of verbosity (0=none to 5=debug)");
static DEFINE_bool(suppressHeader, false, "don't print a header row before the results");
static DEFINE_double(scale, 1, "Scale the size of the canvas and the zoom level by this factor.");
static DEFINE_bool(dumpSamples, false, "print the individual samples to stdout");

static const char header[] =
"   accum    median       max       min   stddev  samples  sample_ms  clock  metric  config    bench";

static const char resultFormat[] =
"%8.4g  %8.4g  %8.4g  %8.4g  %6.3g%%  %7zu  %9i  %-5s  %-6s  %-9s %s";

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
    GpuSync() {}
    ~GpuSync() {}

    void waitIfNeeded();

    sk_gpu_test::FlushFinishTracker* newFlushTracker(GrDirectContext* context);

private:
    enum { kMaxFrameLag = 3 };
    sk_sp<sk_gpu_test::FlushFinishTracker> fFinishTrackers[kMaxFrameLag - 1];
    int fCurrentFlushIdx = 0;
};

enum class ExitErr {
    kOk           = 0,
    kUsage        = 64,
    kData         = 65,
    kUnavailable  = 69,
    kIO           = 74,
    kSoftware     = 70
};

static void flush_with_sync(GrDirectContext*, GpuSync&);
static void draw_skp_and_flush_with_sync(GrDirectContext*, SkSurface*, const SkPicture*, GpuSync&);
static sk_sp<SkPicture> create_warmup_skp();
static sk_sp<SkPicture> create_skp_from_svg(SkStream*, const char* filename);
static bool mkdir_p(const SkString& name);
static SkString         join(const CommandLineFlags::StringArray&);
static void exitf(ExitErr, const char* format, ...);

// An interface used by both static SKPs and animated SKPs
class SkpProducer {
public:
    virtual ~SkpProducer() {}
    // Draw an SkPicture to the provided surface, flush the surface, and sync the GPU.
    // You may use the static draw_skp_and_flush_with_sync declared above.
    // returned int tells how many draw/flush/sync were done.
    virtual int drawAndFlushAndSync(GrDirectContext*, SkSurface* surface, GpuSync& gpuSync) = 0;
};

class StaticSkp : public SkpProducer {
public:
    StaticSkp(sk_sp<SkPicture> skp) : fSkp(skp) {}

    int drawAndFlushAndSync(GrDirectContext* context,
                            SkSurface* surface,
                            GpuSync& gpuSync) override {
        draw_skp_and_flush_with_sync(context, surface, fSkp.get(), gpuSync);
        return 1;
    }

private:
    sk_sp<SkPicture> fSkp;
};

// A class for playing/benchmarking a multi frame SKP file.
// the recorded frames are looped over repeatedly.
// This type of benchmark may have a much higher std dev in frame times.
class MultiFrameSkp : public SkpProducer {
public:
    MultiFrameSkp(const std::vector<SkDocumentPage>& frames) : fFrames(frames){}

    static std::unique_ptr<MultiFrameSkp> MakeFromFile(const SkString& path) {
        // Load the multi frame skp at the given filename.
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path.c_str());
        if (!stream) { return nullptr; }

        // Attempt to deserialize with an image sharing serial proc.
        auto deserialContext = std::make_unique<SkSharingDeserialContext>();
        SkDeserialProcs procs;
        procs.fImageProc = SkSharingDeserialContext::deserializeImage;
        procs.fImageCtx = deserialContext.get();

        // The outer format of multi-frame skps is the multi-picture document, which is a
        // skp file containing subpictures separated by annotations.
        int page_count = SkMultiPictureDocument::ReadPageCount(stream.get());
        if (!page_count) {
            return nullptr;
        }
        std::vector<SkDocumentPage> frames(page_count); // can't call reserve, why?
        if (!SkMultiPictureDocument::Read(stream.get(), frames.data(), page_count, &procs)) {
            return nullptr;
        }

        return std::make_unique<MultiFrameSkp>(frames);
    }

    // Draw the whole animation once.
    int drawAndFlushAndSync(GrDirectContext* context,
                            SkSurface* surface,
                            GpuSync& gpuSync) override {
        for (int i=0; i<this->count(); i++){
            draw_skp_and_flush_with_sync(context, surface, this->frame(i).get(), gpuSync);
        }
        return this->count();
    }
    // Return the requested frame.
    sk_sp<SkPicture> frame(int n) const { return fFrames[n].fPicture; }
    // Return the number of frames in the recording.
    int count() const { return fFrames.size(); }
private:
    std::vector<SkDocumentPage> fFrames;
};

static void ddl_sample(GrDirectContext* dContext, DDLTileHelper* tiles, GpuSync& gpuSync,
                       Sample* sample, SkTaskGroup* recordingTaskGroup, SkTaskGroup* gpuTaskGroup,
                       std::chrono::high_resolution_clock::time_point* startStopTime,
                       SkPicture* picture) {
    using clock = std::chrono::high_resolution_clock;

    clock::time_point start = *startStopTime;

    if (FLAGS_comparableDDL) {
        SkASSERT(!FLAGS_comparableSKP);

        // In this mode we simply alternate between creating a DDL and drawing it - all on one
        // thread. The interleaving is so that we don't starve the GPU.
        // One unfortunate side effect of this is that we can't delete the DDLs until after
        // the GPU work is flushed.
        tiles->interleaveDDLCreationAndDraw(dContext, picture);
    } else if (FLAGS_comparableSKP) {
        // In this mode simply draw the re-inflated per-tile SKPs directly to the GPU w/o going
        // through a DDL.
        tiles->drawAllTilesDirectly(dContext, picture);
    } else {
        tiles->kickOffThreadedWork(recordingTaskGroup, gpuTaskGroup, dContext, picture);
        recordingTaskGroup->wait();
    }

    if (gpuTaskGroup) {
        gpuTaskGroup->add([&]{
            flush_with_sync(dContext, gpuSync);
        });
        gpuTaskGroup->wait();
    } else {
        flush_with_sync(dContext, gpuSync);
    }

    *startStopTime = clock::now();

    if (sample) {
        sample->fDuration += *startStopTime - start;
        sample->fFrames++;
    }
}

static void run_ddl_benchmark(sk_gpu_test::TestContext* testContext,
                              GrDirectContext* dContext,
                              sk_sp<SkSurface> dstSurface,
                              SkPicture* inputPicture,
                              std::vector<Sample>* samples) {
    using clock = std::chrono::high_resolution_clock;
    const Sample::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    GrSurfaceCharacterization dstCharacterization;
    SkAssertResult(dstSurface->characterize(&dstCharacterization));

    SkIRect viewport = dstSurface->imageInfo().bounds();

    auto supportedYUVADataTypes = skgpu::ganesh::SupportedTextureFormats(*dContext);
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADataTypes);
    sk_sp<SkPicture> newSKP = promiseImageHelper.recreateSKP(dContext, inputPicture);
    if (!newSKP) {
        exitf(ExitErr::kUnavailable, "DDL: conversion of skp failed");
    }

    promiseImageHelper.uploadAllToGPU(nullptr, dContext);

    DDLTileHelper tiles(dContext, dstCharacterization, viewport,
                        FLAGS_ddlTilingWidthHeight, FLAGS_ddlTilingWidthHeight,
                        /* addRandomPaddingToDst */ false);

    tiles.createBackendTextures(nullptr, dContext);

    // In comparable modes, there is no GPU thread. The following pointers are all null.
    // Otherwise, we transfer testContext onto the GPU thread until after the bench.
    std::unique_ptr<SkExecutor> gpuThread;
    std::unique_ptr<SkTaskGroup> gpuTaskGroup;
    std::unique_ptr<SkExecutor> recordingThreadPool;
    std::unique_ptr<SkTaskGroup> recordingTaskGroup;
    if (!FLAGS_comparableDDL && !FLAGS_comparableSKP) {
        gpuThread = SkExecutor::MakeFIFOThreadPool(1, false);
        gpuTaskGroup = std::make_unique<SkTaskGroup>(*gpuThread);
        recordingThreadPool = SkExecutor::MakeFIFOThreadPool(FLAGS_ddlNumRecordingThreads, false);
        recordingTaskGroup = std::make_unique<SkTaskGroup>(*recordingThreadPool);
        testContext->makeNotCurrent();
        gpuTaskGroup->add([=]{ testContext->makeCurrent(); });
    }

    clock::time_point startStopTime = clock::now();

    GpuSync gpuSync;
    ddl_sample(dContext, &tiles, gpuSync, nullptr, recordingTaskGroup.get(),
               gpuTaskGroup.get(), &startStopTime, newSKP.get());

    clock::duration cumulativeDuration = std::chrono::milliseconds(0);

    do {
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            tiles.resetAllTiles();
            ddl_sample(dContext, &tiles, gpuSync, &sample, recordingTaskGroup.get(),
                       gpuTaskGroup.get(), &startStopTime, newSKP.get());
        } while (sample.fDuration < sampleDuration);

        cumulativeDuration += sample.fDuration;
    } while (cumulativeDuration < benchDuration || 0 == samples->size() % 2);

    // Move the context back to this thread now that we're done benching.
    if (gpuTaskGroup) {
        gpuTaskGroup->add([=]{
            testContext->makeNotCurrent();
        });
        gpuTaskGroup->wait();
        testContext->makeCurrent();
    }

    if (!FLAGS_png.isEmpty()) {
        // The user wants to see the final result
        skgpu::ganesh::DrawDDL(dstSurface, tiles.composeDDL());
        dContext->flushAndSubmit(dstSurface.get(), GrSyncCpu::kNo);
    }

    tiles.resetAllTiles();

    // Make sure the gpu has finished all its work before we exit this function and delete the
    // fence.
    dContext->flush();
    dContext->submit(GrSyncCpu::kYes);

    promiseImageHelper.deleteAllFromGPU(nullptr, dContext);

    tiles.deleteBackendTextures(nullptr, dContext);
}

static void run_benchmark(GrDirectContext* context,
                          sk_sp<SkSurface> surface,
                          SkpProducer* skpp,
                          std::vector<Sample>* samples) {
    using clock = std::chrono::high_resolution_clock;
    const Sample::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    GpuSync gpuSync;
    int i = 0;
    do {
        i += skpp->drawAndFlushAndSync(context, surface.get(), gpuSync);
    } while(i < kNumFlushesToPrimeCache);

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        clock::time_point sampleStart = now;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            sample.fFrames += skpp->drawAndFlushAndSync(context, surface.get(), gpuSync);
            now = clock::now();
            sample.fDuration = now - sampleStart;
        } while (sample.fDuration < sampleDuration);
    } while (now < endTime || 0 == samples->size() % 2);

    // Make sure the gpu has finished all its work before we exit this function and delete the
    // fence.
    context->flush(surface.get());
    context->submit(GrSyncCpu::kYes);
}

static void run_gpu_time_benchmark(sk_gpu_test::GpuTimer* gpuTimer,
                                   GrDirectContext* context,
                                   sk_sp<SkSurface> surface,
                                   const SkPicture* skp,
                                   std::vector<Sample>* samples) {
    using sk_gpu_test::PlatformTimerQuery;
    using clock = std::chrono::steady_clock;
    const clock::duration sampleDuration = std::chrono::milliseconds(FLAGS_sampleMs);
    const clock::duration benchDuration = std::chrono::milliseconds(FLAGS_duration);

    if (!gpuTimer->disjointSupport()) {
        fprintf(stderr, "WARNING: GPU timer cannot detect disjoint operations; "
                        "results may be unreliable\n");
    }

    GpuSync gpuSync;
    draw_skp_and_flush_with_sync(context, surface.get(), skp, gpuSync);

    PlatformTimerQuery previousTime = 0;
    for (int i = 1; i < kNumFlushesToPrimeCache; ++i) {
        gpuTimer->queueStart();
        draw_skp_and_flush_with_sync(context, surface.get(), skp, gpuSync);
        previousTime = gpuTimer->queueStop();
    }

    clock::time_point now = clock::now();
    const clock::time_point endTime = now + benchDuration;

    do {
        const clock::time_point sampleEndTime = now + sampleDuration;
        samples->emplace_back();
        Sample& sample = samples->back();

        do {
            gpuTimer->queueStart();
            draw_skp_and_flush_with_sync(context, surface.get(), skp, gpuSync);
            PlatformTimerQuery time = gpuTimer->queueStop();

            switch (gpuTimer->checkQueryStatus(previousTime)) {
                using QueryStatus = sk_gpu_test::GpuTimer::QueryStatus;
                case QueryStatus::kInvalid:
                    exitf(ExitErr::kUnavailable, "GPU timer failed");
                    break;
                case QueryStatus::kPending:
                    exitf(ExitErr::kUnavailable, "timer query still not ready after fence sync");
                    break;
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

    // Make sure the gpu has finished all its work before we exit this function and delete the
    // fence.
    context->flush(surface.get());
    context->submit(GrSyncCpu::kYes);
}

void print_result(const std::vector<Sample>& samples, const char* config, const char* bench)  {
    if (0 == (samples.size() % 2)) {
        exitf(ExitErr::kSoftware, "attempted to gather stats on even number of samples");
    }

    if (FLAGS_dumpSamples) {
        printf("Samples: ");
        for (const Sample& sample : samples) {
            printf("%" PRId64 " ", static_cast<int64_t>(sample.fDuration.count()));
        }
        printf("%s\n", bench);
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
    if (configs.size() != 1 || !(config = configs[0]->asConfigGpu())) {
        exitf(ExitErr::kUsage, "invalid config '%s': must specify one (and only one) GPU config",
                               join(FLAGS_config).c_str());
    }

    // Parse the skp.
    if (FLAGS_src.size() != 1) {
        exitf(ExitErr::kUsage,
              "invalid input '%s': must specify a single .skp or .svg file, or 'warmup'",
              join(FLAGS_src).c_str());
    }

    SkGraphics::Init();

    sk_sp<SkPicture> skp;
    std::unique_ptr<MultiFrameSkp> mskp; // populated if the file is multi frame.
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
        } else if (srcfile.endsWith(".mskp")) {
            mskp = MultiFrameSkp::MakeFromFile(srcfile);
            // populate skp with it's first frame, for width height determination.
            skp = mskp->frame(0);
        } else {
            skp = SkPicture::MakeFromStream(srcstream.get());
        }
        if (!skp) {
            exitf(ExitErr::kData, "failed to parse file %s", srcfile.c_str());
        }
        srcname = SkOSPath::Basename(srcfile.c_str());
    }
    int width = std::min(SkScalarCeilToInt(skp->cullRect().width()), 2048),
        height = std::min(SkScalarCeilToInt(skp->cullRect().height()), 2048);
    if (FLAGS_verbosity >= 3 &&
        (width != skp->cullRect().width() || height != skp->cullRect().height())) {
        fprintf(stderr, "%s is too large (%ix%i), cropping to %ix%i.\n",
                        srcname.c_str(), SkScalarCeilToInt(skp->cullRect().width()),
                        SkScalarCeilToInt(skp->cullRect().height()), width, height);
    }
    if (FLAGS_scale != 1) {
        width *= FLAGS_scale;
        height *= FLAGS_scale;
        if (FLAGS_verbosity >= 3) {
            fprintf(stderr, "Scale factor of %.2f: scaling to %ix%i.\n",
                    FLAGS_scale, width, height);
        }
    }

    if (config->getSurfType() != SkCommandLineConfigGpu::SurfType::kDefault) {
        exitf(ExitErr::kUnavailable, "This tool only supports the default surface type. (%s)",
              config->getTag().c_str());
    }

    // Create a context.
    GrContextOptions ctxOptions;
    CommonFlags::SetCtxOptions(&ctxOptions);
    sk_gpu_test::GrContextFactory factory(ctxOptions);
    sk_gpu_test::ContextInfo ctxInfo =
        factory.getContextInfo(config->getContextType(), config->getContextOverrides());
    auto ctx = ctxInfo.directContext();
    if (!ctx) {
        exitf(ExitErr::kUnavailable, "failed to create context for config %s",
                                     config->getTag().c_str());
    }
    if (ctx->maxRenderTargetSize() < std::max(width, height)) {
        exitf(ExitErr::kUnavailable, "render target size %ix%i not supported by platform (max: %i)",
              width, height, ctx->maxRenderTargetSize());
    }
    GrBackendFormat format = ctx->defaultBackendFormat(config->getColorType(), GrRenderable::kYes);
    if (!format.isValid()) {
        exitf(ExitErr::kUnavailable, "failed to get GrBackendFormat from SkColorType: %d",
                                     config->getColorType());
    }
    int supportedSampleCount = ctx->priv().caps()->getRenderTargetSampleCount(
            config->getSamples(), format);
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
    SkImageInfo info = SkImageInfo::Make(
            width, height, config->getColorType(), config->getAlphaType(), config->refColorSpace());
    SkSurfaceProps props(config->getSurfaceFlags(), kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info, config->getSamples(), &props);
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
    if (FLAGS_scale != 1) {
        canvas->scale(FLAGS_scale, FLAGS_scale);
    }
    if (!FLAGS_gpuClock) {
        if (FLAGS_ddl) {
            run_ddl_benchmark(testCtx, ctx, surface, skp.get(), &samples);
        } else if (!mskp) {
            auto s = std::make_unique<StaticSkp>(skp);
            run_benchmark(ctx, surface, s.get(), &samples);
        } else {
            run_benchmark(ctx, surface, mskp.get(), &samples);
        }
    } else {
        if (FLAGS_ddl) {
            exitf(ExitErr::kUnavailable, "DDL: GPU-only timing not supported");
        }
        if (!testCtx->gpuTimingSupport()) {
            exitf(ExitErr::kUnavailable, "GPU does not support timing");
        }
        run_gpu_time_benchmark(testCtx->gpuTimer(), ctx, surface, skp.get(), &samples);
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
        if (!ToolUtils::EncodeImageToPngFile(FLAGS_png[0], bmp)) {
            exitf(ExitErr::kIO, "failed to save png to \"%s\"", FLAGS_png[0]);
        }
    }

    return(0);
}

static void flush_with_sync(GrDirectContext* context, GpuSync& gpuSync) {
    gpuSync.waitIfNeeded();

    GrFlushInfo flushInfo;
    flushInfo.fFinishedProc = sk_gpu_test::FlushFinishTracker::FlushFinished;
    flushInfo.fFinishedContext = gpuSync.newFlushTracker(context);

    context->flush(flushInfo);
    context->submit();
}

static void draw_skp_and_flush_with_sync(GrDirectContext* context, SkSurface* surface,
                                         const SkPicture* skp, GpuSync& gpuSync) {
    auto canvas = surface->getCanvas();
    canvas->drawPicture(skp);

    flush_with_sync(context, gpuSync);
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
    SkPath bigPath = BenchUtils::make_big_path();
    recording->drawPath(bigPath, stroke);

    // Use a perlin shader to warmup the GPU.
    SkPaint perlin;
    perlin.setShader(SkShaders::MakeTurbulence(0.1f, 0.1f, 1, 0, nullptr));
    recording->drawRect(bounds, perlin);

    return recorder.finishRecordingAsPicture();
}

static sk_sp<SkPicture> create_skp_from_svg(SkStream* stream, const char* filename) {
#if defined(SK_ENABLE_SVG)
    sk_sp<SkSVGDOM> svg =
            SkSVGDOM::Builder().setFontManager(ToolUtils::TestFontMgr()).make(*stream);
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
    exitf(ExitErr::kData, "SK_ENABLE_SVG is disabled; cannot open svg file %s", filename);
    return nullptr;
}

bool mkdir_p(const SkString& dirname) {
    if (dirname.isEmpty() || dirname == SkString("/")) {
        return true;
    }
    return mkdir_p(SkOSPath::Dirname(dirname.c_str())) && sk_mkdir(dirname.c_str());
}

static SkString join(const CommandLineFlags::StringArray& stringArray) {
    SkString joined;
    for (int i = 0; i < stringArray.size(); ++i) {
        joined.appendf(i ? " %s" : "%s", stringArray[i]);
    }
    return joined;
}

static void exitf(ExitErr err, const char* format, ...) SK_PRINTF_LIKE(2, 3);

static void exitf(ExitErr err, const char* format, ...) {
    fprintf(stderr, ExitErr::kSoftware == err ? "INTERNAL ERROR: " : "ERROR: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ExitErr::kSoftware == err ? "; this should never happen.\n": ".\n");
    exit((int)err);
}

void GpuSync::waitIfNeeded() {
    if (fFinishTrackers[fCurrentFlushIdx]) {
        fFinishTrackers[fCurrentFlushIdx]->waitTillFinished();
    }
}

sk_gpu_test::FlushFinishTracker* GpuSync::newFlushTracker(GrDirectContext* context) {
    fFinishTrackers[fCurrentFlushIdx].reset(new sk_gpu_test::FlushFinishTracker(context));

    sk_gpu_test::FlushFinishTracker* tracker = fFinishTrackers[fCurrentFlushIdx].get();
    // We add an additional ref to the current flush tracker here. This ref is owned by the finish
    // callback on the flush call. The finish callback will unref the tracker when called.
    tracker->ref();

    fCurrentFlushIdx = (fCurrentFlushIdx + 1) % std::size(fFinishTrackers);
    return tracker;
}
