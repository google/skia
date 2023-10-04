/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/ResultsWriter.h"
#include "bench/benchmark_target/BenchmarkTarget.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTime.h"
#include "src/utils/SkOSPath.h"
#include "tools/AutoreleasePool.h"
#include "tools/ProcStats.h"
#include "tools/Stats.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/timer/Timer.h"

#include <cinttypes>

// When running under Bazel and overriding the output directory, you might encounter errors such
// as "No such file or directory" and "Read-only file system". The former can happen when running
// on RBE because the passed in output dir might not exist on the remote worker, whereas the latter
// can happen when running locally in sandboxed mode, which is the default strategy when running
// outside of RBE. One possible workaround is to run the test as a local subprocess, which can be
// done by passing flag --strategy=TestRunner=local to Bazel.
//
// Reference: https://bazel.build/docs/user-manual#execution-strategy.
static DEFINE_string(outputDir,
                     "",
                     "Directory where to write any output JSON and PNG files. "
                     "Optional when running under Bazel "
                     "(e.g. \"bazel test //path/to:test\") as it defaults to "
                     "$TEST_UNDECLARED_OUTPUTS_DIR.");

static DEFINE_string(surfaceConfig,
                     "",
                     "Name of the Surface configuration to use (e.g. \"8888\"). This determines "
                     "how we construct the SkSurface from which we get the SkCanvas that "
                     "benchmarks will draw on. See file "
                     "//tools/testrunners/surface_manager/SurfaceManager.h for details.");

static DEFINE_bool(
        writePNGs,
        false,
        "Whether or not to write to the output directory any bitmaps produced by benchmarks.");

static DEFINE_string(key,
                     "",
                     "Space-separated key/value pairs identifying the benchmark. These will be "
                     "included in the results.json output file, which can be ingested by Perf.");

// Mutually exclusive with --autoTuneLoops.
static DEFINE_int(loops, 0, "The number of benchmark runs that constitutes a single sample.");

// Mutually exclusive with --loops.
static DEFINE_bool(autoTuneLoops,
                   false,
                   "Auto-tune (automatically determine) the number of benchmark runs that "
                   "constitutes a single sample. Timings are only reported when auto-tuning.");

static DEFINE_int(
        autoTuneLoopsMax,
        1000000,
        "Maximum number of benchmark runs per single sample when auto-tuning. Ignored unless flag "
        "--autoTuneLoops is set.");

// Mutually exclusive with --ms.
static DEFINE_int(samples, 10, "Number of samples to collect for each benchmark.");

// Mutually exclusive with --samples.
static DEFINE_int(ms, 0, "For each benchmark, collect samples for this many milliseconds.");

static DEFINE_int(flushEvery,
                  10,
                  "Flush the results.json output file every n-th run. This file "
                  "can be ingested by Perf.");

static DEFINE_bool(csv, false, "Print status in CSV format.");

static DEFINE_bool2(quiet, q, false, "if true, do not print status updates.");

static DEFINE_bool2(verbose, v, false, "Enable verbose output from the test runner.");

static void validate_flags(bool isBazelTest) {
    if (!isBazelTest && FLAGS_outputDir.isEmpty()) {
        SK_ABORT("Flag --outputDir cannot be empty.");
    }
    if (FLAGS_outputDir.size() > 1) {
        SK_ABORT("Flag --outputDir takes one single value, got %d.", FLAGS_outputDir.size());
    }
    if (FLAGS_surfaceConfig.isEmpty()) {
        SK_ABORT("Flag --surfaceConfig cannot be empty.");
    }
    if (FLAGS_surfaceConfig.size() > 1) {
        SK_ABORT("Flag --surfaceConfig takes one single value, got %d.",
                 FLAGS_surfaceConfig.size());
    }
    if (FLAGS_key.size() % 2 == 1) {
        SK_ABORT("Flag --key takes an even number of arguments, got %d.\n", FLAGS_key.size());
    }

    int waysToDetermineNumLoops = 0;
    if (FLAGS_loops != 0) {
        waysToDetermineNumLoops++;
    }
    if (FLAGS_autoTuneLoops) {
        waysToDetermineNumLoops++;
    }
    if (waysToDetermineNumLoops != 1) {
        SK_ABORT("Exactly one of the following flags must be set: --loops, --autoTuneLoops.\n");
    }

    if (!FLAGS_autoTuneLoops && FLAGS_loops <= 0) {
        SK_ABORT("Flag --loops must be greater or equal than 1, got %d.\n", FLAGS_loops);
    }
    if (FLAGS_autoTuneLoopsMax <= 0) {
        SK_ABORT("Flag --autoTuneLoopsMax must be greater or equal than 1, got %d.\n",
                 FLAGS_autoTuneLoopsMax);
    }

    int waysToDetermineNumSamples = 0;
    if (FLAGS_samples != 0) {
        waysToDetermineNumSamples++;
    }
    if (FLAGS_ms != 0) {
        waysToDetermineNumSamples++;
    }
    if (waysToDetermineNumSamples != 1) {
        SK_ABORT("Exactly one of the following flags must be set: --samples, --ms.\n");
    }

    if (FLAGS_ms == 0 && FLAGS_samples <= 0) {
        SK_ABORT("Flag --samples must be greater or equal than 1, got %d.\n", FLAGS_samples);
    }
    if (FLAGS_samples == 0 && FLAGS_ms <= 0) {
        SK_ABORT("Flag --ms must be greater or equal than 1, got %d.\n", FLAGS_ms);
    }
}

// Manages an autorelease pool for Metal. On other platforms, pool.drain() is a no-op.
AutoreleasePool pool;

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1503.
static void warm_up_test_runner_once(BenchmarkTarget* target, int loops) {
    static bool warm = false;
    if (warm) {
        return;
    }
    if (FLAGS_ms < 1000) {
        // Run the first bench for 1000ms to warm up the test runner if FLAGS_ms < 1000.
        // Otherwise, the first few benches' measurements will be inaccurate.
        auto stop = now_ms() + 1000;
        do {
            target->time(loops);
            pool.drain();
        } while (now_ms() < stop);
    }
    warm = true;
}

// Collects samples for the given benchmark. Returns a boolean indicating success or failure, the
// number of benchmark runs used for each sample, the samples and any statistics produced by the
// benchmark and/or target.
//
// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1489.
static int sample_benchmark(BenchmarkTarget* target,
                            int* loops,
                            skia_private::TArray<double>* samples,
                            skia_private::TArray<SkString>* statKeys,
                            skia_private::TArray<double>* statValues) {
    target->setup();

    if (FLAGS_autoTuneLoops) {
        auto [autoTunedLoops, ok] = target->autoTuneLoops();
        if (!ok) {
            // Can't be timed. A warning has already been printed.
            target->tearDown();
            return false;
        }
        *loops = autoTunedLoops;
        if (*loops > FLAGS_autoTuneLoopsMax) {
            SkDebugf(
                    "Warning: Clamping loops from %d to %d (per the --autoTuneLoopsMax flag) for "
                    "benchmark \"%s\".\n",
                    *loops,
                    FLAGS_autoTuneLoopsMax,
                    target->getBenchmark()->getUniqueName());
            *loops = FLAGS_autoTuneLoopsMax;
        }
    } else {
        *loops = FLAGS_loops;
    }

    // Warm up the test runner to increase the chances of getting consistent measurements. Only
    // done once for the entire lifecycle of the test runner.
    warm_up_test_runner_once(target, *loops);

    // Each individual BenchmarkTarget must also be warmed up.
    target->warmUp(*loops);

    if (FLAGS_ms) {
        // Collect as many samples as possible for a specific duration of time.
        auto stop = now_ms() + FLAGS_ms;
        do {
            samples->push_back(target->time(*loops) / *loops);
            pool.drain();
        } while (now_ms() < stop);
    } else {
        // Collect an exact number of samples.
        samples->reset(FLAGS_samples);
        for (int s = 0; s < FLAGS_samples; s++) {
            (*samples)[s] = target->time(*loops) / *loops;
            pool.drain();
        }
    }

    // Scale each sample to the benchmark's own units, time/unit.
    for (double& sample : *samples) {
        sample *= (1.0 / target->getBenchmark()->getUnits());
    }

    target->dumpStats(statKeys, statValues);
    target->tearDown();

    return true;
}

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#432.
static void maybe_write_png(BenchmarkTarget* target, std::string outputDir) {
    if (target->getBackend() == Benchmark::kNonRendering_Backend) {
        return;
    }

    SkString filename = SkStringPrintf("%s.png", target->getBenchmark()->getUniqueName());
    filename = SkOSPath::Join(outputDir.c_str(), filename.c_str());

    if (!target->getCanvas() ||
        target->getCanvas()->imageInfo().colorType() == kUnknown_SkColorType) {
        return;
    }

    SkBitmap bmp;
    bmp.allocPixels(target->getCanvas()->imageInfo());
    if (!target->getCanvas()->readPixels(bmp, 0, 0)) {
        SkDebugf("Warning: Could not read canvas pixels for benchmark \"%s\".\n",
                 target->getBenchmark()->getUniqueName());
        return;
    }

    SkFILEWStream stream(filename.c_str());
    if (!stream.isValid()) {
        SkDebugf("Warning: Could not write file \"%s\".\n", filename.c_str());
        return;
    }
    if (!SkPngEncoder::Encode(&stream, bmp.pixmap(), {})) {
        SkDebugf("Warning: Could not encode pixels from benchmark \"%s\" as PNG.\n",
                 target->getBenchmark()->getUniqueName());
        return;
    }

    if (FLAGS_verbose) {
        SkDebugf("PNG for benchmark \"%s\" written to: %s",
                 target->getBenchmark()->getUniqueName(),
                 filename.c_str());
    }
}

// Non-static because it is used from RasterBenchmarkTarget.cpp.
SkString humanize(double ms) {
    if (FLAGS_verbose) return SkStringPrintf("%" PRIu64, (uint64_t)(ms * 1e6));
    return HumanizeMs(ms);
}

#define HUMANIZE(ms) humanize(ms).c_str()

static SkString to_string(int n) {
    SkString str;
    str.appendS32(n);
    return str;
}

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1593.
static void print_benchmark_stats(Stats* stats,
                                  skia_private::TArray<double>* samples,
                                  BenchmarkTarget* target,
                                  std::string surfaceConfig,
                                  int loops) {
    if (!FLAGS_autoTuneLoops) {
        SkDebugf("%4d/%-4dMB\t%s\t%s ",
                 sk_tools::getCurrResidentSetSizeMB(),
                 sk_tools::getMaxResidentSetSizeMB(),
                 target->getBenchmark()->getUniqueName(),
                 surfaceConfig.c_str());
        SkDebugf("\n");
    } else if (FLAGS_quiet) {
        const char* mark = " ";
        const double stddev_percent = sk_ieee_double_divide(100 * sqrt(stats->var), stats->mean);
        if (stddev_percent > 5) mark = "?";
        if (stddev_percent > 10) mark = "!";
        SkDebugf("%10.2f %s\t%s\t%s\n",
                 stats->median * 1e3,
                 mark,
                 target->getBenchmark()->getUniqueName(),
                 surfaceConfig.c_str());
    } else if (FLAGS_csv) {
        const double stddev_percent = sk_ieee_double_divide(100 * sqrt(stats->var), stats->mean);
        SkDebugf("%g,%g,%g,%g,%g,%s,%s\n",
                 stats->min,
                 stats->median,
                 stats->mean,
                 stats->max,
                 stddev_percent,
                 surfaceConfig.c_str(),
                 target->getBenchmark()->getUniqueName());
    } else {
        const double stddev_percent = sk_ieee_double_divide(100 * sqrt(stats->var), stats->mean);
        SkDebugf("%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n",
                 sk_tools::getCurrResidentSetSizeMB(),
                 sk_tools::getMaxResidentSetSizeMB(),
                 loops,
                 HUMANIZE(stats->min),
                 HUMANIZE(stats->median),
                 HUMANIZE(stats->mean),
                 HUMANIZE(stats->max),
                 stddev_percent,
                 FLAGS_ms ? to_string(samples->size()).c_str() : stats->plot.c_str(),
                 surfaceConfig.c_str(),
                 target->getBenchmark()->getUniqueName());
    }

    target->printStats();

    if (FLAGS_verbose) {
        SkDebugf("Samples:  ");
        for (int j = 0; j < samples->size(); j++) {
            SkDebugf("%s  ", HUMANIZE((*samples)[j]));
        }
        SkDebugf("%s\n", target->getBenchmark()->getUniqueName());
    }
}

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1337.
int main(int argc, char** argv) {
#ifdef SK_BUILD_FOR_ANDROID
    extern bool gSkDebugToStdOut;  // If true, sends SkDebugf to stdout as well.
    gSkDebugToStdOut = true;
#endif

    // Print command-line for debugging purposes.
    if (argc < 2) {
        SkDebugf("Benchmark runner invoked with no arguments.\n");
    } else {
        SkDebugf("Benchmark runner invoked with arguments:");
        for (int i = 1; i < argc; i++) {
            SkDebugf(" %s", argv[i]);
        }
        SkDebugf("\n");
    }

    // When running under Bazel (e.g. "bazel test //path/to:test"), we'll store output files in
    // $TEST_UNDECLARED_OUTPUTS_DIR unless overridden via the --outputDir flag.
    //
    // See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    std::string testUndeclaredOutputsDir;
    if (char* envVar = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR")) {
        testUndeclaredOutputsDir = envVar;
    }
    bool isBazelTest = !testUndeclaredOutputsDir.empty();

    // Parse and validate flags.
    CommandLineFlags::Parse(argc, argv);
    validate_flags(isBazelTest);

    // TODO(lovisolo): Define an enum for surface configs and turn this flag into an enum value.
    std::string surfaceConfig = FLAGS_surfaceConfig[0];

    // Directory where we will write the output JSON file and any PNGs produced by the benchmarks.
    std::string outputDir =
            FLAGS_outputDir.isEmpty() ? testUndeclaredOutputsDir : FLAGS_outputDir[0];

    // Output JSON file.
    //
    // TODO(lovisolo): Define a constant with the file name, use it here and in flag descriptions.
    SkString jsonPath = SkOSPath::Join(outputDir.c_str(), "results.json");
    SkFILEWStream jsonFile(jsonPath.c_str());
    NanoJSONResultsWriter jsonWriter(&jsonFile, SkJSONWriter::Mode::kPretty);

    jsonWriter.beginObject();  // Root object.

    // Keys.
    if (FLAGS_key.size()) {
        jsonWriter.beginObject("key");  // "key" dictionary.
        for (int i = 1; i < FLAGS_key.size(); i += 2) {
            jsonWriter.appendCString(FLAGS_key[i - 1], FLAGS_key[i]);
        }
        jsonWriter.endObject();  // "key" dictionary.
    }

    jsonWriter.beginObject("results");  // "results" dictionary.

    int runs = 0;
    for (auto benchmarkFactory : BenchRegistry::Range()) {
        std::unique_ptr<Benchmark> benchmark(benchmarkFactory(nullptr));

        jsonWriter.beginBench(benchmark->getUniqueName(),
                              benchmark->getSize().width(),
                              benchmark->getSize().height());

        benchmark->delayedSetup();

        std::unique_ptr<BenchmarkTarget> target =
                BenchmarkTarget::FromConfig(surfaceConfig, benchmark.get());

        if (benchmark->isSuitableFor(target->getBackend())) {
            // Run benchmark and collect samples.
            int loops;
            skia_private::TArray<double> samples;
            skia_private::TArray<SkString> statKeys;
            skia_private::TArray<double> statValues;
            if (!sample_benchmark(target.get(), &loops, &samples, &statKeys, &statValues)) {
                // Sampling failed. A warning has alredy been printed. Move on to the next
                // benchmark.
                continue;
            }

            if (FLAGS_writePNGs) {
                // Save the bitmap produced by the benchmark to disk, if applicable. Not all
                // benchmarks produce bitmaps, e.g. those that use the "nonrendering" config.
                maybe_write_png(target.get(), outputDir);
            }

            // Building stats.plot often shows up in profiles, so skip building it when we're not
            // going to print it anyway.
            const bool want_plot = !FLAGS_quiet && !FLAGS_ms;
            Stats stats(samples, want_plot);

            jsonWriter.beginObject(surfaceConfig.c_str());  // Config object.

            // Options.
            //
            // TODO(lovisolo): Determine these dynamically when we add support for other types of
            //                 benchmarks (e.g. GMBench, SKPBench, etc.).
            jsonWriter.beginObject("options");  // Options object.
            jsonWriter.appendCString("name", benchmark->getName());
            jsonWriter.appendCString("source_type", "bench");
            jsonWriter.appendCString("bench_type", "micro");
            jsonWriter.endObject();  // Options object.

            // Metrics.
            jsonWriter.appendMetric("min_ms", stats.min);
            jsonWriter.appendMetric("min_ratio", sk_ieee_double_divide(stats.median, stats.min));

            // Samples.
            jsonWriter.beginArray("samples");  // Samples object.
            for (double sample : samples) {
                jsonWriter.appendDoubleDigits(sample, 16);
            }
            jsonWriter.endArray();  // Samples object.

            // Statistics.
            if (!statKeys.empty()) {
                // Dump stats to JSON. Only SKPBench currently returns valid key/value pairs.
                SkASSERT(statKeys.size() == statValues.size());
                for (int j = 0; j < statKeys.size(); j++) {
                    jsonWriter.appendMetric(statKeys[j].c_str(), statValues[j]);
                }
            }

            jsonWriter.endObject();  // Config object.

            runs++;
            if (runs % FLAGS_flushEvery == 0) {
                jsonWriter.flush();
            }

            print_benchmark_stats(&stats, &samples, target.get(), surfaceConfig, loops);

            pool.drain();
        }

        jsonWriter.endBench();
    }

    BenchmarkTarget::printGlobalStats();

    SkGraphics::PurgeAllCaches();

    jsonWriter.beginBench("memory_usage", 0, 0);  // Memory usage bench.
    jsonWriter.beginObject("meta");               // Meta object.
    jsonWriter.appendS32("max_rss_mb", sk_tools::getMaxResidentSetSizeMB());
    jsonWriter.endObject();  // Meta object.
    jsonWriter.endBench();   // Memory usage bench.

    jsonWriter.endObject();  // "results" dictionary.
    jsonWriter.endObject();  // Root object.

    SkDebugf("JSON file written to: %s\n", jsonPath.c_str());
    SkDebugf("PNGs (if any) written to: %s\n", outputDir.c_str());

    return 0;
}
