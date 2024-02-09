/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/TestContext.h"
#include "tools/testrunners/benchmark/target/BenchmarkTarget.h"
#include "tools/testrunners/common/TestRunner.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"

// Based on flags found here:
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/tools/flags/CommonFlagsGpu.cpp
//
// These are the only flags used in CI tasks at the time of writing (2023-09-29), but we can
// always backport more flags from //tools/flags/CommonFlagsGpu.cpp as needed.
static DEFINE_double(
        gpuMs,
        5,
        "While auto-tuning the number of benchmark runs per sample, increase the number of runs "
        "until a single sample takes this many milliseconds. Do this for each benchmark.");

static DEFINE_bool(gpuStats, false, "Print GPU stats after each GPU benchmark.");

static DEFINE_bool(gpuStatsDump,
                   false,
                   "Dump GPU stats after each benchmark into the "
                   "results.json output file, which can be ingested by Perf.");

static DEFINE_bool(dmsaaStatsDump,
                   false,
                   "Dump DMSAA stats after each benchmark into the "
                   "results.json output file, which can be ingested by Perf.");

// Estimated maximum number of frames the GPU allows to lag, if unknown.
//
// Based on:
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#131
//
// TODO(lovisolo): This value is overridden by //bench/nanobench.cpp based on the --loops flag, see
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1361.
static constexpr int ESTIMATED_GPU_FRAME_LAG = 5;

GrRecordingContextPriv::DMSAAStats combinedDMSAAStats;

void BenchmarkTarget::printGlobalStats() {
    if (FLAGS_dmsaaStatsDump) {
        TestRunner::Log("<<Total Combined DMSAA Stats>>");
        combinedDMSAAStats.dump();
    }
}

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#240.
class GaneshBenchmarkTarget : public BenchmarkTarget {
public:
    GaneshBenchmarkTarget(std::unique_ptr<SurfaceManager> surfaceManager, Benchmark* benchmark)
            : BenchmarkTarget(std::move(surfaceManager), benchmark) {}

    ~GaneshBenchmarkTarget() override {
        // For Vulkan we need to release all our refs to the GrContext before destroy the vulkan
        // context which happens at the end of this destructor. Thus we need to release the surface
        // here which holds a ref to the GrContext.
        fSurfaceManager->getSurface().reset();
    }

    Benchmark::Backend getBackend() const override { return Benchmark::kGPU_Backend; }

    // TODO(lovisolo): Do we still need this?
    void setup() const override {
        fSurfaceManager->getGaneshContextInfo()->testContext()->makeCurrent();
        // Make sure we're done with whatever came before.
        fSurfaceManager->getGaneshContextInfo()->testContext()->finish();

        BenchmarkTarget::setup();
    }

    // TODO(lovisolo): Do we still need this?
    void onAfterDraw() const override {
        if (fSurfaceManager->getGaneshContextInfo()->testContext()) {
            fSurfaceManager->getGaneshContextInfo()->testContext()->flushAndWaitOnSync(
                    fSurfaceManager->getGaneshContextInfo()->directContext());
        }
    }

    // Based on nanobench's setup_gpu_bench():
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#510.
    std::tuple<int, bool> autoTuneLoops() const override {
        int maxFrameLag = computeMaxFrameLag();

        // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
        int loops = 1;
        double elapsed = 0;
        do {
            if (1 << 30 == loops) {
                // We're about to wrap. Something's wrong with the bench.
                loops = 0;
                break;
            }
            loops *= 2;
            // If the GPU lets frames lag at all, we need to make sure we're timing
            // _this_ round, not still timing last round.
            for (int i = 0; i < maxFrameLag; i++) {
                elapsed = time(loops);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little. Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);

        // Make sure we're not still timing our calibration.
        fSurfaceManager->getGaneshContextInfo()->testContext()->finish();

        return std::make_tuple(loops, true);
    }

    // Based on
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#539.
    void warmUp(int loops) const override {
        // Pretty much the same deal as the calibration: do some warmup to make
        // sure we're timing steady-state pipelined frames.
        int maxFrameLag = computeMaxFrameLag();
        for (int i = 0; i < maxFrameLag; i++) {
            time(loops);
        }
    }

    void dumpStats(skia_private::TArray<SkString>* keys,
                   skia_private::TArray<double>* values) const override {
        if (FLAGS_gpuStatsDump) {
            // TODO cache stats
            fBenchmark->getGpuStats(getCanvas(), keys, values);
        }
        if (FLAGS_dmsaaStatsDump && fBenchmark->getDMSAAStats(getCanvas()->recordingContext())) {
            const auto& dmsaaStats = getCanvas()->recordingContext()->priv().dmsaaStats();
            dmsaaStats.dumpKeyValuePairs(keys, values);
            dmsaaStats.dump();
            combinedDMSAAStats.merge(dmsaaStats);
        }
    }

    void printStats() const override {
        if (FLAGS_gpuStats) {
            auto context = fSurfaceManager->getGaneshContextInfo()->directContext();

            context->priv().printCacheStats();
            context->priv().printGpuStats();
            context->priv().printContextStats();
        }
    }

private:
    // Based on nanobench's GPUTarget::needsFrameTiming():
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#264.
    int computeMaxFrameLag() const {
        int maxFrameLag;
        if (!fSurfaceManager->getGaneshContextInfo()->testContext()->getMaxGpuFrameLag(
                    &maxFrameLag)) {
            // Frame lag is unknown.
            maxFrameLag = ESTIMATED_GPU_FRAME_LAG;
        }
        return maxFrameLag;
    }
};

std::unique_ptr<BenchmarkTarget> BenchmarkTarget::FromConfig(std::string surfaceConfig,
                                                             Benchmark* benchmark) {
    SurfaceOptions surfaceOptions = {
            .width = benchmark->getSize().width(),
            .height = benchmark->getSize().height(),
            .modifyGrContextOptions = [&](GrContextOptions* grContextOptions) {
                benchmark->modifyGrContextOptions(grContextOptions);
            }};
    std::unique_ptr<SurfaceManager> surfaceManager =
            SurfaceManager::FromConfig(surfaceConfig, surfaceOptions);
    if (surfaceManager == nullptr) {
        SK_ABORT("Unknown --surfaceConfig flag value: %s.", surfaceConfig.c_str());
    }

    if (surfaceManager->getGaneshContextInfo()->testContext()->fenceSyncSupport()) {
        TestRunner::Log(
                "WARNING: GL context for config \"%s\" does not support fence sync. "
                "Timings might not be accurate.",
                surfaceConfig.c_str());
    }

    return std::make_unique<GaneshBenchmarkTarget>(std::move(surfaceManager), benchmark);
}
