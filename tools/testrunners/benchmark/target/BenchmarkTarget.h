/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"

#include <memory>
#include <tuple>

// Represents a target against which to time a benchmark. Provides an SkCanvas and all necessary
// timing methods.
//
// Based on nanobench's Target struct:
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.h#36.
class BenchmarkTarget {
public:
    static std::unique_ptr<BenchmarkTarget> FromConfig(std::string surfaceConfig,
                                                       Benchmark* benchmark);

    // Prints to standard output overall statistics collected from all benchmark targets
    // instantiated during the lifetime of the test runner.
    static void printGlobalStats();

    virtual ~BenchmarkTarget() = default;

    // Returns the backend used by this benchmark target.
    virtual Benchmark::Backend getBackend() const = 0;

    // Should be called once, immediately before any timing or drawing.
    virtual void setup() const;

    // Estimates the number of required benchmark runs to get a meaningful measurement. Returns
    // the estimated number of runs, and a boolean indicating success or failure.
    virtual std::tuple<int, bool> autoTuneLoops() const = 0;

    // Should be called once, immediately before any timing or drawing. Implementations may time
    // the benchmark for the passed in number of loops multiple times until a steady state is
    // reached.
    virtual void warmUp(int loops) const {}

    // Times the benchmark by drawing for the given number of interations. Returns the number of
    // milliseconds elapsed. It can be called multiple times between the setup() and tearDown()
    // calls.
    double time(int loops) const;

    // Should be called once after the test runner is done with the benchmark.
    void tearDown() const;

    // Produces statistics that test runner should include in the output JSON file.
    virtual void dumpStats(skia_private::TArray<SkString>* keys,
                           skia_private::TArray<double>* values) const {}

    // Prints various statistics to standard output.
    virtual void printStats() const {}

    SkCanvas* getCanvas() const;

    Benchmark* getBenchmark() const;

    // Returns the subset of Perf key/value pairs that are determined by the surface config. The
    // returned map includes keys "cpu_or_gpu" and "cpu_or_gpu_value", which are populated based
    // on the cpuName and gpuName arguments, and whether the surface config is CPU or GPU bound.
    std::map<std::string, std::string> getKeyValuePairs(std::string cpuName,
                                                        std::string gpuName) const;

    // Returns an enum indicating whether the surface is CPU or GPU bound.
    SurfaceManager::CpuOrGpu isCpuOrGpuBound() const;

protected:
    BenchmarkTarget(std::unique_ptr<SurfaceManager> surfaceManager, Benchmark* benchmark)
            : fSurfaceManager(std::move(surfaceManager)), fBenchmark(benchmark) {}

    // Called *after* the clock timer is started, before the benchmark is drawn. Most backends just
    // return the canvas passed in, but some may replace it.
    virtual SkCanvas* onBeforeDraw(SkCanvas* canvas) const { return canvas; }

    // Called *after* a benchmark is drawn, but before the clock timer is stopped.
    virtual void onAfterDraw() const {}

    double nowMs() const;

    std::unique_ptr<SurfaceManager> fSurfaceManager;
    Benchmark* fBenchmark;
};
