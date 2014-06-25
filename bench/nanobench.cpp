/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "CrashHandler.h"
#include "Stats.h"
#include "Timer.h"

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkString.h"
#include "SkSurface.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_string(match, "", "The usual filters on file names of benchmarks to measure.");
DEFINE_bool2(quiet, q, false, "Print only bench name and minimum sample.");
DEFINE_bool2(verbose, v, false, "Print all samples.");
DEFINE_string(config, "8888 nonrendering",
              "Configs to measure. Options: 565 8888 nonrendering");

// TODO: GPU benches

static SkString humanize(double ms) {
    if (ms > 1e+3) return SkStringPrintf("%.3gs",  ms/1e3);
    if (ms < 1e-3) return SkStringPrintf("%.3gns", ms*1e6);
    if (ms < 1)    return SkStringPrintf("%.3gµs", ms*1e3);
    return SkStringPrintf("%.3gms", ms);
}

static double estimate_timer_overhead() {
    double overhead = 0;
    WallTimer timer;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        timer.start();
        timer.end();
        overhead += timer.fWall;
    }
    return overhead / FLAGS_overheadLoops;
}

static void safe_flush(SkCanvas* canvas) {
    if (canvas) {
        canvas->flush();
    }
}

static int guess_loops(double overhead, Benchmark* bench, SkCanvas* canvas) {
    WallTimer timer;

    // Measure timer overhead and bench time together.
    do {
        timer.start();
        bench->draw(1, canvas);
        safe_flush(canvas);
        timer.end();
    } while (timer.fWall < overhead);  // Shouldn't normally happen.

    // Later we'll just start and stop the timer once, but loop N times.
    // We'll pick N to make timer overhead negligible:
    //
    //           Timer Overhead
    //  -------------------------------  < FLAGS_overheadGoal
    //  Timer Overhead + N * Bench Time
    //
    // where timer.fWall ≈ Timer Overhead + Bench Time.
    //
    // Doing some math, we get:
    //
    //  (Timer Overhead / FLAGS_overheadGoal) - Timer Overhead
    //  -----------------------------------------------------  < N
    //           (timer.fWall - Timer Overhead)
    //
    // Luckily, this also works well in practice. :)
    const double numer = overhead / FLAGS_overheadGoal - overhead;
    const double denom = timer.fWall - overhead;
    return (int)ceil(numer / denom);
}

static bool push_config_if_enabled(const char* config, SkTDArray<const char*>* configs) {
    if (FLAGS_config.contains(config)) {
        configs->push(config);
        return true;
    }
    return false;
}

static void create_surfaces(Benchmark* bench,
                            SkTDArray<SkSurface*>* surfaces,
                            SkTDArray<const char*>* configs) {

    if (bench->isSuitableFor(Benchmark::kNonRendering_Backend)
        && push_config_if_enabled("nonrendering", configs)) {
        surfaces->push(NULL);
    }

    if (bench->isSuitableFor(Benchmark::kRaster_Backend)) {
        const int w = bench->getSize().fX,
                  h = bench->getSize().fY;

        if (push_config_if_enabled("8888", configs)) {
            const SkImageInfo info = { w, h, kN32_SkColorType, kPremul_SkAlphaType };
            surfaces->push(SkSurface::NewRaster(info));
        }

        if (push_config_if_enabled("565", configs)) {
            const SkImageInfo info = { w, h, kRGB_565_SkColorType, kOpaque_SkAlphaType };
            surfaces->push(SkSurface::NewRaster(info));
        }
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkCommandLineFlags::Parse(argc, argv);

    const double overhead = estimate_timer_overhead();

    if (FLAGS_verbose) {
        // No header.
    } else if (FLAGS_quiet) {
        SkDebugf("min\tbench\tconfig\n");
    } else {
        SkDebugf("loops\tmin\tmean\tmax\tstddev\tbench\tconfig\n");
    }

    for (const BenchRegistry* r = BenchRegistry::Head(); r != NULL; r = r->next()) {
        SkAutoTDelete<Benchmark> bench(r->factory()(NULL));
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getName())) {
            continue;
        }

        SkTDArray<SkSurface*> surfaces;
        SkTDArray<const char*> configs;
        create_surfaces(bench.get(), &surfaces, &configs);

        bench->preDraw();
        for (int j = 0; j < surfaces.count(); j++) {
            SkCanvas* canvas = surfaces[j] ? surfaces[j]->getCanvas() : NULL;
            const char* config = configs[j];

            bench->draw(1, canvas);  // Just paranoid warmup.
            safe_flush(canvas);
            const int loops = guess_loops(overhead, bench.get(), canvas);

            SkAutoTMalloc<double> samples(FLAGS_samples);
            WallTimer timer;
            for (int i = 0; i < FLAGS_samples; i++) {
                timer.start();
                bench->draw(loops, canvas);
                safe_flush(canvas);
                timer.end();
                samples[i] = timer.fWall / loops;
            }

            Stats stats(samples.get(), FLAGS_samples);

            if (FLAGS_verbose) {
                for (int i = 0; i < FLAGS_samples; i++) {
                    SkDebugf("%s  ", humanize(samples[i]).c_str());
                }
                SkDebugf("%s\n", bench->getName());
            } else if (FLAGS_quiet) {
                if (configs.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\t%s\n", humanize(stats.min).c_str(), bench->getName(), config);
            } else {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf("%d\t%s\t%s\t%s\t%.0f%%\t%s\t%s\n"
                        , loops
                        , humanize(stats.min).c_str()
                        , humanize(stats.mean).c_str()
                        , humanize(stats.max).c_str()
                        , stddev_percent
                        , bench->getName()
                        , config
                        );
            }
        }
        surfaces.deleteAll();
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
