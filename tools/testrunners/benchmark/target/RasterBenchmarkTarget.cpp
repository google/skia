/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/testrunners/benchmark/target/BenchmarkTarget.h"

static DEFINE_int(maxCalibrationAttempts,
                  3,
                  "Try up to this many times to guess loops for a benchmark, or skip the "
                  "benchmark.");
static DEFINE_double(overheadGoal,
                     0.0001,
                     "Loop until timer overhead is at most this fraction of our measurments.");
static DEFINE_int(overheadLoops, 100000, "Loops to estimate timer overhead.");

// Defined in BazelBenchmarkTestRunner.cpp.
SkString humanize(double ms);

void BenchmarkTarget::printGlobalStats() {}

class RasterBenchmarkTarget : public BenchmarkTarget {
public:
    RasterBenchmarkTarget(std::unique_ptr<SurfaceManager> surfaceManager, Benchmark* benchmark)
            : BenchmarkTarget(std::move(surfaceManager), benchmark) {}

    Benchmark::Backend getBackend() const override { return Benchmark::kRaster_Backend; }

    // Based on nanobench's setup_cpu_bench():
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#466.
    std::tuple<int, bool> autoTuneLoops() const override {
        // Estimate timer overhead. Based on:
        // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#402.
        double overhead = 0;
        for (int i = 0; i < FLAGS_overheadLoops; i++) {
            double start = nowMs();
            overhead += nowMs() - start;
        }
        overhead /= FLAGS_overheadLoops;

        // First figure out approximately how many loops of bench it takes to make overhead
        // negligible.
        double bench_plus_overhead = 0.0;
        int round = 0;
        while (bench_plus_overhead < overhead) {
            if (round++ == FLAGS_maxCalibrationAttempts) {
                SkDebugf("Warning: Cannot estimate loops for %s (%s vs. %s); skipping.\n",
                         fBenchmark->getUniqueName(),
                         humanize(bench_plus_overhead).c_str(),
                         humanize(overhead).c_str());
                return std::make_tuple(0, false);
            }
            bench_plus_overhead = time(1);
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
        const double numer = overhead / FLAGS_overheadGoal - overhead;
        const double denom = bench_plus_overhead - overhead;
        int loops = (int)ceil(numer / denom);

        return std::make_tuple(loops, true);
    }
};

class NonRenderingBenchmarkTarget : public RasterBenchmarkTarget {
public:
    NonRenderingBenchmarkTarget(Benchmark* benchmark) : RasterBenchmarkTarget(nullptr, benchmark) {}

    Benchmark::Backend getBackend() const override { return Benchmark::kNonRendering_Backend; }
};

std::unique_ptr<BenchmarkTarget> BenchmarkTarget::FromConfig(std::string surfaceConfig,
                                                             Benchmark* benchmark) {
    if (surfaceConfig == "nonrendering") {
        return std::make_unique<NonRenderingBenchmarkTarget>(benchmark);
    }

    std::unique_ptr<SurfaceManager> surfaceManager = SurfaceManager::FromConfig(
            surfaceConfig, {benchmark->getSize().width(), benchmark->getSize().height()});
    if (surfaceManager == nullptr) {
        SK_ABORT("Unknown --surfaceConfig flag value: %s.", surfaceConfig.c_str());
    }

    return std::make_unique<RasterBenchmarkTarget>(std::move(surfaceManager), benchmark);
}
