/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
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
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/AutoreleasePool.h"
#include "tools/ProcStats.h"
#include "tools/Stats.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/testrunners/benchmark/target/BenchmarkTarget.h"
#include "tools/testrunners/common/TestRunner.h"
#include "tools/testrunners/common/compilation_mode_keys/CompilationModeKeys.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"
#include "tools/timer/Timer.h"

#include <cinttypes>
#include <sstream>

static DEFINE_string(skip, "", "Space-separated list of test cases (regexps) to skip.");
static DEFINE_string(
        match,
        "",
        "Space-separated list of test cases (regexps) to run. Will run all tests if omitted.");

// TODO(lovisolo): Should we check that this is a valid Git hash?
static DEFINE_string(
        gitHash,
        "",
        "Git hash to include in the results.json output file, which can be ingested by Perf.");

static DEFINE_string(issue,
                     "",
                     "Changelist ID (e.g. a Gerrit changelist number) to include in the "
                     "results.json output file, which can be ingested by Perf.");

static DEFINE_string(patchset,
                     "",
                     "Patchset ID (e.g. a Gerrit patchset number) to include in the results.json "
                     "output file, which can be ingested by Perf.");

static DEFINE_string(key,
                     "",
                     "Space-separated key/value pairs common to all benchmarks. These will be "
                     "included in the results.json output file, which can be ingested by Perf.");

static DEFINE_string(
        links,
        "",
        "Space-separated name/URL pairs with additional information about the benchmark execution, "
        "for example links to the Swarming bot and task pages, named \"swarming_bot\" and "
        "\"swarming_task\", respectively. These links are included in the "
        "results.json output file, which can be ingested by Perf.");

// When running under Bazel and overriding the output directory, you might encounter errors
// such as "No such file or directory" and "Read-only file system". The former can happen
// when running on RBE because the passed in output dir might not exist on the remote
// worker, whereas the latter can happen when running locally in sandboxed mode, which is
// the default strategy when running outside of RBE. One possible workaround is to run the
// test as a local subprocess, which can be done by passing flag --strategy=TestRunner=local
// to Bazel.
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
                     "//tools/testrunners/common/surface_manager/SurfaceManager.h for details.");

static DEFINE_string(
        cpuName,
        "",
        "Contents of the \"cpu_or_gpu_value\" dimension for CPU-bound traces (e.g. \"AVX512\").");

static DEFINE_string(
        gpuName,
        "",
        "Contents of the \"cpu_or_gpu_value\" dimension for GPU-bound traces (e.g. \"RTX3060\").");

static DEFINE_bool(
        writePNGs,
        false,
        "Whether or not to write to the output directory any bitmaps produced by benchmarks.");

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

// Set in //bazel/devicesrc but only consumed by adb_test_runner.go. We cannot use the
// DEFINE_string macro because the flag name includes dashes.
[[maybe_unused]] static bool unused =
        SkFlagInfo::CreateStringFlag("device-specific-bazel-config",
                                     nullptr,
                                     new CommandLineFlags::StringArray(),
                                     nullptr,
                                     "Ignored by this test runner.",
                                     nullptr);

static void validate_flags(bool isBazelTest) {
    TestRunner::FlagValidators::AllOrNone(
            {{"--issue", FLAGS_issue.size() > 0}, {"--patchset", FLAGS_patchset.size() > 0}});
    TestRunner::FlagValidators::StringAtMostOne("--issue", FLAGS_issue);
    TestRunner::FlagValidators::StringAtMostOne("--patchset", FLAGS_patchset);
    TestRunner::FlagValidators::StringEven("--key", FLAGS_key);
    TestRunner::FlagValidators::StringEven("--links", FLAGS_links);

    if (!isBazelTest) {
        TestRunner::FlagValidators::StringNonEmpty("--outputDir", FLAGS_outputDir);
    }
    TestRunner::FlagValidators::StringAtMostOne("--outputDir", FLAGS_outputDir);

    TestRunner::FlagValidators::StringNonEmpty("--surfaceConfig", FLAGS_surfaceConfig);
    TestRunner::FlagValidators::StringAtMostOne("--surfaceConfig", FLAGS_surfaceConfig);

    TestRunner::FlagValidators::StringAtMostOne("--cpuName", FLAGS_cpuName);
    TestRunner::FlagValidators::StringAtMostOne("--gpuName", FLAGS_gpuName);

    TestRunner::FlagValidators::ExactlyOne(
            {{"--loops", FLAGS_loops != 0}, {"--autoTuneLoops", FLAGS_autoTuneLoops}});
    if (!FLAGS_autoTuneLoops) {
        TestRunner::FlagValidators::IntGreaterOrEqual("--loops", FLAGS_loops, 1);
    }

    TestRunner::FlagValidators::IntGreaterOrEqual("--autoTuneLoopsMax", FLAGS_autoTuneLoopsMax, 1);

    TestRunner::FlagValidators::ExactlyOne(
            {{"--samples", FLAGS_samples != 0}, {"--ms", FLAGS_ms != 0}});
    if (FLAGS_ms == 0) {
        TestRunner::FlagValidators::IntGreaterOrEqual("--samples", FLAGS_samples, 1);
    }
    if (FLAGS_samples == 0) {
        TestRunner::FlagValidators::IntGreaterOrEqual("--ms", FLAGS_ms, 1);
    }
}

// Helper class to produce JSON files in Perf ingestion format. The format is determined by Perf's
// format.Format Go struct:
//
// https://skia.googlesource.com/buildbot/+/e12f70e0a3249af6dd7754d55958ee64a22e0957/perf/go/ingest/format/format.go#168
//
// Note that the JSON format produced by this class differs from Nanobench. The latter follows
// Perf's legacy format, which is determined by the format.BenchData Go struct:
//
// https://skia.googlesource.com/buildbot/+/e12f70e0a3249af6dd7754d55958ee64a22e0957/perf/go/ingest/format/leagacyformat.go#26
class ResultsJSONWriter {
public:
    // This struct mirrors Perf's format.SingleMeasurement Go struct:
    // https://skia.googlesource.com/buildbot/+/e12f70e0a3249af6dd7754d55958ee64a22e0957/perf/go/ingest/format/format.go#31.
    struct SingleMeasurement {
        std::string value;
        double measurement;
    };

    // This struct mirrors Perf's format.Result Go struct:
    // https://skia.googlesource.com/buildbot/+/e12f70e0a3249af6dd7754d55958ee64a22e0957/perf/go/ingest/format/format.go#69.
    //
    // Note that the format.Result Go struct supports either one single measurement, or multiple
    // measurements represented as a dictionary from arbitrary string keys to an array of
    // format.SingleMeasurement Go structs. This class focuses on the latter variant.
    struct Result {
        std::map<std::string, std::string> key;
        std::map<std::string, std::vector<SingleMeasurement>> measurements;
    };

    ResultsJSONWriter(const char* path)
            : fFileWStream(path)
            , fJson(&fFileWStream, SkJSONWriter::Mode::kPretty)
            , fAddingResults(false) {
        fJson.beginObject();  // Root object.
        fJson.appendS32("version", 1);
    }

    void addGitHash(std::string gitHash) {
        assertNotAddingResults();
        fJson.appendCString("git_hash", gitHash.c_str());
    }

    void addChangelistInfo(std::string issue, std::string patchset) {
        assertNotAddingResults();
        fJson.appendCString("issue", issue.c_str());
        fJson.appendCString("patchset", patchset.c_str());
    }

    void addKey(std::map<std::string, std::string> key) {
        assertNotAddingResults();
        fJson.beginObject("key");
        for (auto const& [name, value] : key) {
            fJson.appendCString(name.c_str(), value.c_str());
        }
        fJson.endObject();
    }

    void addLinks(std::map<std::string, std::string> links) {
        assertNotAddingResults();
        fJson.beginObject("links");
        for (auto const& [key, value] : links) {
            fJson.appendCString(key.c_str(), value.c_str());
        }
        fJson.endObject();
    }

    void addResult(Result result) {
        if (!fAddingResults) {
            fAddingResults = true;
            fJson.beginArray("results");  // "results" array.
        }

        fJson.beginObject();  // Result object.

        // Key.
        fJson.beginObject("key");  // "key" dictionary.
        for (auto const& [name, value] : result.key) {
            fJson.appendCString(name.c_str(), value.c_str());
        }
        fJson.endObject();  // "key" dictionary.

        // Measurements.
        fJson.beginObject("measurements");  // "measurements" dictionary.
        for (auto const& [name, singleMeasurements] : result.measurements) {
            fJson.beginArray(name.c_str());  // <name> array.
            for (const SingleMeasurement& singleMeasurement : singleMeasurements) {
                // Based on
                // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/ResultsWriter.h#51.
                //
                // Don't record if NaN or Inf.
                if (SkIsFinite(singleMeasurement.measurement)) {
                    fJson.beginObject();
                    fJson.appendCString("value", singleMeasurement.value.c_str());
                    fJson.appendDoubleDigits("measurement", singleMeasurement.measurement, 16);
                    fJson.endObject();
                }
            }
            fJson.endArray();  // <name> array.
        }
        fJson.endObject();  // "measurements" dictionary.

        fJson.endObject();  // Result object.
    }

    void flush() { fJson.flush(); }

    ~ResultsJSONWriter() {
        if (fAddingResults) {
            fJson.endArray();  // "results" array;
        }
        fJson.endObject();  // Root object.
    }

private:
    void assertNotAddingResults() {
        if (fAddingResults) {
            SK_ABORT("Cannot perform this operation after addResults() is called.");
        }
    }

    SkFILEWStream fFileWStream;
    SkJSONWriter fJson;
    bool fAddingResults;
};

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
            TestRunner::Log(
                    "Warning: Clamping loops from %d to %d (per the --autoTuneLoopsMax flag) for "
                    "benchmark \"%s\".",
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
    if (target->getBackend() == Benchmark::Backend::kNonRendering) {
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
        TestRunner::Log("Warning: Could not read canvas pixels for benchmark \"%s\".",
                        target->getBenchmark()->getUniqueName());
        return;
    }

    SkFILEWStream stream(filename.c_str());
    if (!stream.isValid()) {
        TestRunner::Log("Warning: Could not write file \"%s\".", filename.c_str());
        return;
    }
    if (!SkPngEncoder::Encode(&stream, bmp.pixmap(), {})) {
        TestRunner::Log("Warning: Could not encode pixels from benchmark \"%s\" as PNG.",
                        target->getBenchmark()->getUniqueName());
        return;
    }

    if (FLAGS_verbose) {
        TestRunner::Log("PNG for benchmark \"%s\" written to: %s",
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
        TestRunner::Log("%4d/%-4dMB\t%s\t%s",
                        sk_tools::getCurrResidentSetSizeMB(),
                        sk_tools::getMaxResidentSetSizeMB(),
                        target->getBenchmark()->getUniqueName(),
                        surfaceConfig.c_str());
    } else if (FLAGS_quiet) {
        const char* mark = " ";
        const double stddev_percent = sk_ieee_double_divide(100 * sqrt(stats->var), stats->mean);
        if (stddev_percent > 5) mark = "?";
        if (stddev_percent > 10) mark = "!";
        TestRunner::Log("%10.2f %s\t%s\t%s",
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
        TestRunner::Log("%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s",
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
        std::ostringstream oss;
        oss << "Samples: ";
        for (int j = 0; j < samples->size(); j++) {
            oss << HUMANIZE((*samples)[j]) << "  ";
        }
        oss << target->getBenchmark()->getUniqueName();
        TestRunner::Log("%s", oss.str().c_str());
    }
}

// Based on
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1337.
int main(int argc, char** argv) {
    TestRunner::InitAndLogCmdlineArgs(argc, argv);

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

    std::string cpuName = FLAGS_cpuName.isEmpty() ? "" : FLAGS_cpuName[0];
    std::string gpuName = FLAGS_gpuName.isEmpty() ? "" : FLAGS_gpuName[0];

    // Output JSON file.
    //
    // TODO(lovisolo): Define a constant with the file name, use it here and in flag descriptions.
    SkString jsonPath = SkOSPath::Join(outputDir.c_str(), "results.json");
    ResultsJSONWriter jsonWriter(jsonPath.c_str());

    if (FLAGS_gitHash.size() == 1) {
        jsonWriter.addGitHash(FLAGS_gitHash[0]);
    } else {
        TestRunner::Log(
                "Warning: No --gitHash flag was specified. Perf ingestion ignores JSON files that "
                "do not specify a Git hash. This is fine for local debugging, but CI tasks should "
                "always set the --gitHash flag.");
    }
    if (FLAGS_issue.size() == 1 && FLAGS_patchset.size() == 1) {
        jsonWriter.addChangelistInfo(FLAGS_issue[0], FLAGS_patchset[0]);
    }

    // Key.
    std::map<std::string, std::string> keyValuePairs = {
            // Add a key/value pair that nanobench will never use in order to avoid accidentally
            // polluting an existing trace.
            {"build_system", "bazel"},
    };
    for (int i = 1; i < FLAGS_key.size(); i += 2) {
        keyValuePairs[FLAGS_key[i - 1]] = FLAGS_key[i];
    }
    keyValuePairs.merge(GetCompilationModeGoldAndPerfKeyValuePairs());
    jsonWriter.addKey(keyValuePairs);

    // Links.
    if (FLAGS_links.size()) {
        std::map<std::string, std::string> links;
        for (int i = 1; i < FLAGS_links.size(); i += 2) {
            links[FLAGS_links[i - 1]] = FLAGS_links[i];
        }
        jsonWriter.addLinks(links);
    }

    int runs = 0;
    bool missingCpuOrGpuWarningLogged = false;
    for (auto benchmarkFactory : BenchRegistry::Range()) {
        std::unique_ptr<Benchmark> benchmark(benchmarkFactory(nullptr));

        if (!TestRunner::ShouldRunTestCase(benchmark->getUniqueName(), FLAGS_match, FLAGS_skip)) {
            TestRunner::Log("Skipping %s", benchmark->getName());
            continue;
        }

        benchmark->delayedSetup();

        std::unique_ptr<BenchmarkTarget> target =
                BenchmarkTarget::FromConfig(surfaceConfig, benchmark.get());
        SkASSERT_RELEASE(target);

        if (benchmark->isSuitableFor(target->getBackend())) {
            // Print warning about missing cpu_or_gpu key if necessary.
            if (target->isCpuOrGpuBound() == SurfaceManager::CpuOrGpu::kCPU && cpuName == "" &&
                !missingCpuOrGpuWarningLogged) {
                TestRunner::Log(
                        "Warning: The surface is CPU-bound, but flag --cpuName was not provided. "
                        "Perf traces will omit keys \"cpu_or_gpu\" and \"cpu_or_gpu_value\".");
                missingCpuOrGpuWarningLogged = true;
            }
            if (target->isCpuOrGpuBound() == SurfaceManager::CpuOrGpu::kGPU && gpuName == "" &&
                !missingCpuOrGpuWarningLogged) {
                TestRunner::Log(
                        "Warning: The surface is GPU-bound, but flag --gpuName was not provided. "
                        "Perf traces will omit keys \"cpu_or_gpu\" and \"cpu_or_gpu_value\".");
                missingCpuOrGpuWarningLogged = true;
            }

            // Run benchmark and collect samples.
            int loops;
            skia_private::TArray<double> samples;
            skia_private::TArray<SkString> statKeys;
            skia_private::TArray<double> statValues;
            if (!sample_benchmark(target.get(), &loops, &samples, &statKeys, &statValues)) {
                // Sampling failed. A warning has already been printed.
                pool.drain();
                continue;
            }

            if (FLAGS_writePNGs) {
                // Save the bitmap produced by the benchmark to disk, if applicable. Not all
                // benchmarks produce bitmaps, e.g. those that use the "nonrendering" config.
                maybe_write_png(target.get(), outputDir);
            }

            // Building stats.plot often shows up in profiles, so skip building it when we're
            // not going to print it anyway.
            const bool want_plot = !FLAGS_quiet && !FLAGS_ms;
            Stats stats(samples, want_plot);

            print_benchmark_stats(&stats, &samples, target.get(), surfaceConfig, loops);

            ResultsJSONWriter::Result result;
            result.key = {
                    // Based on
                    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1566.
                    {"name", std::string(benchmark->getName())},

                    // Replaces the "config" and "extra_config" keys set by nanobench.
                    {"surface_config", surfaceConfig},

                    // Based on
                    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1578.
                    //
                    // TODO(lovisolo): Determine these dynamically when we add support for GMBench,
                    //                 SKPBench, etc.
                    {"source_type", "bench"},
                    {"bench_type", "micro"},

                    // Nanobench adds a "test" key consisting of "<unique name>_<width>_<height>",
                    // presumably with the goal of making the trace ID unique, see
                    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1456.
                    //
                    // However, we can accomplish unique trace IDs by expressing "<width>" and
                    // "<height>" as their own keys.
                    //
                    // Regarding the "<unique name>" part of the "test" key:
                    //
                    //  - Nanobench sets "<unique name>" to the result of
                    //    Benchmark::getUniqueName():
                    //    https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/Benchmark.h#41.
                    //
                    //  - Benchmark::getUniqueName() returns Benchmark::getName() except for the
                    //    following two cases.
                    //
                    //  - SKPBench::getUniqueName() returns "<name>_<scale>":
                    //    https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/SKPBench.cpp#33.
                    //
                    //  - SKPAnimationBench returns "<name>_<tag>":
                    //    https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/SKPAnimationBench.cpp#18.
                    //
                    // Therefore it is important that we add "<scale>" and "<tag>" as their own
                    // keys when we eventually add support for these kinds of benchmarks.
                    //
                    // Based on
                    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1456.
                    {"width", SkStringPrintf("%d", benchmark->getSize().width()).c_str()},
                    {"height", SkStringPrintf("%d", benchmark->getSize().height()).c_str()},
            };
            result.key.merge(target->getKeyValuePairs(cpuName, gpuName));
            result.measurements["ms"] = {
                    // Based on
                    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1571.
                    {.value = "min", .measurement = stats.min},
                    {.value = "ratio",
                     .measurement = sk_ieee_double_divide(stats.median, stats.min)},
            };
            if (!statKeys.empty()) {
                // Based on
                // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1580.
                //
                // Only SKPBench currently returns valid key/value pairs.
                SkASSERT(statKeys.size() == statValues.size());
                result.measurements["stats"] = {};
                for (int i = 0; i < statKeys.size(); i++) {
                    result.measurements["stats"].push_back(
                            {.value = statKeys[i].c_str(), .measurement = statValues[i]});
                }
            }
            jsonWriter.addResult(result);

            runs++;
            if (runs % FLAGS_flushEvery == 0) {
                jsonWriter.flush();
            }

            pool.drain();
        } else {
            if (FLAGS_verbose) {
                TestRunner::Log("Skipping \"%s\" because backend \"%s\" was unsuitable.\n",
                                target->getBenchmark()->getUniqueName(),
                                surfaceConfig.c_str());
            }
        }
    }

    BenchmarkTarget::printGlobalStats();

    SkGraphics::PurgeAllCaches();

    // Based on
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#1668.
    jsonWriter.addResult({
            .key =
                    {
                            {"name", "memory_usage"},
                    },
            .measurements =
                    {
                            {"resident_set_size_mb",
                             {{.value = "max",
                               .measurement = double(sk_tools::getMaxResidentSetSizeMB())}}},
                    },
    });

    TestRunner::Log("JSON file written to: %s", jsonPath.c_str());
    TestRunner::Log("PNGs (if any) written to: %s", outputDir.c_str());

    return 0;
}
